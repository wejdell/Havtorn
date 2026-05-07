// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "HexAudio.h"

#ifdef HV_AUDIO_BACKEND_WWISE
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkMemoryMgrModule.h>
#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>

// TODO.NW: The IO Hook is currently windows only, see if we can remedy that, 
// if not make it platform independent then at least include the right thing based on platform
#include <AK/SoundEngine/Common/AkFilePackageLowLevelIODeferred.h>

#include <AK/SoundEngine/Common/AkSoundEngine.h>
// TODO.NW: Might want to make a toggle of this
#include <AK/SpatialAudio/Common/AkSpatialAudio.h>

#ifndef AK_OPTIMIZED
#include <AK/Comm/AkCommunication.h>
#endif
#endif

#include "Scene/Scene.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/AudioEmitterComponent.h"
#include "ECS/Components/AudioListenerComponent.h"
#include "ECS/GUIDManager.h"
#include "Engine.h"
#include "Input/InputMapper.h"

#include <Filesystem.h>
#include <PlatformUtilities.h>

namespace Havtorn
{
	namespace HexAudio
	{
		constexpr U64 DefaultListenerID = 1;

#ifdef HV_AUDIO_BACKEND_WWISE
		class CAudioBackend::CAudioImplementation
		{
		public:
			CAudioImplementation() = default;

			static void ErrorCallback(AK::Monitor::ErrorCode errorCode, const AkOSChar* wideErrorMessage, AK::Monitor::ErrorLevel errorLevel, AkPlayingID playingID, AkGameObjectID audioObjectID)
			{
				std::string errorMessage;
				ToString(wideErrorMessage, errorMessage);

				const char* errorCodeString = magic_enum::enum_name<AK::Monitor::ErrorCode>(errorCode).data();

				switch (errorLevel)
				{
				case AK::Monitor::ErrorLevel_Message:
					HV_LOG_TRACE("Wwise Backend | (AudioObjectID: %u, PlayingID: %u) : %s: %s", audioObjectID, playingID, errorCodeString, errorMessage.c_str());
					break;
				case AK::Monitor::ErrorLevel_Error: [[fallthrough]];
				default: 
					HV_LOG_ERROR("Wwise Backend | (AudioObjectID: %u, PlayingID: %u) : %s: %s", audioObjectID, playingID, errorCodeString, errorMessage.c_str());
					break;
				}
			}

			bool Init()
			{
				AkMemSettings memSettings;
				AK::MemoryMgr::GetDefaultSettings(memSettings);

				if (AK::MemoryMgr::Init(&memSettings) != AK_Success)
					return false;

				AkStreamMgrSettings stmSettings;
				AK::StreamMgr::GetDefaultSettings(stmSettings);

				if (!AK::StreamMgr::Create(stmSettings))
					return false;

				AkDeviceSettings deviceSettings;
				AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);
				if (LowLevelIO.Init(deviceSettings) != AK_Success)
					return false;

				AkInitSettings initSettings;
				AkPlatformInitSettings platformInitSettings;
				AK::SoundEngine::GetDefaultInitSettings(initSettings);
				AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);

				if (AK::SoundEngine::Init(&initSettings, &platformInitSettings) != AK_Success)
					return false;

				AkSpatialAudioInitSettings spatialSettings;
				if (AK::SpatialAudio::Init(spatialSettings) != AK_Success)
					return false;

#ifndef AK_OPTIMIZED
				AkCommSettings commSettings;
				AK::Comm::GetDefaultInitSettings(commSettings);
				if (AK::Comm::Init(commSettings) != AK_Success)
					return false;
#endif

				if (LowLevelIO.SetBasePath(AKTEXT("GeneratedSoundBanks/Windows/")) != AK_Success)
					return false;

				AK::StreamMgr::SetCurrentLanguage(AKTEXT("English(US)"));

				AK::Monitor::SetLocalOutput(AK::Monitor::ErrorLevel_All, ErrorCallback);

				AkBankID bankID;
				if (AK::SoundEngine::LoadBank("Init.bnk", bankID) != AK_Success)
					HV_LOG_WARN("AudioBackend::LoadAsset: Could not load Init soundbank");

				return true;
			}

