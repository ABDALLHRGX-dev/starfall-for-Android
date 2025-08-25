#include <jni.h>
#include <android/log.h>
#include "base.h"
#include "request.h"
#include "exit.h"
#include "opts.h"

#define LOG_TAG "Starfall"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Define Engine version dynamically (default to UE5)
enum class EngineType { UE4, UE5 };
static EngineType CurrentEngine = EngineType::UE5;

extern "C" {

// -------------------- UE4 functions --------------------
JNIEXPORT void JNICALL Java_com_epicgames_ue4_GameActivity_nativeOnActivityCreated(JNIEnv* env, jobject obj) {
    if (CurrentEngine == EngineType::UE4) {
        if (!Starfall::IsInitialized()) {
            LOGI("Initializing Starfall library (UE4)...");
            Starfall::Init();
            LOGI("Starfall initialized successfully (UE4)!");
        } else {
            LOGI("Starfall already initialized (UE4).");
        }
    }
}

JNIEXPORT jboolean JNICALL Java_com_epicgames_ue4_GameActivity_nativeIsStarfallInitialized(JNIEnv* env, jobject obj) {
    if (CurrentEngine == EngineType::UE4)
        return Starfall::IsInitialized() ? JNI_TRUE : JNI_FALSE;
    return JNI_FALSE;
}

JNIEXPORT void JNICALL Java_com_epicgames_ue4_GameActivity_nativeReloadStarfall(JNIEnv* env, jobject obj) {
    if (CurrentEngine == EngineType::UE4) {
        if (Starfall::IsInitialized()) {
            LOGI("Shutting down Starfall for reload (UE4)...");
            Starfall::Shutdown();
        }
        Starfall::Init();
        LOGI("Starfall reloaded successfully (UE4)!");
    }
}

// -------------------- UE5 functions --------------------
JNIEXPORT void JNICALL Java_com_epicgames_unreal_GameActivity_nativeOnActivityCreated(JNIEnv* env, jobject obj) {
    if (CurrentEngine == EngineType::UE5) {
        if (!Starfall::IsInitialized()) {
            LOGI("Initializing Starfall library (UE5)...");
            Starfall::Init();
            LOGI("Starfall initialized successfully (UE5)!");
        } else {
            LOGI("Starfall already initialized (UE5).");
        }
    }
}

JNIEXPORT jboolean JNICALL Java_com_epicgames_unreal_GameActivity_nativeIsStarfallInitialized(JNIEnv* env, jobject obj) {
    if (CurrentEngine == EngineType::UE5)
        return Starfall::IsInitialized() ? JNI_TRUE : JNI_FALSE;
    return JNI_FALSE;
}

JNIEXPORT void JNICALL Java_com_epicgames_unreal_GameActivity_nativeReloadStarfall(JNIEnv* env, jobject obj) {
    if (CurrentEngine == EngineType::UE5) {
        if (Starfall::IsInitialized()) {
            LOGI("Shutting down Starfall for reload (UE5)...");
            Starfall::Shutdown();
        }
        Starfall::Init();
        LOGI("Starfall reloaded successfully (UE5)!");
    }
}

JNIEXPORT void JNICALL Java_com_epicgames_unreal_GameActivity_nativeHandleTouch(JNIEnv* env, jobject obj, jint x, jint y) {
    if (CurrentEngine == EngineType::UE5) {
        Starfall::HandleTouch(x, y);
        LOGI("UE5 touch event at (%d,%d)", x, y);
    }
}

JNIEXPORT void JNICALL Java_com_epicgames_unreal_GameActivity_nativeOnPause(JNIEnv* env, jobject obj) {
    if (CurrentEngine == EngineType::UE5) {
        Starfall::OnPause();
        LOGI("Starfall paused (UE5).");
    }
}

JNIEXPORT void JNICALL Java_com_epicgames_unreal_GameActivity_nativeOnResume(JNIEnv* env, jobject obj) {
    if (CurrentEngine == EngineType::UE5) {
        Starfall::OnResume();
        LOGI("Starfall resumed (UE5).");
    }
}

JNIEXPORT void JNICALL Java_com_epicgames_unreal_GameActivity_nativeLogMessage(JNIEnv* env, jobject obj, jstring msg) {
    if (CurrentEngine == EngineType::UE5 && msg) {
        const char* cmsg = env->GetStringUTFChars(msg, nullptr);
        LOGI("UE5 log: %s", cmsg);
        env->ReleaseStringUTFChars(msg, cmsg);
    }
}

JNIEXPORT void JNICALL Java_com_epicgames_unreal_GameActivity_nativeUE5SpecialFunction(JNIEnv* env, jobject obj) {
    if (CurrentEngine == EngineType::UE5)
        LOGI("UE5 special function called.");
}

} // extern "C"
