#include <Python.h>
#include <assert.h>
#include <sndfile.h>

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

int streamFileNo = 0;
static SNDFILE* sndFile;
static int bitRate;


// These methods used by libsndfile's virtual file open function
static sf_count_t virtualSndfileGetLength(void* unused) {
	assert(0 && "virtualSndfileGetLength should not be called");
	return 0;
}
static sf_count_t virtualSndfileRead(void* const dest, sf_count_t count, void* const unused) {
	// So, the WAV file format starts with 12 bytes: the letters "RIFF", 4 byte
	// chunk size, and the letters "WAVE". Because we're streaming to a pipe, we
	// don't know the size of the file, and avconv just sticks in 0xFFFFFFFF, so
	// we'll just fudge a value in.
	if (count == 12) {
		read(streamFileNo, dest, count);  // Just skip these
		memcpy(dest, "RIFF\x42\xB0\x0A\x00WAVE", count);
		return count;
	}

	const ssize_t bytesRead = read(streamFileNo, dest, count);
	return bytesRead;
}
static sf_count_t virtualSndfileTell(void* const unused) {
	assert(0 && "virtualSndfileTell should not be called");
	return 0;
}
static sf_count_t virtualSndfileSeek(
		const sf_count_t offset,
		const int whence,
		void* const unused
) {
	assert(0 && "virtualSndfileSeek should not be called");
	return 0;
}


typedef struct {
	double frequency;
	uint32_t waitForThisSample;
} samplerf_t;
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
	samplerf.waitForThisSample = 1e9 / ((float)bitRate);  //en 100 de nanosecond
	char* const out = outBuffer;

	while (numBytesWritten < count) {
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
			wavFilled = sf_readf_float(sndFile, wavBuffer, COUNT_OF(wavBuffer));
			wavOffset = 0;
		}
	}
	return numBytesWritten;
}
static void dummyFunction(void) {
	assert(0 && "dummyFunction should not be called");
}


static PyObject*
_rpitx_broadcast_fm(PyObject* self, PyObject* args) {
	float frequency;

	if (!PyArg_ParseTuple(args, "if", &streamFileNo, &frequency)) {
		struct module_state *st = GETSTATE(self);
		PyErr_SetString(st->error, "Invalid arguments");
		return NULL;
	}

	SF_INFO sfInfo;
	SF_VIRTUAL_IO virtualIo = {
			.get_filelen = virtualSndfileGetLength,
			.seek = virtualSndfileSeek,
			.read = virtualSndfileRead,
			.write = NULL,
			.tell = virtualSndfileTell
	};
	sndFile = sf_open_virtual(&virtualIo, SFM_READ, &sfInfo, NULL);
	if (sf_error(sndFile) != SF_ERR_NO_ERROR) {
		char message[100];
		snprintf(
			message,
			COUNT_OF(message),
			"Unable to open pipe: %s",
			sf_strerror(sndFile));
		message[COUNT_OF(message) - 1] = '\0';
		struct module_state *st = GETSTATE(self);
		PyErr_SetString(st->error, message);
		return NULL;
	}
	bitRate = sfInfo.samplerate;

	int skipSignals[] = {
		SIGALRM,
		SIGVTALRM,
		SIGCHLD,  // We fork whenever calling broadcast_fm
		SIGWINCH,  // Window resized
		0
	};

	pitx_run(MODE_RF, bitRate, frequency * 1000.0, 0.0, 0, formatRfWrapper, dummyFunction, skipSignals, 0);

	Py_RETURN_NONE;
}


static PyMethodDef _rpitx_methods[] = {
	{
		"broadcast_fm",
		_rpitx_broadcast_fm,
		METH_VARARGS,
		"Low-level broadcasting.\n\n"
			"Broadcast a WAV formatted 48KHz file.\n"
			"Args:\n"
			"    file_name (str): The WAV file to broadcast.\n"
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
