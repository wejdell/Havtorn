// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/System.h"
#include "Input/InputTypes.h"
#include "Assets/AssetReference.h"

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

			void LoadAsset(const SAssetReference& assetRef);
			void UnloadAsset(const SAssetReference& assetRef);
			void UnloadAll();

			U64 RegisterAudioObject(const bool isListener, const std::vector<SAssetReference>& assetReferences);
			void UnregisterAudioObject(const U64 id, const std::vector<SAssetReference>& assetReferences);

			void SetPosition(const U64 id, const STransformComponent* transform, const SVector& localOffset);

			void PlayAudio(const SAssetReference& assetRef, const U64 emitterID);
			void StopAudio(const SAssetReference& assetRef, const U64 emitterID);

		private:
			class CAudioImplementation;
			Ptr<CAudioImplementation> Impl;
			U64 MainListenerID = 0;
			const U64 AudioBackendRequesterID = 4510;
			std::vector<SAssetReference> LoadedAssets;
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
			bool QueueDebugAudio = false;
		};
	}
}
