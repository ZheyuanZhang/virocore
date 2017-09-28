#ifndef TYPES_JNI_h
#define TYPES_JNI_h

#include <cstddef>
#include <cassert>
#include <memory>
#include <stdexcept>

#include <jni.h>

namespace jni
   {
    // Not using bool because its size is implementation-defined, and a defined size
    // is necessary for array operations.

    using ::jboolean;

    const jboolean jni_false = 0;
    const jboolean jni_true  = 1;

    using ::jbyte;
    using ::jchar;
    using ::jshort;
    using ::jint;
    using ::jlong;
    using ::jfloat;
    using ::jdouble;

    using jsize = std::size_t;

    struct jobject
       {
         jobject()                          = delete;
        ~jobject()                          = delete;
         jobject(const jobject&)            = delete;
         jobject& operator=(const jobject&) = delete;
       };

    struct jclass     : public jobject {};
    struct jstring    : public jobject {};
    struct jthrowable : public jobject {};

    template < class T >
    struct jarray : public jobject {};

    using jobjectArray  = jarray< jobject  >;
    using jbooleanArray = jarray< jboolean >;
    using jbyteArray    = jarray< jbyte    >;
    using jcharArray    = jarray< jchar    >;
    using jshortArray   = jarray< jshort   >;
    using jintArray     = jarray< jint     >;
    using jlongArray    = jarray< jlong    >;
    using jfloatArray   = jarray< jfloat   >;
    using jdoubleArray  = jarray< jdouble  >;

    using jfieldID  = std::pointer_traits< ::jfieldID  >::element_type;
    using jmethodID = std::pointer_traits< ::jmethodID >::element_type;

    using ::jobjectRefType;

    using ::JavaVM;
    using ::JNIEnv;

    template < class T >
    struct JNINativeMethod;

    template < class R, class... Args >
    struct JNINativeMethod< R (JNIEnv*, jclass*, Args...) >
       {
        const char* name;
        const char* signature;
        R (*fnPtr)(JNIEnv*, jclass*, Args...);
       };

    template < class R, class... Args >
    struct JNINativeMethod< R (JNIEnv*, jobject*, Args...) >
       {
        const char* name;
        const char* signature;
        R (*fnPtr)(JNIEnv*, jobject*, Args...);
       };

    enum class version : jint {};

    const version jni_version_1_1 = version(JNI_VERSION_1_1);
    const version jni_version_1_2 = version(JNI_VERSION_1_2);
    const version jni_version_1_4 = version(JNI_VERSION_1_4);
    const version jni_version_1_6 = version(JNI_VERSION_1_6);

    enum class error : jint {};

    const error jni_ok        = error(JNI_OK);
    const error jni_err       = error(JNI_ERR);
    const error jni_edetached = error(JNI_EDETACHED);
    const error jni_eversion  = error(JNI_EVERSION);
   }

#endif
