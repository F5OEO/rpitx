#include <Python.h>
#include <assert.h>
#include <sndfile.h>

#include "../RpiTx.h"
#include "../RpiGpio.h"

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))


static void* sampleBase;
static sf_count_t sampleLength;
static sf_count_t sampleOffset;
static SNDFILE* sndFile;
static int bitRate;
static PyObject* rpitxError;


// These methods used by libsndfile's virtual file open function
static sf_count_t virtualSndfileGetLength(void* unused) {
	return sampleLength;
}
static sf_count_t virtualSndfileRead(void* const dest, const sf_count_t count, void* const userData) {
	const sf_count_t bytesAvailable = sampleLength - sampleOffset;
	const int numBytes = bytesAvailable > count ? count : bytesAvailable;
	memcpy(dest, ((char*)userData) + sampleOffset, numBytes);
	sampleOffset += numBytes;
	return numBytes;
}
static sf_count_t virtualSndfileTell(void* const unused) {
	return sampleOffset;
}
static sf_count_t virtualSndfileSeek(
		const sf_count_t offset,
		const int whence,
		void* const unused
) {
	switch (whence) {
		case SEEK_CUR:
			sampleOffset += offset;
			break;
		case SEEK_SET:
			sampleOffset = offset;
			break;
		case SEEK_END:
			sampleOffset = sampleLength - offset;
			break;
		default:
			assert(!"Invalid whence");
	}
	return sampleOffset;
}


typedef struct {
	double frequency;
	uint32_t waitForThisSample;
} samplerf_t;
/**
 * Formats a chunk of an array of a mono 44k wav at a time and outputs IQ
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
static void reset(void) {
	sampleOffset = 0;
}


static PyObject*
_rpitx_broadcast_fm(PyObject* self, PyObject* args) {
	long int address;
	float frequency;

	assert(sizeof(address) == sizeof(sampleBase));
	if (!PyArg_ParseTuple(args, "lif", &address, &sampleLength, &frequency)) {
		PyErr_SetString(rpitxError, "Invalid arguments");
		Py_RETURN_NONE;
	}

	sampleBase = ((void*)address);
	sampleOffset = 0;

	SF_VIRTUAL_IO virtualIo = {
		.get_filelen = virtualSndfileGetLength,
		.seek = virtualSndfileSeek,
		.read = virtualSndfileRead,
		.write = NULL,
		.tell = virtualSndfileTell
	};
	SF_INFO sfInfo;
	sndFile = sf_open_virtual(&virtualIo, SFM_READ, &sfInfo, sampleBase);
	if (sf_error(sndFile) != SF_ERR_NO_ERROR) {
		char message[100];
		snprintf(
				message,
				COUNT_OF(message),
				"Unable to open sound file: %s",
				sf_strerror(sndFile));
		message[COUNT_OF(message) - 1] = '\0';
		PyErr_SetString(rpitxError, message);
		Py_RETURN_NONE;
	}
	bitRate = sfInfo.samplerate;

	pitx_run(MODE_RF, bitRate, frequency * 1000.0, 0.0, 0, formatRfWrapper, reset);
	sf_close(sndFile);

	Py_RETURN_NONE;
}


static PyMethodDef _rpitx_methods[] = {
	{
		"broadcast_fm",
		_rpitx_broadcast_fm,
		METH_VARARGS,
		"Low-level broadcasting.\n\n"
			"Broadcast a WAV formatted 48KHz memory array.\n"
			"Args:\n"
			"    address (int): Address of the memory array.\n"
			"    length (int): Length of the memory array.\n"
			"    frequency (float): The frequency, in MHz, to broadcast on.\n"
	},
	{NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC
init_rpitx(void) {
	PyObject* const module = Py_InitModule("_rpitx", _rpitx_methods);
	if (module == NULL) {
		return;
	}
	rpitxError = PyErr_NewException("_rpitx.error", NULL, NULL);
	Py_INCREF(rpitxError);
	PyModule_AddObject(module, "error", rpitxError);
}
