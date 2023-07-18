cmake_minimum_required(VERSION 3.4.1)
project(pyjsi)
set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 17)


set(PYTHON_LIB ${CMAKE_SOURCE_DIR}/../externals/${ANDROID_ABI}/lib/libpython3.10.so)
set(PYTHON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../externals/${ANDROID_ABI}/include/python3.10)


set(LIB_CRYPT ${CMAKE_SOURCE_DIR}/../externals/${ANDROID_ABI}/lib/libcrypt.so)


add_library(python SHARED IMPORTED)
set_target_properties(python PROPERTIES IMPORTED_LOCATION ${PYTHON_LIB})

add_library(crypt SHARED IMPORTED)
set_target_properties(crypt PROPERTIES IMPORTED_LOCATION ${LIB_CRYPT})


add_library(pyjsi
            SHARED
            ../cpp/react-native-pyjsi.cpp
            cpp-adapter.cpp
)

# Specifies a path to native header files.
include_directories(
            ../cpp
            ${PYTHON_INCLUDE_DIR}
)

find_package(ReactAndroid REQUIRED CONFIG)


target_link_libraries(pyjsi crypt python ReactAndroid::jsi android log)
