#include <Python.h>
#include <assert.h>

#include "../RpiTx.h"
#include "../RpiGpio.h"

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

struct module_state {
	PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

static int streamFileNo = 0;
static int sampleRate = 48000;

typedef struct {
	double frequency;
	uint32_t waitForThisSample;
} samplerf_t;

static size_t read_float(int streamFileNo, float* wavBuffer, const size_t count);

/**
 * Formats a chunk of an array of a mono 48k wav at a time and outputs IQ
 * formatted array for broadcast.
 */
static ssize_t formatRfWrapper(void* const outBuffer, const size_t count) {
	static float wavBuffer[1024];
	static int wavOffset = -1;
	static int wavFilled = -1;

	if (wavFilled == 0) {
		return 0;
	}

	const int excursion = 6000;
	int numBytesWritten = 0;
	samplerf_t samplerf;
	samplerf.waitForThisSample = 1e9 / ((float)sampleRate);  //en 100 de nanosecond
	char* const out = outBuffer;

	while (numBytesWritten <= count - sizeof(samplerf_t)) {
		for (
				;
				numBytesWritten <= count - sizeof(samplerf_t) && wavOffset < wavFilled;
				++wavOffset
		) {
			const float x = wavBuffer[wavOffset];
			samplerf.frequency = x * excursion * 2.0;
			memcpy(&out[numBytesWritten], &samplerf, sizeof(samplerf_t));
			numBytesWritten += sizeof(samplerf_t);
		}

		assert(wavOffset <= wavFilled);

		if (wavOffset == wavFilled) {
			wavFilled = read_float(streamFileNo, wavBuffer, COUNT_OF(wavBuffer));
			wavOffset = 0;
		}
	}
	return numBytesWritten;
}
static size_t read_float(int streamFileNo, float* wavBuffer, const size_t count) {
	// Samples are stored as 16 bit signed integers in range of -32768 to 32767
	uint16_t sample;
	int i;
	for (i = 0; i < count; ++i) {
		const int byteCount = read(streamFileNo, &sample, sizeof(sample));
		if (byteCount != sizeof(sample)) {
			return i;
		}
		// TODO: I don't know if this should be dividing by 32767 or 32768. Probably
		// doesn't matter too much.
		*wavBuffer = ((float)le16toh(sample)) / 32768;
		++wavBuffer;
	}
	return count;
}
static void dummyFunction(void) {
	assert(0 && "dummyFunction should not be called");
}


static PyObject*
_rpitx_broadcast_fm(PyObject* self, PyObject* args) {
	float frequency;

	struct module_state *st = GETSTATE(self);

	if (!PyArg_ParseTuple(args, "if", &streamFileNo, &frequency)) {
		PyErr_SetString(st->error, "Invalid arguments");
		return NULL;
	}

	union {
		char char4[4];
		uint32_t uint32;
		uint16_t uint16;
	} buffer;
	size_t byteCount = read(streamFileNo, buffer.char4, 4);
	if (byteCount != 4 || strncmp(buffer.char4, "RIFF", 4) != 0) {
		PyErr_SetString(st->error, "Not a WAV file");
		return NULL;
	}

	// Skip chunk size
	byteCount = read(streamFileNo, buffer.char4, 4);
	if (byteCount != 4) {
		PyErr_SetString(st->error, "Not a WAV file");
		return NULL;
	}

	byteCount = read(streamFileNo, buffer.char4, 4);
	if (byteCount != 4 || strncmp(buffer.char4, "WAVE", 4) != 0) {
		PyErr_SetString(st->error, "Not a WAV file");
		return NULL;
	}

	byteCount = read(streamFileNo, buffer.char4, 4);
	if (byteCount != 4 || strncmp(buffer.char4, "fmt ", 4) != 0) {
		PyErr_SetString(st->error, "Not a WAV file");
		return NULL;
	}

	byteCount = read(streamFileNo, &buffer.uint32, 4);
	buffer.uint32 = le32toh(buffer.uint32);
	if (byteCount != 4 || buffer.uint32 != 16) {
		PyErr_SetString(st->error, "Not a PCM WAV file");
		return NULL;
	}

	byteCount = read(streamFileNo, &buffer.uint16, 2);
	buffer.uint16 = le16toh(buffer.uint16);
	if (byteCount != 2 || buffer.uint16 != 1) {
		PyErr_SetString(st->error, "Not an uncompressed WAV file");
		return NULL;
	}

	byteCount = read(streamFileNo, &buffer.uint16, 2);
	buffer.uint16 = le16toh(buffer.uint16);
	if (byteCount != 2 || buffer.uint16 != 1) {
		PyErr_SetString(st->error, "Not a mono WAV file");
		return NULL;
	}

	byteCount = read(streamFileNo, &buffer.uint32, 4);
	sampleRate = le32toh(buffer.uint32);
	// TODO: Is this required to be 48000?
	if (byteCount != 4) {
		PyErr_SetString(st->error, "Not a WAV file");
		return NULL;
	}

	// Skip byte rate
	byteCount = read(streamFileNo, &buffer.uint32, 4);
	if (byteCount != 4) {
		PyErr_SetString(st->error, "Not a WAV file");
		return NULL;
	}

	// Skip block align
	byteCount = read(streamFileNo, &buffer.uint16, 2);
	if (byteCount != 2) {
		PyErr_SetString(st->error, "Not a WAV file");
		return NULL;
	}

	byteCount = read(streamFileNo, &buffer.uint16, 2);
	buffer.uint16 = le16toh(buffer.uint16);
	if (byteCount != 2 || buffer.uint16 != 16) {
		PyErr_SetString(st->error, "Not a 16 bit WAV file");
		return NULL;
	}

	byteCount = read(streamFileNo, buffer.char4, 4);
	// Normal WAV files have "data" here, but avcov spits out "LIST"
	if (
			byteCount != 4 || (
				strncmp(buffer.char4, "data", 4) != 0
				&& strncmp(buffer.char4, "LIST", 4) != 0
				)
	) {
		PyErr_SetString(st->error, "Not a WAV file");
		return NULL;
	}

	// Skip subchunk2 size
	byteCount = read(streamFileNo, &buffer.uint32, 4);
	if (byteCount != 4) {
		PyErr_SetString(st->error, "Not a WAV file");
		return NULL;
	}

	int skipSignals[] = {
		SIGALRM,
		SIGVTALRM,
		SIGCHLD,  // We fork whenever calling broadcast_fm
		SIGWINCH,  // Window resized
		0
	};

	pitx_run(MODE_RF, sampleRate, frequency * 1000.0, 0.0, 0, formatRfWrapper, dummyFunction, skipSignals, 0);

	Py_RETURN_NONE;
}


static PyMethodDef _rpitx_methods[] = {
	{
		"broadcast_fm",
		_rpitx_broadcast_fm,
		METH_VARARGS,
		"Low-level broadcasting.\n\n"
			"Broadcast a WAV formatted 48KHz file from a pipe file descriptor.\n"
			"Args:\n"
			"    pipe_file_no (int): The fileno of the pipe that the WAV is being written to.\n"
			"    frequency (float): The frequency, in MHz, to broadcast on.\n"
	},
	{NULL, NULL, 0, NULL}
};


#if PY_MAJOR_VERSION >= 3
static int _rpitx_traverse(PyObject* m, visitproc visit, void* arg) {
	Py_VISIT(GETSTATE(m)->error);
	return 0;
}


static int _rpitx_clear(PyObject *m) {
	Py_CLEAR(GETSTATE(m)->error);
	return 0;
}


static struct PyModuleDef moduledef = {
	PyModuleDef_HEAD_INIT,
	"_rpitx",
	NULL,
	sizeof(struct module_state),
	_rpitx_methods,
	NULL,
	_rpitx_traverse,
	_rpitx_clear,
	NULL
};

#define INITERROR return NULL

PyObject*
PyInit__rpitx(void)

#else

#define INITERROR return

void
init_rpitx(void)
#endif
{
#if PY_MAJOR_VERSION >= 3
	PyObject* const module = PyModule_Create(&moduledef);
#else
	PyObject* const module = Py_InitModule("_rpitx", _rpitx_methods);
#endif
	if (module == NULL) {
		INITERROR;
	}
	struct module_state* st = GETSTATE(module);
	st->error = PyErr_NewException("_rpitx.Error", NULL, NULL);
	if (st->error == NULL) {
		Py_DECREF(module);
		INITERROR;
	}
	Py_INCREF(st->error);
	PyModule_AddObject(module, "error", st->error);

#if PY_MAJOR_VERSION >= 3
	return module;
#endif
}
