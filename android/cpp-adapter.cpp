#include <jni.h>
#include "react-native-pyjsi.h"
#include <iostream>
#include "Python.h"


extern "C"
JNIEXPORT void JNICALL
Java_com_pyjsi_PyJsiModule_nativeInstall(JNIEnv *env, jclass clazz, jlong jsi_ptr,
                                         jstring python_path) {
    const char *python_path_str = env->GetStringUTFChars(python_path, nullptr);

    std::cout << "python_path_str: " << python_path_str << std::endl;

    auto pyVersion = Py_GetVersion();

    std::cout << "python version: " << pyVersion << std::endl;

}

