#ifndef PY_EXTENSION_H
#define PY_EXTENSION_H

#include <Python.h>

class PyExtension {
public:
    PyExtension(char *libs_dir);

    static const char *tag;
    static PyObject *mImpLoadDynamic;
    static PyObject* libDict;
    static char *libs_dir;
    static PyObject *mNativehooksModule;

    static void LOG(const char *x);
    static void setSharedLibsToPydict(const char* libdir);
    static PyObject *NativeHooks_write(PyObject *self, PyObject *args);
    static PyObject *NativeHooks_flush(PyObject *self, PyObject *args);
    static PyObject *NativeHooks_find_module(PyObject *self, PyObject *args);
    static PyObject *NativeHooks_load_module(PyObject *self, PyObject *args);
    static PyObject *NativeHooks_say_hello(PyObject *self, PyObject *args);


};

PyMODINIT_FUNC  PyInit_hello(void);

#endif // PY_EXTENSION_H
