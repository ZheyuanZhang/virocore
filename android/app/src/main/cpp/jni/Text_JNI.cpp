//
//  Text_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include "PersistentRef.h"
#include <VROTypefaceAndroid.h>
#include <VROStringUtil.h>
#include "VROText.h"
#include "Node_JNI.h"
#include "TextDelegate_JNI.h"
#include "ViroContext_JNI.h"
#include "VROPlatformUtil.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_Text_##method_name

namespace Text {
    inline jlong jptr(std::shared_ptr<VROText> shared_node) {
        PersistentRef<VROText> *native_text = new PersistentRef<VROText>(shared_node);
        return reinterpret_cast<intptr_t>(native_text);
    }

    inline std::shared_ptr<VROText> native(jlong ptr) {
        PersistentRef<VROText> *persistentText = reinterpret_cast<PersistentRef<VROText> *>(ptr);
        return persistentText->get();
    }
}

VROTextHorizontalAlignment getHorizontalAlignmentEnum(const std::string& strName) {
    // Depending on string, return the right enum
    if (VROStringUtil::strcmpinsensitive(strName, "Right")) {
        return VROTextHorizontalAlignment::Right;
    } else if (VROStringUtil::strcmpinsensitive(strName, "Center")) {
        return VROTextHorizontalAlignment::Center;
    } else {
        // Default to left alignment
        return VROTextHorizontalAlignment::Left;
    }

}

VROTextVerticalAlignment getVerticalAlignmentEnum(const std::string& strName) {
    // Depending on string, return the right enum
    if (VROStringUtil::strcmpinsensitive(strName, "Bottom")) {
        return VROTextVerticalAlignment::Bottom;
    } else if (VROStringUtil::strcmpinsensitive(strName, "Center")) {
        return VROTextVerticalAlignment::Center;
    } else {
        // Default to Top alignment
        return VROTextVerticalAlignment::Top;
    }

}

VROLineBreakMode getLineBreakModeEnum(const std::string& strName) {
    // Depending on string, return the right enum
    if (VROStringUtil::strcmpinsensitive(strName, "WordWrap")) {
        return VROLineBreakMode::WordWrap;
    } else if (VROStringUtil::strcmpinsensitive(strName, "CharWrap")) {
        return VROLineBreakMode::CharWrap;
    } else if (VROStringUtil::strcmpinsensitive(strName, "Justify")) {
        return VROLineBreakMode::Justify;
    } else {
        // Default to none
        return VROLineBreakMode::None;
    }
}

VROTextClipMode getTextClipModeEnum(const std::string& strName) {
    // Depending on string, return the right enum
    if (VROStringUtil::strcmpinsensitive(strName, "ClipToBounds")) {
        return VROTextClipMode::ClipToBounds;
    } else {
        return VROTextClipMode::None;
    }
}

