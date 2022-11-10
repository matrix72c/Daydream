#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "cupti_activity_tracer.h"

extern "C" {
static PyObject* start(PyObject* self, PyObject* args) {
    cupti_activity_tracer_init();

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject* stop(PyObject* self, PyObject* args) {
    cupti_activity_tracer_close();

    Py_INCREF(Py_None);
    return Py_None;
}
}
static PyMethodDef methods[] = {{"start", (PyCFunction)start, METH_O, NULL},
                                {"stop", (PyCFunction)stop, METH_NOARGS, NULL},
                                {NULL, NULL, 0, NULL}};

static PyModuleDef definitions = {PyModuleDef_HEAD_INIT, "daydream", "", -1,
                                  methods};

PyMODINIT_FUNC PyInit_daydream() {
    PyObject* module = PyModule_Create(&definitions);
    return module;
}
