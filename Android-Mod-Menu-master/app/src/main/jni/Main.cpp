#include <list>
#include <vector>
#include <string.h>
#include <pthread.h>
#include <cstring>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include "KittyMemory/MemoryPatch.h"
#include "Includes/Logger.h"
#include "Includes/Utils.h"
#include "Includes/obfuscate.h"

#include "Menu.h"

#include "Toast.h"

#if defined(__aarch64__) //Compile for arm64 lib only
#include <And64InlineHook/And64InlineHook.hpp>
#else //Compile for armv7 lib only. Do not worry about greyed out highlighting code, it still works

#include <Substrate/SubstrateHook.h>
#include <Substrate/CydiaSubstrate.h>

#endif

// fancy struct for patches for kittyMemory
struct My_Patches {
    MemoryPatch GodMode, GodMode2, SliderExample;
} hexPatches;


bool feature1 = false;
bool feature2 = false;
bool featureHookToggle = false;


int sliderValue = 1;
int dmgmul = 1;
int defmul = 1;
void *instanceBtn;


// See https://guidedhacking.com/threads/android-function-pointers-hooking-template-tutorial.14771/
void (*AddMoneyExample)(void *instance, int amount);

//Target lib here
#define targetLibName OBFUSCATE("libil2cpp.so")

extern "C" {
JNIEXPORT void JNICALL
Java_uk_lgl_modmenu_Preferences_Changes(JNIEnv *env, jclass clazz, jobject obj,
                                        jint feature, jint value, jboolean boolean, jstring str) {

    const char *featureName = env->GetStringUTFChars(str, 0);
    feature += 1;  // No need to count from 0 anymore. yaaay :)))

    LOGD(OBFUSCATE("Feature name: %d - %s | Value: = %d | Bool: = %d"), feature, featureName, value,
         boolean);

    //!!! BE CAREFUL NOT TO ACCIDENTLY REMOVE break; !!!//

    switch (feature) {
        case 1:
            // The category was 1 so is not used
            break;
        case 2:
            feature2 = boolean;
            if (feature2) {
                hexPatches.GodMode.Modify();
                hexPatches.GodMode2.Modify();
            } else {
                hexPatches.GodMode.Restore();
                hexPatches.GodMode2.Restore();
            }
            break;
        case 3:
            if (value >= 1) {
                sliderValue = value;
            }
            break;
        case 4:
            switch (value) {
                case 0:
                    hexPatches.SliderExample = MemoryPatch::createWithHex(
                            targetLibName, string2Offset(
                                    OBFUSCATE_KEY("0x100000", 't')),
                            OBFUSCATE(
                                    "00 00 A0 E3 1E FF 2F E1"));
                    hexPatches.SliderExample.Modify();
                    break;
                case 1:
                    hexPatches.SliderExample = MemoryPatch::createWithHex(
                            targetLibName, string2Offset(
                                    OBFUSCATE_KEY("0x100000",
                                                  'b')),
                            OBFUSCATE(
                                    "01 00 A0 E3 1E FF 2F E1"));
                    hexPatches.SliderExample.Modify();
                    break;
                case 2:
                    hexPatches.SliderExample = MemoryPatch::createWithHex(
                            targetLibName,
                            string2Offset(
                                    OBFUSCATE_KEY("0x100000",
                                                  'q')),
                            OBFUSCATE(
                                    "02 00 A0 E3 1E FF 2F E1"));
                    hexPatches.SliderExample.Modify();
                    break;
            }
            break;
        case 5:
            switch (value) {
                case 0:
                    LOGD(OBFUSCATE("Selected item 1"));
                    break;
                case 1:
                    LOGD(OBFUSCATE("Selected item 2"));
                    break;
                case 2:
                    LOGD(OBFUSCATE("Selected item 3"));
                    break;
            }
            break;
        case 6:
            LOGD(OBFUSCATE("Feature 6 Called"));
            // See more https://guidedhacking.com/threads/android-function-pointers-hooking-template-tutorial.14771/
            if (instanceBtn != NULL)
                AddMoneyExample(instanceBtn, 999999);
            //MakeToast(env, obj, OBFUSCATE("Button pressed"), Toast::LENGTH_SHORT); //When the button is pressed it shows a message
            break;
        case 8:
            LOGD(OBFUSCATE("Feature 8 Called"));
            featureHookToggle = boolean;
            break;
    }
}
}

