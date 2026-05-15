/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the 
"Apache License"); you may not use this file except in compliance with the 
Apache License. You may obtain a copy of the Apache License at 
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Copyright (c) 2026 Audiokinetic Inc.
*******************************************************************************/

#pragma once

#include <openxr/openxr.h>

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkSoundEngineExport.h>


namespace AK
{
    AK_CALLBACK(XrResult, _akmotionXrResultToString)(XrInstance instance, XrResult value, char *buffer);
    AK_CALLBACK(XrResult, _akmotionXrEnumerateInstanceExtensionProperties)(const char *layerName, uint32_t propertyCapacityInput, uint32_t *propertyCountOutput, XrExtensionProperties *properties);
    AK_CALLBACK(XrResult, _akmotionXrStringToPath)(XrInstance instance, const char* pathString, XrPath* path);
    AK_CALLBACK(XrResult, _akmotionXrGetInstanceProcAddr)(XrInstance instance, const char* name, PFN_xrVoidFunction* function);
    AK_CALLBACK(XrResult, _akmotionXrApplyHapticFeedback)(XrSession session, const XrHapticActionInfo* hapticActionInfo, const XrHapticBaseHeader* hapticFeedback);
}

// Helper functions to facilitate passing OpenXR functions and handles to the AkMotionSink.
// If AkMotion is statically linked into your program, call AKMOTIONSINK_SET_OPENXR_FUNCTIONS_AND_HANDLES_WWISE_STATIC inside a function, after XrInstance, XrSession and XrAction handles are available.
// If AkMotion is dynamically linked into your program, call AKMOTIONSINK_SET_OPENXR_FUNCTIONS_AND_HANDLES_WWISE_DYNAMIC after Init.bnk has been loaded and XrInstance, XrSession and XrAction handles are available.
#define AKMOTIONSINK_SET_OPENXR_FUNCTIONS_AND_HANDLES_WWISE_STATIC(Instance, Session, Action) \
    AkMotionInitializeOpenXRFunctions(\
        (AK::_akmotionXrResultToString)xrResultToString, \
        (AK::_akmotionXrEnumerateInstanceExtensionProperties)xrEnumerateInstanceExtensionProperties, \
        (AK::_akmotionXrStringToPath)xrStringToPath, \
        (AK::_akmotionXrGetInstanceProcAddr)xrGetInstanceProcAddr, \
        (AK::_akmotionXrApplyHapticFeedback)xrApplyHapticFeedback); \
    AkMotionSetOpenXRHapticAction(Instance, Session, Action); \

#if defined(_WIN32)
#define AKMOTIONSINK_SET_OPENXR_FUNCTIONS_AND_HANDLES_WWISE_DYNAMIC(Instance, Session, Action) \
    { \
        HMODULE _akmotion_Hmod; \
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, "AkMotion", &_akmotion_Hmod)) \
        { \
            typedef void(*_akmotionOpenXRInitFunc)( \
                AK::_akmotionXrResultToString in_pXrResultToString, \
                AK::_akmotionXrEnumerateInstanceExtensionProperties in_pXrEnumerateInstanceExtensionProperties, \
                AK::_akmotionXrStringToPath in_pXrStringToPath, \
                AK::_akmotionXrGetInstanceProcAddr in_pXrGetInstanceProcAddr, \
                AK::_akmotionXrApplyHapticFeedback in_pXrApplyHapticFeedback); \
            _akmotionOpenXRInitFunc _akmotion_pInitFn = reinterpret_cast<_akmotionOpenXRInitFunc>( reinterpret_cast<void*>( \
                    GetProcAddress(_akmotion_Hmod, "AkMotionInitializeOpenXRFunctions") \
                )); \
            _akmotion_pInitFn( \
                (AK::_akmotionXrResultToString)xrResultToString, \
                (AK::_akmotionXrEnumerateInstanceExtensionProperties)xrEnumerateInstanceExtensionProperties, \
                (AK::_akmotionXrStringToPath)xrStringToPath, \
                (AK::_akmotionXrGetInstanceProcAddr)xrGetInstanceProcAddr, \
                (AK::_akmotionXrApplyHapticFeedback)xrApplyHapticFeedback); \
            typedef void(*_akmotionSetOpenXRHapticActionFunc)(XrInstance in_instance, XrSession in_session, XrAction in_hapticAction); \
            _akmotionSetOpenXRHapticActionFunc _akmotion_pSetOpenXRHapticActionFn = reinterpret_cast<_akmotionSetOpenXRHapticActionFunc>( reinterpret_cast<void*>( \
                    GetProcAddress(_akmotion_Hmod, "AkMotionSetOpenXRHapticAction") \
                )); \
            _akmotion_pSetOpenXRHapticActionFn( \
                Instance, \
                Session, \
                Action); \
        } \
    } \

