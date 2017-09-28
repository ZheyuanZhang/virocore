#ifndef MAKE_JNI_h
#define MAKE_JNI_h

namespace jni
   {
    template < class Result > struct ThingToMake {};

    template < class Result, class... P >
    Result Make(P&&... p)
       {
        return MakeAnything(ThingToMake<Result>(), std::forward<P>(p)...);
       }
   }

#endif
