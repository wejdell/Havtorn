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

#ifdef HV_AUDIO_BACKEND_FMOD
#include <fmod_common.h>
#include <fmod_studio.h>
#include <fmod_studio.hpp>
#include <fmod_studio_common.h>
#include <fmod_errors.h>
#endif

#ifdef HV_AUDIO_BACKEND_SDL
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_filesystem.h>
// TODO.NW: Add SDL3 Mixer if we end up using SDL audio, for spatialization
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

#ifdef HV_AUDIO_BACKEND_FMOD

		using namespace FMOD;

		static std::string FmodError = "";
		bool FmodCheck(const FMOD_RESULT result)
		{ 
			if (result != FMOD_OK) 
				FmodError = FMOD_ErrorString(result); 
			
			return result == FMOD_OK; 
		}

		class CAudioBackend::CAudioImplementation
		{
		public:
			CAudioImplementation() = default;

			bool Init()
			{
				if (!FmodCheck(Studio::System::create(&StudioSystem)))
				{
					HV_LOG_ERROR("AudioBackend::Init: %s", FmodError.c_str());
					return false;
				}

				FMOD_STUDIO_INITFLAGS initFlags = FMOD_STUDIO_INIT_NORMAL;
#ifndef HV_RELEASE
				initFlags |= FMOD_STUDIO_INIT_LIVEUPDATE;
#endif // HV_RELEASE

				if (!FmodCheck(StudioSystem->initialize(MaxChannels, initFlags, FMOD_INIT_NORMAL | FMOD_INIT_PROFILE_ENABLE, 0)))
				{
					HV_LOG_ERROR("AudioBackend::Init: %s", FmodError.c_str());
					return false;
				}

				if (!FmodCheck(StudioSystem->getCoreSystem(&CoreSystem)))
				{
					HV_LOG_ERROR("AudioBackend::Init: %s", FmodError.c_str());
					return false;
				}
				
				return true;
			}

			void Terminate()
			{
				if (!FmodCheck(StudioSystem->release()))
					HV_LOG_ERROR("AudioBackend::Terminate: %s", FmodError.c_str());
			}

			void Update()
			{
				StudioSystem->update();
			}

			bool LoadAsset(const std::string_view assetName)
			{
				if (LoadedSounds.contains(assetName.data()))
					return false;

				std::string assetPath = UFileSystem::GetWorkingPath();
				assetPath.append("AudioSource/");
				assetPath.append(assetName.data());
				assetPath.append(".wav");

				// TODO.NW: Need some sort of settings structure, for picking 3D, looping etc.

				Sound* newSound = nullptr;
				if (!FmodCheck(CoreSystem->createSound(assetPath.c_str(), FMOD_3D, nullptr, &newSound)))
				{
					HV_LOG_WARN("AudioBackend::LoadAsset: %s", FmodError.c_str());
					return false;
				}

				// Sound memory is handled by Fmod, just need to make sure we release it when unloading
				LoadedSounds.emplace(assetName.data(), newSound);
				return true;
			}

			void UnloadAsset(const std::string_view assetName)
			{
				if (!LoadedSounds.contains(assetName.data()))
					return;

				LoadedSounds.at(assetName.data())->release();
				LoadedSounds.erase(assetName.data());
			}

			void RegisterAudioObject(const U64 id, const bool isListener)
			{
				if (isListener)
					return;

				Emitters.emplace(id, nullptr);
			}

			void UnregisterAudioObject(const U64 id)
			{
				// TODO.NW: Figure out how to deal with multiple listeners, if we want that.
				if (id == DefaultListenerID)
					return;

				if (!Emitters.contains(id))
					return;

				bool isPlaying = false; 
				if (!FmodCheck(Emitters.at(id)->isPlaying(&isPlaying)) || !isPlaying)
				{
					Emitters.erase(id);
				}
			}

			void SetPosition(const U64 id, const STransformComponent* transform, const SVector& localOffset)
			{
				const SMatrix transformMatrix = transform->Transform.GetMatrix();
				const SVector position = transformMatrix.GetTranslation() + (SVector4(localOffset, 1.0f) * transformMatrix).ToVector3();
				const SVector upVector = transformMatrix.GetUp().GetNormalized();
				const SVector forwardVector = transformMatrix.GetForward().GetNormalized();

				// TODO.NW: Figure out how to deal with multiple listeners, if we want that.
				if (id == DefaultListenerID)
				{
					FMOD_3D_ATTRIBUTES audioTransform;
					audioTransform.position = { position.X, position.Y, position.Z };
					audioTransform.velocity = { 0.0f, 0.0f, 0.0f }; // TODO.NW: Add
					audioTransform.forward = { forwardVector.X, forwardVector.Y, forwardVector.Z }; 
					audioTransform.up = { upVector.X, upVector.Y, upVector.Z };

					// NW: Use this to base the attenuation on something else
					FMOD_VECTOR attenuationPosition = audioTransform.position;

					if (!FmodCheck(StudioSystem->setListenerAttributes(0, &audioTransform, &attenuationPosition)))
						HV_LOG_WARN("AudioBackend::SetPosition: %s", FmodError.c_str());

					return;
				}

				if (!Emitters.contains(id))
					return;

				FMOD_VECTOR emitterPosition = { position.X, position.Y, position.Z };
				FMOD_VECTOR emitterVelocity = { 0.0f, 0.0f, 0.0f }; // TODO.NW: Add
				Emitters.at(id)->set3DAttributes(&emitterPosition, &emitterVelocity);
			}

			void PlayAudio(const std::string_view name, const U64 audioObjectID)
			{
				if (!LoadedSounds.contains(name.data()))
					return;

				if (!Emitters.contains(audioObjectID))
					return;

				CoreSystem->playSound(LoadedSounds.at(name.data()), nullptr, false, &Emitters.at(audioObjectID));
			}

			void StopAudio(const std::string_view /*name*/, const U64 /*audioObjectID*/)
			{
			}

		private:
			Studio::System* StudioSystem = nullptr;
			System* CoreSystem = nullptr;
			std::map<std::string, Sound*> LoadedSounds;
			std::map<U64, Channel*> Emitters;
			U32 MaxChannels = 256;
		};