#elif defined(__ANDROID__)
#include <dlfcn.h>

#define AKMOTIONSINK_SET_OPENXR_FUNCTIONS_AND_HANDLES_WWISE_DYNAMIC(Instance, Session, Action) \
    { \
        void* _akmotion_handle = dlopen("libAkMotion.so", RTLD_LAZY | RTLD_NOLOAD); \
        typedef void(*_akmotionOpenXRInitFunc)( \
            AK::_akmotionXrResultToString in_pXrResultToString, \
            AK::_akmotionXrEnumerateInstanceExtensionProperties in_pXrEnumerateInstanceExtensionProperties, \
            AK::_akmotionXrStringToPath in_pXrStringToPath, \
            AK::_akmotionXrGetInstanceProcAddr in_pXrGetInstanceProcAddr, \
            AK::_akmotionXrApplyHapticFeedback in_pXrApplyHapticFeedback); \
        _akmotionOpenXRInitFunc _akmotion_pInitFn = reinterpret_cast<_akmotionOpenXRInitFunc>( \
                dlsym(_akmotion_handle, "AkMotionInitializeOpenXRFunctions") \
            ); \
        _akmotion_pInitFn( \
            (AK::_akmotionXrResultToString)xrResultToString, \
            (AK::_akmotionXrEnumerateInstanceExtensionProperties)xrEnumerateInstanceExtensionProperties, \
            (AK::_akmotionXrStringToPath)xrStringToPath, \
            (AK::_akmotionXrGetInstanceProcAddr)xrGetInstanceProcAddr, \
            (AK::_akmotionXrApplyHapticFeedback)xrApplyHapticFeedback); \
        typedef void(*_akmotionSetOpenXRHapticActionFunc)(XrInstance in_instance, XrSession in_session, XrAction in_hapticAction); \
        _akmotionSetOpenXRHapticActionFunc _akmotion_pSetOpenXRHapticActionFn = reinterpret_cast<_akmotionSetOpenXRHapticActionFunc>( \
                dlsym(_akmotion_handle, "AkMotionSetOpenXRHapticAction") \
            ); \
        _akmotion_pSetOpenXRHapticActionFn( \
            Instance, \
            Session, \
            Action); \
    } \

#endif

#ifdef __cplusplus
extern "C" {
#endif
    AK_DLLEXPORT void AkMotionInitializeOpenXRFunctions(
    AK::_akmotionXrResultToString in_pXrResultToString,
    AK::_akmotionXrEnumerateInstanceExtensionProperties in_pXrEnumerateInstanceExtensionProperties,
    AK::_akmotionXrStringToPath in_pXrStringToPath,
    AK::_akmotionXrGetInstanceProcAddr in_pXrGetInstanceProcAddr,
    AK::_akmotionXrApplyHapticFeedback in_pXrApplyHapticFeedback);

    AK_DLLEXPORT void AkMotionSetOpenXRHapticAction(XrInstance in_instance, XrSession in_session, XrAction in_hapticAction);
#ifdef __cplusplus
}
#endif

