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
static sf_count_t virtualSndfileSeek(const sf_count_t offset, const int whence, void* const unused) {
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
static sf_count_t virtualSndfileWrite(const void *ptr, sf_count_t count, void *user_data) {
	return 0;
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
	static int wavOffset = 0;
	static int wavFilled = 0;

	const int excursion = 6000;
	int numBytesWritten = 0;
	samplerf_t samplerf;
	samplerf.waitForThisSample = 1e9 / ((float)bitRate);  //en 100 de nanosecond
	char* const out = outBuffer;

	while (numBytesWritten < count) {
		for (; numBytesWritten <= count - sizeof(samplerf_t) && wavOffset < wavFilled; ++wavOffset) {
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
	int address;
	int length;
	float frequency;
	if (!PyArg_ParseTuple(args, "iif", &address, &length, &frequency)) {
		return NULL;
	}

	sampleBase = (void*)address;
	sampleLength = length;
	sampleOffset = 0;

	SF_VIRTUAL_IO virtualIo = {
		.get_filelen = virtualSndfileGetLength,
		.seek = virtualSndfileSeek,
		.read = virtualSndfileRead,
		.write = virtualSndfileWrite,
		.tell = virtualSndfileTell
	};
	SF_INFO sfInfo;
	sndFile = sf_open_virtual(&virtualIo, SFM_READ, &sfInfo, sampleBase);
	if (sf_error(sndFile) != SF_ERR_NO_ERROR) {
		// TODO: Throw an exception
		fprintf(stderr, "Unable to open sound file: %s\n", sf_strerror(sndFile));
		Py_RETURN_NONE;
	}
	bitRate = sfInfo.samplerate;

	pitx_run(MODE_RF, bitRate, frequency * 1000.0, 0.0, 0, formatRfWrapper, reset);
	sf_close(sndFile);

	Py_RETURN_NONE;
}


static PyMethodDef _rpitx_methods[] = {
	{"broadcast_fm", _rpitx_broadcast_fm, METH_VARARGS, "Low-level broadcasting."},
	{NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC
init_rpitx(void) {
	(void) Py_InitModule("_rpitx", _rpitx_methods);
}