			void Terminate()
			{
#ifndef AK_OPTIMIZED
				AK::Comm::Term();
#endif
				
				AK::SoundEngine::Term();
				
				LowLevelIO.Term();
				if (AK::IAkStreamMgr* streamManager = AK::IAkStreamMgr::Get())
					streamManager->Destroy();

				AK::MemoryMgr::Term();
			}

			void Update()
			{
				AK::SoundEngine::RenderAudio();
			}

			bool LoadAsset(const std::string_view assetName)
			{
				std::string bankPath = "Event/"; 
				bankPath.append(assetName.data()); 
				bankPath.append(".bnk");

				AkBankID bankID;	
				if (AK::SoundEngine::LoadBank(bankPath.c_str(), bankID) != AK_Success)
					return false;

				const char* names[] = { assetName.data() };
				AK::SoundEngine::PrepareEvent(AK::SoundEngine::Preparation_Load, names, 1);
				return true;

			}

			void UnloadAsset(const std::string_view assetName)
			{
				const char* names[] = { assetName.data() };
				AK::SoundEngine::PrepareEvent(AK::SoundEngine::Preparation_Unload, names, 1);

				std::string bankPath = "Event/";
				bankPath.append(assetName.data());
				bankPath.append(".bnk");
				AK::SoundEngine::UnloadBank(bankPath.c_str(), nullptr);
			}

			void RegisterAudioObject(const U64 id, const bool isListener)
			{
				if (isListener)
				{
					// TODO.NW: Would be nice to expose the names so we can view them when profiling
					AK::SoundEngine::RegisterGameObj(id, "Listener Object");
					
					if (id == DefaultListenerID)
						AK::SoundEngine::SetDefaultListeners(&id, 1);
				}
				else
				{
					AK::SoundEngine::RegisterGameObj(id, "Emitter Object");
				}
			}

			void UnregisterAudioObject(const U64 id)
			{
				AK::SoundEngine::UnregisterGameObj(id);
			}

			void SetPosition(const U64 id, const STransformComponent* transform, const SVector& localOffset)
			{
				const SMatrix transformMatrix = transform->Transform.GetMatrix();
				const SVector position = transformMatrix.GetTranslation() + (SVector4(localOffset, 1.0f) * transformMatrix).ToVector3();
				const SVector upVector = transformMatrix.GetUp().GetNormalized();
				const SVector forwardVector = transformMatrix.GetForward().GetNormalized();

				AkSoundPosition audioTransform;
				audioTransform.SetOrientation({ forwardVector.X, forwardVector.Y, forwardVector.Z }, { upVector.X, upVector.Y, upVector.Z });
				audioTransform.SetPosition({ position.X, position.Y, position.Z });

				AK::SoundEngine::SetPosition(id, audioTransform);
			}

			void PlayAudio(const std::string_view name, const U64 audioObjectID)
			{
				if (AK::SoundEngine::PostEvent(name.data(), audioObjectID) == AK_INVALID_PLAYING_ID)
					HV_LOG_WARN("AudioBackend::PlayAudio: Failed to play audio with name: %s", name.data());
			}

			void StopAudio(const std::string_view name, const U64 audioObjectID)
			{
				if (AK::SoundEngine::ExecuteActionOnEvent(name.data(), AkActionOnEventType_Stop, audioObjectID) != AK_Success)
					HV_LOG_WARN("AudioBackend::StopAudio: Failed to stop audio with name: %s", name.data());
			}

		private:
			CAkFilePackageLowLevelIODeferred LowLevelIO;
		};
#endif

#ifdef HV_AUDIO_BACKEND_SDL
	class CAudioBackend::CAudioImplementation
	{
	public:
		CAudioImplementation() = default;

		bool Init()
		{
			return true;
		}

		void Terminate()
		{
		}

		void Update()
		{
		}

		bool LoadAsset(const std::string_view /*assetName*/)
		{
			return true;
		}

		void UnloadAsset(const std::string_view /*assetName*/)
		{
		}

		U64 RegisterAudioObject(const U64 /*id*/, const bool /*isListener*/)
		{
			return U64();
		}

		void UnregisterAudioObject(const U64 /*id*/)
		{
		}

		void SetPosition(const U64 /*id*/, const STransformComponent* /*transform*/, const SVector& /*localOffset*/)
		{
		}

		void PlayAudio(const std::string_view /*name*/, const U64 /*audioObjectID*/)
		{
		}

		void StopAudio(const std::string_view /*name*/, const U64 /*audioObjectID*/)
		{
		}

	private:
	};
