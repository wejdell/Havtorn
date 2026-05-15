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

/// \file

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>

#if defined(AK_WIN)

#include <AK/Tools/Common/AkFNVHash.h>
#include <windef.h>

// Do a bitwise-OR with the deviceID (The deviceID returned by GetAppLocalDeviceIdHash), when adding the output, to utilize the GameInput Advanced Haptics functionality.
// If unset, the device will use GameInput Rumble functionality.
#define AKMOTION_GAMEINPUT_HAPTICS_MODE 0x80000000

namespace AK
{
	// Helper function to use with GameInput devices.
	// DeviceIds for GameInput-based Motion Outputs will be based on the value of GetAppLocalDeviceIdHash(&GameInputDeviceInfo::deviceId).
	// The MSB of this 32-bit value is reserved for haptic/rumble differentiation.
	static AkUInt32 GetAppLocalDeviceIdHash(const APP_LOCAL_DEVICE_ID* in_pAppLocalDeviceId)
	{
		AK::FNVHash32 hash;
		return hash.Compute(&in_pAppLocalDeviceId->value, APP_LOCAL_DEVICE_ID_SIZE) & ~AKMOTION_GAMEINPUT_HAPTICS_MODE;
	}
}

#endif // defined AK_WIN