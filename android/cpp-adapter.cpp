#include <jni.h>
#include "react-native-pyjsi.h"
#include <iostream>
#include "Python.h"
#include <android/log.h>
#include <dirent.h>
#include <jsi/jsi.h>
#include "Logger.h"
#include "py-ext-register.h"


#define PY_SSIZE_T_CLEAN
static const char *tag = "pyjsi-bridge";

static PyObject *mImpLoadDynamic;
static PyObject *mNativehooksModule;

static int pfd[2];
static pthread_t mLogThread;

using namespace facebook;

facebook::jsi::Runtime *globalRuntime;
#define LOG(x) logMessage(*globalRuntime, tag, x);

/**
 * Pipe to catch output from stdout and log it
 */
struct ThreadParams
{
  const char *tag;
  facebook::jsi::Runtime *runtime;
};

static void *thread_func(void *arg)
{
  ThreadParams *params = (ThreadParams *)arg;

  ssize_t rdsz;
  char buf[512];
  while ((rdsz = read(pfd[0], buf, sizeof buf - 1)) > 0)
  {
    if (buf[rdsz - 1] == '\n')
      --rdsz;
    buf[rdsz] = 0; /* add null-terminator */
    __android_log_print(ANDROID_LOG_DEBUG, params->tag, "%s", buf);
  }
  return 0;
}

/**
 * Create a pipe and spawn a thread to catch output from nkd libraries
 * and send it to androids log.
 *
 * Thanks to https://codelab.wordpress.com/2014/11/03/how-to-use-standard-output-streams-for-logging-in-android-apps/
 */
int start_logger(facebook::jsi::Runtime &runtime)
{
  /* make stdout line-buffered and stderr unbuffered */
  setvbuf(stdout, 0, _IOLBF, 0);
  setvbuf(stderr, 0, _IONBF, 0);

  /* create the pipe and redirect stdout and stderr */
  pipe(pfd);
  dup2(pfd[1], 1);
  dup2(pfd[1], 2);

  ThreadParams *params = new ThreadParams();
  params->tag = tag;
  params->runtime = &runtime;

  /* spawn the logging thread */
  if (pthread_create(&mLogThread, 0, thread_func, params) == -1)
    return -1;
  pthread_detach(mLogThread);
  return 0;
}



// assetsPath, cachePath, nativePath);
extern "C" JNIEXPORT void JNICALL
Java_com_pyjsi_PyJsiModule_nativeInstall(JNIEnv *env, jclass clazz, jlong jsi_ptr, jstring assets_path, jstring cache_path, jstring jni_path)
{

  auto runtime = reinterpret_cast<facebook::jsi::Runtime *>(jsi_ptr);

  if (runtime == nullptr)
  {
    LOG("Runtime is null");
    return;
  }

  globalRuntime = runtime;


  installLogger(*globalRuntime);
  LOG("Installing native hooks");

  // This forwards NDK stdout output to logcat
  start_logger(*globalRuntime);

  // Get the location of the python files
  const char *assetspath = (env)->GetStringUTFChars(assets_path, NULL);
  const char *cachepath = (env)->GetStringUTFChars(cache_path, NULL);
  const char *jnipath = (env)->GetStringUTFChars(jni_path, NULL);

  // Build paths for the Python interpreter
  char paths[512];
  // Remove extra paths, the smaller the search path the faster the import
  snprintf(paths, sizeof(paths), "%s:%s/python:%s/python/site-packages", assetspath, assetspath,
           assetspath);


  setenv("ASSETS", assetspath, 1);
  setenv("PYTHONPATH", paths, 1);
  setenv("TMP", cachepath, 1);
  setenv("PYTHONHOME", jnipath, 1);
  setenv("PY_LIB_DIR", jnipath, 1);

  char* lib_dir = getenv("PY_LIB_DIR");

  PyExtension myExtension(lib_dir);
  PyImport_AppendInittab("hello", PyInit_hello);
  Py_Initialize();

    PyRun_SimpleString(
      "try:\n"
      "    import hello\n"
      "    import sys\n"
      "    import os\n"
      "    import math\n"
      "    a = math.sqrt(4)\n"
      "    import numpy as np\n"
      "    arr = np.array([1, 2, 3])\n"
      "    print(type(arr))\n"
      "    print(f'--> {arr}')\n"
      "    print(f'--> {a}')\n"
      "    print(dir(hello))\n"
      "    #print(f'--> {hello.find_module('numpy')}')\n"
      "    sys.path = [p for p in sys.path if not p.startswith('/usr/local')] \n"
      "    #print(f'Sys path: {sys.path}')\n"
      "    #print(f'modules {sys.modules}')\n"
      "    print(f'Hello: {hello.say_hello()}')\n"
      "    #print(f'Environ: {os.environ}')\n"
      "except Exception as e:\n"
      "    try:\n"
      "        import traceback\n"
      "        traceback.print_exc()\n"
      "    except:\n"
      "        print(e)\n");


    Py_Finalize();
}


