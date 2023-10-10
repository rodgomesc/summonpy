/*
 DOCS for Extension module
   - flow
   - retrieve extracted shared libraries from android directory
   - create a dictionary to hold the shared libraries name and location
   - create a python extension to hook importlib.machinery.ExtensionFileLoader
     - override load_module, find_module, write, flush to look for our shared libs in a custom location
   - add the python extension to the meta path
*/

#include "py-ext-register.h"
#include "Logger.h"
#include <android/log.h>
#include <dirent.h>

#define PY_SSIZE_T_CLEAN

const char *PyExtension::tag = "pyjsi-ext-register";
PyObject *PyExtension::libDict = nullptr;
PyObject *PyExtension::mNativehooksModule = nullptr;
PyObject *PyExtension::mImpLoadDynamic = nullptr;

char *PyExtension::libs_dir = nullptr;

void PyExtension::LOG(const char *x)
{
  __android_log_write(ANDROID_LOG_INFO, tag, x);
}

// Implement static methods
PyObject *PyExtension::NativeHooks_write(PyObject *self, PyObject *args)
{
  char *str;
  if (!PyArg_ParseTuple(args, "s", &str))
  {
    return NULL;
  }
  LOG(str);
  Py_RETURN_NONE;
}

PyObject *PyExtension::NativeHooks_flush(PyObject *self, PyObject *args)
{
  Py_RETURN_NONE;
}

PyObject *PyExtension::NativeHooks_find_module(PyObject *self, PyObject *args)
{
  PyObject *mod = NULL;
  PyObject *path = NULL;
  if (!PyArg_ParseTuple(args, "OO", &mod, &path))
  {
    return NULL;
  }
  if (PyDict_Contains(libDict, mod))
  {
    Py_INCREF(mNativehooksModule);
    return mNativehooksModule;
  }
  Py_RETURN_NONE;
}

PyObject *PyExtension::NativeHooks_load_module(PyObject *self, PyObject *args)
{


    // Print the contents of args tuple
    Py_ssize_t len = PyTuple_Size(args);
    for (Py_ssize_t i = 0; i < len; i++) {
        PyObject *item = PyTuple_GetItem(args, i);
        PyObject_Print(item, stdout, 0);
        printf("\n");
    }

    PyObject *mod;
  if (!PyArg_ParseTuple(args, "O", &mod))
  {
    return NULL;
  }
  PyObject *sys_modules = PyImport_GetModuleDict();
  if (PyDict_Contains(sys_modules, mod))
  {
    return PyDict_GetItem(sys_modules, mod);
  }
  PyObject *loader = PyObject_CallFunction(mImpLoadDynamic, "OO", mod, PyDict_GetItem(libDict, mod));
  PyObject *result = PyObject_CallMethod(loader, "load_module", NULL);
  Py_XDECREF(loader);
  return result;
}

PyObject *PyExtension::NativeHooks_say_hello(PyObject *self, PyObject *args)
{
  return Py_BuildValue("s", "Hello, World!");
}

PyExtension::PyExtension(char *ldir)
{
  libs_dir = ldir;
}

void PyExtension::setSharedLibsToPydict(const char *libdir)
{
  libDict = PyDict_New();

  DIR *dir = opendir(libdir);
  struct dirent *ent;

  if (dir != NULL)
  {
    while ((ent = readdir(dir)) != NULL)
    {
      if (ent->d_type == DT_REG && strncmp("lib.", ent->d_name, 4) == 0)
      {
        char mod[256], path[256];
        snprintf(mod, sizeof(mod), "%.*s", (int)strlen(ent->d_name) - 7, ent->d_name + 4);
        snprintf(path, sizeof(path), "%s/%s", libdir, ent->d_name);
        PyDict_SetItem(libDict, Py_BuildValue("s", mod), Py_BuildValue("s", path));
      }
    }
    closedir(dir);
  }
  else
  {
    PyExtension::LOG("Python extension dir is invalid");
  }
}

PyMethodDef HelloMethods[] = {
    {"write", PyExtension::NativeHooks_write, METH_VARARGS, "Write to android log"},
    {"flush", PyExtension::NativeHooks_flush, METH_VARARGS, "Required for logging"},
    {"load_module", PyExtension::NativeHooks_load_module, METH_VARARGS, "Load an extension"},
    {"find_module", PyExtension::NativeHooks_find_module, METH_VARARGS, "Find an extension"},
    {"say_hello", PyExtension::NativeHooks_say_hello, METH_VARARGS, "Say hello"},
    {NULL, NULL, 0, NULL}};

struct PyModuleDef helloModule = {
    PyModuleDef_HEAD_INIT,
    "hello",
    "docs",
    -1,
    HelloMethods};


PyMODINIT_FUNC PyInit_hello(void)
{
  PyExtension::mNativehooksModule = PyModule_Create(&helloModule);
  if (PyExtension::mNativehooksModule == NULL)
  {
    return NULL;
  }

  PyObject* imp =  PyImport_ImportModule("importlib.machinery");
  PyExtension::mImpLoadDynamic = PyObject_GetAttrString(imp, "ExtensionFileLoader");


  PySys_SetObject("stdout", PyExtension::mNativehooksModule);
  PySys_SetObject("stderr", PyExtension::mNativehooksModule);

  // create a dictionary to hold the shared libraries name and location
  PyExtension::setSharedLibsToPydict(PyExtension::libs_dir);
//
//    PyObject *test_module_name = Py_BuildValue("s", "_math.so");
//    if (test_module_name == NULL) {
//        PyErr_Print();
//        return NULL;  // or however you handle errors
//    }
//
//    PyObject *args_tuple = PyTuple_New(1);
//    if (args_tuple == NULL) {
//        Py_XDECREF(test_module_name);
//        PyErr_Print();
//        return NULL;  // or however you handle errors
//    }
//
//    if (PyTuple_SetItem(args_tuple, 0, test_module_name) < 0) {
//        Py_XDECREF(args_tuple);
//        PyErr_Print();
//        return NULL;  // or however you handle errors
//    }
//
//    PyObject *loaded_module = PyExtension::NativeHooks_load_module(NULL, args_tuple);
//    if (loaded_module == NULL) {
//        Py_XDECREF(args_tuple);
//        PyErr_Print();
//        return NULL;  // or however you handle errors
//    }
//
//    Py_XDECREF(args_tuple);

    // Add to meta path
    Py_INCREF(PyExtension::mNativehooksModule);
   // why? https://stackoverflow.com/a/49529009
    PyObject* meta_path = PySys_GetObject("meta_path");
    PyObject_CallMethod(meta_path, "append", "O", PyExtension::mNativehooksModule);


  return PyExtension::mNativehooksModule;
}