// ---------- Hooking ---------- //

bool (*old_get_BoolExample)(void *instance);

bool get_BoolExample(void *instance) {
    if (instance != NULL && featureHookToggle) {
        return true;
    }
    return old_get_BoolExample(instance);
}

float (*old_get_FloatExample)(void *instance);

float get_FloatExample(void *instance) {
    if (instance != NULL && sliderValue > 1) {
        return (float) sliderValue;
    }
    return old_get_FloatExample(instance);
}

void (*old_Update)(void *instance);

void Update(void *instance) {
    instanceBtn = instance;
    old_Update(instance);
}

void *hack_thread(void *) {
    LOGI(OBFUSCATE("pthread called"));

    //Check if target lib is loaded
    do {
        sleep(1);
    } while (!isLibraryLoaded(targetLibName));

    LOGI(OBFUSCATE("%s has been loaded"), (const char *) targetLibName);

#if defined(__aarch64__) //Compile for arm64 lib only.
    // New way to patch hex via KittyMemory without need to  specify len. Spaces or without spaces are fine
    hexPatches.GodMode = MemoryPatch::createWithHex(targetLibName,
                                                    string2Offset(OBFUSCATE_KEY("0x123456", '3')),
                                                    OBFUSCATE("00 00 A0 E3 1E FF 2F E1"));

    // Offset Hook example
    A64HookFunction((void *) getAbsoluteAddress(targetLibName, string2Offset(OBFUSCATE_KEY("0x123456", 'l'))), (void *) get_BoolExample,
                    (void **) &old_get_BoolExample);

    // Function pointer splitted because we want to avoid crash when the il2cpp lib isn't loaded.
    // See https://guidedhacking.com/threads/android-function-pointers-hooking-template-tutorial.14771/
    AddMoneyExample = (void(*)(void *,int))getAbsoluteAddress(targetLibName, 0x123456);

#else //Compile for armv7 lib only.

    hexPatches.GodMode = MemoryPatch::createWithHex(targetLibName,
                                                    string2Offset(OBFUSCATE_KEY("0x123456", '-')),
                                                    OBFUSCATE("00 00 A0 E3 1E FF 2F E1"));

    //hexPatches.GodMode.Modify(); //Activates Automatically


    hexPatches.GodMode2 = MemoryPatch::createWithHex("libtargetLibHere.so", //The lib you want
                                                     string2Offset(OBFUSCATE_KEY("0x222222", 'g')),
                                                     OBFUSCATE("00 00 A0 E3 1E FF 2F E1"));

    //hexPatches.GodMode2.Modify(); //Activates Automatically


    MSHookFunction(
            (void *) getAbsoluteAddress(targetLibName,
                                        string2Offset(OBFUSCATE_KEY("0x123456", '?'))),
            (void *) get_BoolExample,
            (void **) &old_get_BoolExample);

    // Symbol hook example (untested). Symbol/function names can be found in IDA if the lib are not stripped. This is not for il2cpp games
    MSHookFunction((void *) ("__SymbolNameExample"), (void *) get_BoolExample,
                   (void **) &old_get_BoolExample);


    AddMoneyExample = (void (*)(void *, int)) getAbsoluteAddress(targetLibName, 0x123456);

    LOGI(OBFUSCATE("Hooked"));
#endif

    return NULL;
}
__attribute__((constructor))
void lib_main() {
    // Create a new thread so it does not block the main thread, means the game would not freeze
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);
}

/*
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *globalEnv;
    vm->GetEnv((void **) &globalEnv, JNI_VERSION_1_6);

    return JNI_VERSION_1_6;
}
 */