#endif

		CAudioBackend::CAudioBackend()
			: Impl(std::make_unique<CAudioBackend::CAudioImplementation>())
		{
			if (!Impl->Init())
				HV_LOG_ERROR("AudioBackend::AudioBackend: Could not initialize audio backend!");

			// TODO.NW: Listen for unfocusing the application and call 
			// AK::SoundEngine::Suspend(false) or AK::SoundEngine::Suspend(true). 
			// If using Suspend(false), do not continue posting events while 
			// suspended as those would queue up and play all at the same time when 
			// regaining focus and calling AK::SoundEngine::WakeupFromSuspend().
		}

		CAudioBackend::~CAudioBackend()
		{
			Impl->Terminate();
		}

		void CAudioBackend::Update()
		{
			Impl->Update();
		}

		void CAudioBackend::LoadAsset(const std::string_view assetName)
		{
			if (Impl->LoadAsset(assetName))
				LoadedAssets.push_back(assetName.data());
		}

		void CAudioBackend::UnloadAsset(const std::string_view assetName)
		{
			Impl->UnloadAsset(assetName);
		}

		void CAudioBackend::UnloadAll()
		{
			for (const std::string& asset : LoadedAssets)
				Impl->UnloadAsset(asset);
		}

		U64 CAudioBackend::RegisterAudioObject(const bool isListener)
		{
			U64 newID = UGUIDManager::Generate();

			if (isListener && MainListenerID == 0)
			{
				newID = DefaultListenerID;
				MainListenerID = newID;
			}


			Impl->RegisterAudioObject(newID, isListener);
			return newID;
		}

		void CAudioBackend::UnregisterAudioObject(const U64 id)
		{
			Impl->UnregisterAudioObject(id);
		}

		void CAudioBackend::SetPosition(const U64 id, const STransformComponent* transform, const SVector& localOffset)
		{
			// TODO.NW: Don't do this if object is static, set only once on init. Can maybe let RegisterAudioObject take flags instead or something
			Impl->SetPosition(id, transform, localOffset);
		}

		void CAudioBackend::PlayAudio(const std::string_view name, const U64 emitterID)
		{
			Impl->PlayAudio(name, emitterID);
		}

		void CAudioBackend::StopAudio(const std::string_view name, const U64 emitterID)
		{
			Impl->StopAudio(name, emitterID);
		}

		CAudioSystem::CAudioSystem(CAudioBackend* backend)
			: Backend(backend)
		{
			GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderPassForward).AddMember(this, &CAudioSystem::DebugAudio);
			Backend->LoadAsset("PlaySound");
		}

		CAudioSystem::~CAudioSystem()
		{
			GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderPassForward).RemoveObject(this);
			Backend->UnloadAll();
		}

		void CAudioSystem::Update(std::vector<Ptr<CScene>>& scenes)
		{
			for (const Ptr<CScene>& scene : scenes)
			{
				for (SAudioEmitterComponent* emitter : scene->GetComponents<SAudioEmitterComponent>())
				{
					STransformComponent* transform = scene->GetComponent<STransformComponent>(emitter->Owner);
					Backend->SetPosition(emitter->AudioObjectID, transform, emitter->LocalOffset);

					DebugEmitterID = emitter->AudioObjectID;
				}

				for (SAudioListenerComponent* listener : scene->GetComponents<SAudioListenerComponent>())
				{
					STransformComponent* transform = scene->GetComponent<STransformComponent>(listener->Owner);
					Backend->SetPosition(listener->AudioObjectID, transform, listener->LocalOffset);
				}
			}

			Backend->Update();
		}

		void CAudioSystem::DebugAudio(const SInputActionPayload payload)
		{
			if (!payload.IsPressed)
				return;

			if (DebugEmitterID != 0)
				Backend->PlayAudio("PlaySound", DebugEmitterID);
		}
	}
}
