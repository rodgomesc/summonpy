#include <jni.h>
#include "react-native-pyjsi.h"

extern "C"
JNIEXPORT jdouble JNICALL
Java_com_pyjsi_PyjsiModule_nativeMultiply(JNIEnv *env, jclass type, jdouble a, jdouble b) {
    return pyjsi::multiply(a, b);
}