extern "C" {

JNI_METHOD(jlong, nativeCreateText)(JNIEnv *env,
                                    jobject object,
                                    jlong context_j,
                                    jstring text_j,
                                    jstring fontFamily_j,
                                    jint size,
                                    jlong color,
                                    jfloat width,
                                    jfloat height,
                                    jstring horizontalAlignment_j,
                                    jstring verticalAlignment_j,
                                    jstring lineBreakMode_j,
                                    jstring clipMode_j,
                                    jint maxLines) {
    // Get the text string
    const jchar *text_c = env->GetStringChars(text_j, NULL);
    jsize textLength = env->GetStringLength(text_j);

    std::wstring text;
    text.assign(text_c, text_c + textLength);
    env->ReleaseStringChars(text_j, text_c);

    // Get the color
    float a = ((color >> 24) & 0xFF) / 255.0;
    float r = ((color >> 16) & 0xFF) / 255.0;
    float g = ((color >> 8) & 0xFF) / 255.0;
    float b = (color & 0xFF) / 255.0;
    VROVector4f vecColor(r, g, b, a);

    // Get horizontal alignment
    const char *horizontalAlignment_c = env->GetStringUTFChars(horizontalAlignment_j, NULL);
    std::string horizontalAlignment_s(horizontalAlignment_c);
    VROTextHorizontalAlignment horizontalAlignment = getHorizontalAlignmentEnum(horizontalAlignment_s);
    env->ReleaseStringUTFChars(horizontalAlignment_j, horizontalAlignment_c);

    // Get vertical alignment
    const char *verticalAlignment_c = env->GetStringUTFChars(verticalAlignment_j, NULL);
    std::string verticalAlignment_s(verticalAlignment_c);
    VROTextVerticalAlignment verticalAlignment = getVerticalAlignmentEnum(verticalAlignment_s);
    env->ReleaseStringUTFChars(verticalAlignment_j, verticalAlignment_c);

    // Get line break mode
    const char *lineBreakMode_c = env->GetStringUTFChars(lineBreakMode_j, NULL);
    std::string lineBreakMode_s(lineBreakMode_c);
    VROLineBreakMode lineBreakMode = getLineBreakModeEnum(lineBreakMode_s);
    env->ReleaseStringUTFChars(lineBreakMode_j, lineBreakMode_c);

    // Get clip mode
    const char *clipMode_c = env->GetStringUTFChars(clipMode_j, NULL);
    std::string clipMode_s(clipMode_c);
    VROTextClipMode clipMode = getTextClipModeEnum(clipMode_s);
    env->ReleaseStringUTFChars(clipMode_j, clipMode_c);

    const char *fontFamily_c = env->GetStringUTFChars(fontFamily_j, NULL);
    std::string fontFamily(fontFamily_c);
    env->ReleaseStringUTFChars(fontFamily_j, fontFamily_c);

    std::shared_ptr<ViroContext> context = ViroContext::native(context_j);
    std::shared_ptr<VRODriver> driver = context->getDriver();
    std::shared_ptr<VROTypeface> typeface = driver.get()->newTypeface(fontFamily, size);

    std::shared_ptr<VROText> vroText = std::make_shared<VROText>(text, typeface, vecColor, width,
                                                                 height, horizontalAlignment,
                                                                 verticalAlignment,
                                                                 lineBreakMode,
                                                                 clipMode, maxLines);

    // Update text on renderer thread (glyph creation requires this)
    VROPlatformDispatchAsyncRenderer([vroText] {
        vroText->update();
    });

    return Text::jptr(vroText);
}

JNI_METHOD(void, nativeDestroyText)(JNIEnv *env,
                                    jobject obj,
                                    jlong text_j) {
    delete reinterpret_cast<PersistentRef<VROText> *>(text_j);
}

JNI_METHOD(void, nativeSetText)(JNIEnv *env,
                                jobject obj,
                                jlong text_j,
                                jstring text_string_j) {

    const jchar *text_c = env->GetStringChars(text_string_j, NULL);
    jsize textLength = env->GetStringLength(text_string_j);

    std::wstring text_string;
    text_string.assign(text_c, text_c + textLength);
    env->ReleaseStringChars(text_string_j, text_c);

    std::weak_ptr<VROText> text_w = Text::native(text_j);
    VROPlatformDispatchAsyncRenderer([text_w, text_string] {
        std::shared_ptr<VROText> text = text_w.lock();
        if (!text) {
            return;
        }
        text->setText(text_string);
    });
}

JNI_METHOD(void, nativeSetFont)(JNIEnv *env,
                                jobject obj,
                                jlong context_j,
                                jlong text_j,
                                jint size,
                                jstring family_j) {

    const char *family_c = env->GetStringUTFChars(family_j, NULL);
    std::string family(family_c);
    env->ReleaseStringUTFChars(family_j, family_c);

    std::shared_ptr<ViroContext> context = ViroContext::native(context_j);
    std::shared_ptr<VROTypeface> typeface = context->getDriver()->newTypeface(family, size);

    std::weak_ptr<VROText> text_w = Text::native(text_j);
    VROPlatformDispatchAsyncRenderer([text_w, typeface] {
        std::shared_ptr<VROText> text = text_w.lock();
        if (!text) {
            return;
        }
        text->setTypeface(typeface);
    });
}

JNI_METHOD(void, nativeSetColor)(JNIEnv *env,
                                 jobject obj,
                                 jlong text_j,
                                 jlong color_j) {

    float a = ((color_j >> 24) & 0xFF) / 255.0;
    float r = ((color_j >> 16) & 0xFF) / 255.0;
    float g = ((color_j >> 8) & 0xFF) / 255.0;
    float b = (color_j & 0xFF) / 255.0;
    VROVector4f color(r, g, b, a);

    std::weak_ptr<VROText> text_w = Text::native(text_j);
    VROPlatformDispatchAsyncRenderer([text_w, color] {
        std::shared_ptr<VROText> text = text_w.lock();
        if (!text) {
            return;
        }
        text->setColor(color);
    });
}

JNI_METHOD(void, nativeSetWidth)(JNIEnv *env,
                                 jobject obj,
                                 jlong text_j,
                                 jfloat width) {

    std::weak_ptr<VROText> text_w = Text::native(text_j);
    VROPlatformDispatchAsyncRenderer([text_w, width] {
        std::shared_ptr<VROText> text = text_w.lock();
        if (!text) {
            return;
        }
        text->setWidth(width);
    });
}

JNI_METHOD(void, nativeSetHeight)(JNIEnv *env,
                                  jobject obj,
                                  jlong text_j,
                                  jfloat height) {

    std::weak_ptr<VROText> text_w = Text::native(text_j);
    VROPlatformDispatchAsyncRenderer([text_w, height] {
        std::shared_ptr<VROText> text = text_w.lock();
        if (!text) {
            return;
        }
        text->setHeight(height);
    });
}

JNI_METHOD(void, nativeSetHorizontalAlignment)(JNIEnv *env,
                                               jobject obj,
                                               jlong text_j,
                                               jstring horizontalAlignment_j) {

    const char *horizontalAlignment_c = env->GetStringUTFChars(horizontalAlignment_j, NULL);
    std::string horizontalAlignment_s(horizontalAlignment_c);
    VROTextHorizontalAlignment horizontalAlignment = getHorizontalAlignmentEnum(horizontalAlignment_s);
    env->ReleaseStringUTFChars(horizontalAlignment_j, horizontalAlignment_c);

    std::weak_ptr<VROText> text_w = Text::native(text_j);
    VROPlatformDispatchAsyncRenderer([text_w, horizontalAlignment] {
        std::shared_ptr<VROText> text = text_w.lock();
        if (!text) {
            return;
        }
        text->setHorizontalAlignment(horizontalAlignment);
    });
}

JNI_METHOD(void, nativeSetVerticalAlignment)(JNIEnv *env,
                                             jobject obj,
                                             jlong text_j,
                                             jstring verticalAlignment_j) {

    const char *verticalAlignment_c = env->GetStringUTFChars(verticalAlignment_j, NULL);
    std::string verticalAlignment_s(verticalAlignment_c);
    VROTextVerticalAlignment verticalAlignment = getVerticalAlignmentEnum(verticalAlignment_s);
    env->ReleaseStringUTFChars(verticalAlignment_j, verticalAlignment_c);

    std::weak_ptr<VROText> text_w = Text::native(text_j);
    VROPlatformDispatchAsyncRenderer([text_w, verticalAlignment] {
        std::shared_ptr<VROText> text = text_w.lock();
        if (!text) {
            return;
        }
        text->setVerticalAlignment(verticalAlignment);
    });
}

JNI_METHOD(void, nativeSetLineBreakMode)(JNIEnv *env,
                                         jobject obj,
                                         jlong text_j,
                                         jstring lineBreakMode_j) {

    const char *lineBreakMode_c = env->GetStringUTFChars(lineBreakMode_j, NULL);
    std::string lineBreakMode_s(lineBreakMode_c);
    VROLineBreakMode lineBreakMode = getLineBreakModeEnum(lineBreakMode_s);
    env->ReleaseStringUTFChars(lineBreakMode_j, lineBreakMode_c);

    std::weak_ptr<VROText> text_w = Text::native(text_j);
    VROPlatformDispatchAsyncRenderer([text_w, lineBreakMode] {
        std::shared_ptr<VROText> text = text_w.lock();
        if (!text) {
            return;
        }
        text->setLineBreakMode(lineBreakMode);
    });
}

JNI_METHOD(void, nativeSetClipMode)(JNIEnv *env,
                                    jobject obj,
                                    jlong text_j,
                                    jstring clipMode_j) {

    // Get clip mode
    const char *clipMode_c = env->GetStringUTFChars(clipMode_j, NULL);
    std::string clipMode_s(clipMode_c);
    VROTextClipMode clipMode = getTextClipModeEnum(clipMode_s);
    env->ReleaseStringUTFChars(clipMode_j, clipMode_c);

    std::weak_ptr<VROText> text_w = Text::native(text_j);
    VROPlatformDispatchAsyncRenderer([text_w, clipMode] {
        std::shared_ptr<VROText> text = text_w.lock();
        if (!text) {
            return;
        }
        text->setClipMode(clipMode);
    });
}

JNI_METHOD(void, nativeSetMaxLines)(JNIEnv *env,
                                    jobject obj,
                                    jlong text_j,
                                    jint maxLines) {

    std::weak_ptr<VROText> text_w = Text::native(text_j);
    VROPlatformDispatchAsyncRenderer([text_w, maxLines] {
        std::shared_ptr<VROText> text = text_w.lock();
        if (!text) {
            return;
        }
        text->setMaxLines(maxLines);
    });
}

} // extern "C"

