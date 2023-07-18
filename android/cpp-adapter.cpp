#include <jni.h>
#include "react-native-pyjsi.h"
#include <iostream>
#include "Python.h"
#include <android/log.h>
#include <dirent.h>

#define PY_SSIZE_T_CLEAN
static const char *tag = "pyjsi-bridge";
#define LOG(x) __android_log_write(ANDROID_LOG_INFO, tag, (x))

static PyObject* mExtensions;
static PyObject* mImpLoadDynamic;
static PyObject* mNativehooksModule;
static int pfd[2];
static pthread_t mLogThread;



/**
 * Pipe to catch output from stdout and log it
 */
static void *thread_func(void* x) {
    ssize_t rdsz;
    char buf[128];
    while((rdsz = read(pfd[0], buf, sizeof buf - 1)) > 0) {
        if(buf[rdsz - 1] == '\n') --rdsz;
        buf[rdsz] = 0;  /* add null-terminator */
        __android_log_write(ANDROID_LOG_DEBUG, tag, buf);
    }
    return 0;
}

/**
 * Create a pipe and spawn a thread to catch output from nkd libraries
 * and send it to androids log.
 *
 * Thanks to https://codelab.wordpress.com/2014/11/03/how-to-use-standard-output-streams-for-logging-in-android-apps/
 */
int start_logger() {
    /* make stdout line-buffered and stderr unbuffered */
    setvbuf(stdout, 0, _IOLBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);

    /* create the pipe and redirect stdout and stderr */
    pipe(pfd);
    dup2(pfd[1], 1);
    dup2(pfd[1], 2);

    /* spawn the logging thread */
    if(pthread_create(&mLogThread, 0, thread_func, 0) == -1)
        return -1;
    pthread_detach(mLogThread);
    return 0;
}


/**
 * ExtensionImporter API
 */
static PyObject *NativeHooks_load_module(PyObject *self, PyObject *args) {
    PyObject *mod;
    if (!PyArg_ParseTuple(args, "O", &mod)) {
        return NULL;
    }
    PyObject* sys_modules = PyImport_GetModuleDict();
    if (PyDict_Contains(sys_modules, mod)) {
        return PyDict_GetItem(sys_modules, mod);
    }
#if PY_MAJOR_VERSION >= 3
    // Create an ExtensionFileLoader instance
    PyObject *loader = PyObject_CallFunction(mImpLoadDynamic, "OO", mod, PyDict_GetItem(mExtensions, mod));
    PyObject *result =  PyObject_CallMethod(loader, "load_module", NULL);
    Py_XDECREF(loader);
    return result;
#else
    return PyObject_CallFunction(mImpLoadDynamic, "OO", mod, PyDict_GetItem(mExtensions, mod));
#endif
}

static PyObject *NativeHooks_find_module(PyObject *self, PyObject *args) {
    PyObject *mod = NULL;
    PyObject *path = NULL;
    if (!PyArg_ParseTuple(args, "OO", &mod, &path)) {
        return NULL;
    }
    if (PyDict_Contains(mExtensions, mod)) {
        Py_INCREF(mNativehooksModule);
        return mNativehooksModule;
    }
    Py_RETURN_NONE;
}




static PyObject *NativeHooks_write(PyObject *self, PyObject *args) {
    char *str;
    if (!PyArg_ParseTuple(args, "s", &str)) {
        return NULL;
    }
    LOG(str);
    Py_RETURN_NONE;
}

static PyMethodDef NativeHooksMethods[] = {
       {"write", NativeHooks_write, METH_VARARGS, "Write to android log"},
//        {"flush", NativeHooks_flush, METH_VARARGS, "Required for logging"},
//        {"publish", NativeHooks_publish, METH_VARARGS, "Send events to the Java implementation"},
        {"load_module", NativeHooks_load_module, METH_VARARGS, "Load an extension"},
        {"find_module", NativeHooks_find_module, METH_VARARGS, "Find an extension"},
        {NULL, NULL, 0, NULL}
};



static struct PyModuleDef NativeHooksModule = {
        PyModuleDef_HEAD_INIT,
        "nativehooks",        /* m_name */
        "Android native hooks",   /* m_doc */
        -1,                  /* m_size */
        NativeHooksMethods    /* m_methods */
};


