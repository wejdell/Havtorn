// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/System.h"
#include "Input/InputTypes.h"

namespace Havtorn
{
	struct STransformComponent;

	namespace HexAudio
	{
		class CAudioBackend
		{
		public:
			CAudioBackend();
			~CAudioBackend();

			void Update();

			void LoadAsset(const std::string_view assetName);
			void UnloadAsset(const std::string_view assetName);
			void UnloadAll();

			U64 RegisterAudioObject(const bool isListener);
			void UnregisterAudioObject(const U64 id);

			void SetPosition(const U64 id, const STransformComponent* transform, const SVector& localOffset);

			void PlayAudio(const std::string_view name, const U64 emitterID);
			void StopAudio(const std::string_view name, const U64 emitterID);

		private:
			class CAudioImplementation;
			Ptr<CAudioImplementation> Impl;
			U64 MainListenerID = 0;
			std::vector<std::string> LoadedAssets;
		};

		class CAudioSystem : public ISystem
		{
		public:
			CAudioSystem(CAudioBackend* backend);
			~CAudioSystem() override;

			void Update(std::vector<Ptr<CScene>>& scenes) override;
			void DebugAudio(const SInputActionPayload payload);

		private:
			CAudioBackend* Backend = nullptr;
			U64 DebugEmitterID = 0;
		};
	}
}
