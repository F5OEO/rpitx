#include <Python.h>
#include "../RpiTx.h"
#include "../RpiGpio.h"


static PyObject*
_rpitx_broadcast(PyObject* self, PyObject* args) {
    int address;
    int length;
    float frequency;
    if (!PyArg_ParseTuple(args, "iif", &address, &length, &frequency)) {
        return NULL;
    }

    setUpReadArray((void*)address, length);
    pitx_run(MODE_IQ, 44000, frequency, 0.0, 0, readArray, NULL);
    Py_RETURN_NONE;
}


static PyMethodDef _rpitx_methods[] = {
    {"broadcast", _rpitx_broadcast, METH_VARARGS, "Low-level broadcasting."},
    {NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC
init_rpitx(void) {
    (void) Py_InitModule("_rpitx", _rpitx_methods);
}
