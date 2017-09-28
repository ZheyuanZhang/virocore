#ifndef FIELD_JNI_h
#define FIELD_JNI_h

#include <jni/functions.hpp>
#include <jni/class.hpp>
#include <jni/object.hpp>
#include <jni/type_signature.hpp>
#include <jni/tagging.hpp>

namespace jni
   {
    template < class TheTag, class T >
    class Field
       {
        private:
            jfieldID& field;

        public:
            using TagType = TheTag;

            Field(JNIEnv& env, const Class<TagType>& clazz, const char* name)
              : field(GetFieldID(env, clazz, name, TypeSignature<T>()()))
               {}

            operator jfieldID&() const { return field; }
       };
   }

#endif