// assetsPath, cachePath, nativePath);
extern "C"
JNIEXPORT void JNICALL
Java_com_pyjsi_PyJsiModule_nativeInstall(JNIEnv *env, jclass clazz, jlong jsi_ptr, jstring assets_path, jstring cache_path, jstring jni_path) {


    LOG("Initializing the Python interpreter");

    // This forwards NDK stdout output to logcat
    start_logger();


    // Get the location of the python files
    const char *assetspath = (env)->GetStringUTFChars(assets_path, NULL);
    const char *cachepath = (env)->GetStringUTFChars(cache_path, NULL);
    const char *jnipath = (env)->GetStringUTFChars(jni_path, NULL);
    LOG(cachepath);
    LOG(assetspath);
    LOG(jnipath);

    // Build paths for the Python interpreter
    char paths[512];
    // Remove extra paths, the smaller the search path the faster the import
    snprintf(paths, sizeof(paths), "%s:%s/python:%s/python/site-packages", assetspath, assetspath,
             assetspath);


    LOG(paths);

    // Set JNI path
    setenv("ASSETS", assetspath, 1);
    setenv("PYTHONPATH", paths, 1);
    setenv("TMP", cachepath, 1);
    setenv("PY_LIB_DIR", jnipath, 1);

    Py_Initialize();


    // load native hooks
    PyObject *imp = PyImport_ImportModule("importlib.machinery");

    // Build extension dict
    mExtensions = PyDict_New();
    if (mExtensions != NULL) {
        char *lib_dir = getenv("PY_LIB_DIR");
        unsigned int nlib = strlen(lib_dir);
        unsigned int n;
        DIR *dir;
        char mod[256];// lib. .so+'\0'
        char path[256];
        struct dirent *ent;
        if ((dir = opendir(lib_dir)) != NULL) {
            // print all the files and directories within directory
            while ((ent = readdir(dir)) != NULL) {
                // If startswith lib. strip mod
                if (ent->d_type == DT_REG && strncmp("lib.", ent->d_name, 4) == 0) {
                    LOG(ent->d_name);
                    n = strlen(ent->d_name);
                    unsigned int mod_end = n - 7;
                    unsigned int path_end = nlib + n + 1;
                    if (mod_end > 255 || path_end > 255)
                        continue; // out of bounds
                    strncpy(mod, ent->d_name + 4, mod_end);
                    mod[mod_end] = '\0';
                    strcpy(path, lib_dir);
                    strcat(path, "/");
                    strcat(path, ent->d_name);
                    path[path_end] = '\0';
                    PyDict_SetItem(mExtensions, Py_BuildValue("s", mod), Py_BuildValue("s", path));
                }
            }
            closedir(dir);
        } else {
            /* could not open directory */
            LOG("Python extension dir is invalid");
        }
    } else {
        LOG("Extension modules null");
    }


    // Add to meta path
    Py_XDECREF(mNativehooksModule);

    PyObject* meta_path = PySys_GetObject("meta_path");
    PyObject* result = PyObject_CallMethod(meta_path, "append", "O", mNativehooksModule);

    Py_XDECREF(result);
    Py_XDECREF(imp);
    // mNativehooksModule



    mNativehooksModule = PyModule_Create(&NativeHooksModule);

    // Redirect stdout and stderr to this module
    PySys_SetObject("stdout", mNativehooksModule);
    PySys_SetObject("stderr", mNativehooksModule);

    // hello world from python
     PyRun_SimpleString(
            "try:\n"\
        "    import sys\n"\
        "    import os\n"\
        "    sys.path = [p for p in sys.path if not p.startswith('/usr/local')] \n"\
        "    print(f'Sys path: {sys.path}')\n"\
        "    print(f'Environ: {os.environ}')\n"\
        "except Exception as e:\n"\
        "    try:\n"\
        "        import traceback\n"\
        "        traceback.print_exc()\n"\
        "    except:\n"\
        "        print(e)\n" \
    );


    // Cleanup
    (env)->ReleaseStringUTFChars(assets_path, assetspath);
    (env)->ReleaseStringUTFChars(cache_path, cachepath);
    (env)->ReleaseStringUTFChars( jni_path, jnipath);


}