#endif

#ifdef HV_AUDIO_BACKEND_SDL
	struct SSoundData
	{
		~SSoundData()
		{
			if (WavData != nullptr)
				SDL_free(WavData);
		}

		U8* WavData = nullptr;
		SDL_AudioSpec Spec = {};
		U32 DataSize = 0;
	};

	struct SChannel
	{
		~SChannel()
		{
			SDL_DestroyAudioStream(Stream);
		}

		SDL_AudioStream* Stream = nullptr;
		bool IsUsed = false;
	};

	class CAudioBackend::CAudioImplementation
	{
	public:
		CAudioImplementation() = default;

		bool Init()
		{
			DeviceID = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
			if (DeviceID == 0)
			{
				HV_LOG_WARN("AudioBackend::Init: Failed to open audio device: %s", SDL_GetError());
				return false;
			}
			
			return true;
		}

		void Terminate()
		{
			SDL_CloseAudioDevice(DeviceID);
			Channels.clear();
		}

		void Update()
		{
			for (Ptr<SChannel>& channel : Channels)
			{
				if (SDL_GetAudioStreamAvailable(channel->Stream) == 0)
				{
					SDL_FlushAudioStream(channel->Stream);
					channel->IsUsed = false;					
				}
			}

			std::erase_if(Channels, [](const Ptr<SChannel>& channel) { return !channel->IsUsed; });
		}

		bool LoadAsset(const std::string_view assetName)
		{
			if (LoadedAssets.contains(assetName.data()))
				return false;
			
			std::string assetPath = UFileSystem::GetWorkingPath();
			assetPath.append("AudioSource/");
			assetPath.append(assetName.data());
			assetPath.append(".wav");

			Ptr<SSoundData> data = std::make_unique<SSoundData>();
			if (!SDL_LoadWAV(assetPath.c_str(), &data->Spec, &data->WavData, &data->DataSize))
			{
				HV_LOG_WARN("AudioBackend::LoadAsset: Could not load WAV: %s", SDL_GetError());
				return false;
			}

			LoadedAssets.emplace(assetName.data(), std::move(data));
			return true;
		}

		void UnloadAsset(const std::string_view assetName)
		{
			if (!LoadedAssets.contains(assetName.data()))
				return;

			LoadedAssets.erase(assetName.data());
		}

		void RegisterAudioObject(const U64 /*id*/, const bool /*isListener*/)
		{
		}

		void UnregisterAudioObject(const U64 /*id*/)
		{
		}

		void SetPosition(const U64 /*id*/, const STransformComponent* /*transform*/, const SVector& /*localOffset*/)
		{
		}

		void PlayAudio(const std::string_view name, const U64 /*audioObjectID*/)
		{
			if (!LoadedAssets.contains(name.data()))
			{
				HV_LOG_WARN("AudioBackend::PlayAudio: Failed to play '%s', it was not loaded!", name.data());
				return;
			}

			SSoundData* data = LoadedAssets.at(name.data()).get();

			SDL_AudioStream* freeStream = nullptr;
			for (Ptr<SChannel>& channel : Channels)
			{
				if (channel->IsUsed)
					continue;

				freeStream = channel->Stream;
				channel->IsUsed = true;
			}

			if (freeStream == nullptr)
			{
				if (!InitNewChannel(data->Spec))
				{
					HV_LOG_WARN("AudioBackend::PlayAudio: Failed to play '%s', could not create a new channel based on its spec!", name.data());
					return;
				}

				freeStream = Channels.back()->Stream;
			}

			if (!SDL_PutAudioStreamData(freeStream, data->WavData, data->DataSize))
			{
				HV_LOG_WARN("AudioBackend::PlayAudio: Failed to play '%s': %s", SDL_GetError());
				return;
			}

			Channels.back()->IsUsed = true;
		}

		void StopAudio(const std::string_view /*name*/, const U64 /*audioObjectID*/)
		{
		}

	private:
		bool InitNewChannel(SDL_AudioSpec& spec)
		{
			Channels.emplace_back(std::make_unique<SChannel>(SDL_CreateAudioStream(&spec, nullptr), false));
			if (Channels.back()->Stream == nullptr)
			{
				HV_LOG_WARN("AudioBackend::InitNewChannel: Could not create audio stream: %s", SDL_GetError());
				Channels.pop_back();
				return false;
			}

			if (!SDL_BindAudioStream(DeviceID, Channels.back()->Stream))
			{
				HV_LOG_WARN("AudioBackend::InitNewChannel: Failed to bind audio stream to device!");
				Channels.pop_back();
				return false;
			}

			return true;
		}

		SDL_AudioDeviceID DeviceID = 0;
		std::map<std::string, Ptr<SSoundData>> LoadedAssets;
		std::vector<Ptr<SChannel>> Channels;
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
