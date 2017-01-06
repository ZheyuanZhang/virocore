//
//  VRORenderer_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_RENDERER_JNI_H
#define ANDROID_RENDERER_JNI_H

#include <jni.h>
#include <memory>
#include <VROSceneRenderer.h>
#include "PersistentRef.h"

namespace Renderer{
    inline jlong jptr(std::shared_ptr<VROSceneRenderer> native_renderer) {
        PersistentRef<VROSceneRenderer> *persistedRenderer = new PersistentRef<VROSceneRenderer>(native_renderer);
        return reinterpret_cast<intptr_t>(persistedRenderer);
    }

    inline std::shared_ptr<VROSceneRenderer> native(jlong ptr) {
        PersistentRef<VROSceneRenderer> *persistedRenderer = reinterpret_cast<PersistentRef<VROSceneRenderer> *>(ptr);
        return persistedRenderer->get();
    }
}

#endif
