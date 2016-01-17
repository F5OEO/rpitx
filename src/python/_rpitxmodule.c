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


// These methods used by libsndfile's virtual file open function
sf_count_t virtualSndfileGetLength(void* unused) {
    return sampleLength;
}
sf_count_t virtualSndfileRead(void* const dest, const sf_count_t count, void* const userData) {
    const sf_count_t bytesAvailable = sampleLength - sampleOffset;
    const int numBytes = bytesAvailable > count ? count : bytesAvailable;
    memcpy(dest, userData, numBytes);
    sampleOffset += numBytes;
    return numBytes;
}


typedef struct {
    double frequency;
    uint32_t waitForThisSample;
} samplerf_t;
/**
 * Formats a chunk of an array of a mono 44k wav at a time and outputs IQ
 * formatted array for broadcast.
 */
ssize_t formatIqWrapper(void* const outBuffer, size_t count) {
    static float readBuffer[1024];
    const int excursion = 6000;
    samplerf_t samplerf;
    char* out = outBuffer;

    int readCount;
    int k;
    int totalBytesToRead = count / sizeof(samplerf_t);
    int bytesToRead = totalBytesToRead;
    if (bytesToRead > COUNT_OF(readBuffer)) {
        bytesToRead = COUNT_OF(readBuffer);
    }
    int bytesWritten = 0;
    while ((readCount = sf_readf_float(sndFile, readBuffer, bytesToRead))) {
        for (k = 0; k < readCount; k++) {
            const int x = readBuffer[k];
            samplerf.frequency = x * excursion * 2.0;
            samplerf.waitForThisSample = 1e9 / 48000.0; //en 100 de nanosecond
            memcpy(&out[bytesWritten], &samplerf, sizeof(samplerf_t));
            bytesWritten += sizeof(samplerf_t);
        }
        totalBytesToRead -= bytesToRead;
        if (totalBytesToRead <= 0) {
            break;
        }
    }
    return bytesWritten;
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
        .seek = NULL,
        .read = virtualSndfileRead,
        .write = NULL,
        .tell = NULL
    };
	SF_INFO sfInfo ;
    sndFile = sf_open_virtual(&virtualIo, SFM_READ, &sfInfo, sampleBase);

    pitx_run(MODE_IQ, 44000, frequency, 0.0, 0, formatIqWrapper, NULL);
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
