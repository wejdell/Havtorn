// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"
#include "ECS/Entity.h"

namespace Havtorn
{
	class CWorld;

	class CScriptSystem : public ISystem
	{
	public:
		CScriptSystem(CWorld* world);
		~CScriptSystem() override = default;
		ENGINE_API void Update(std::vector<Ptr<CScene>>& scenes) override;

		void OnBeginPlay(std::vector<Ptr<CScene>>& scenes);
		void OnEndPlay(std::vector<Ptr<CScene>>& scenes);
		void OnBeginOverlap(CScene* scene, const SEntity triggerEntity, const SEntity otherEntity);
		void OnEndOverlap(CScene* scene, const SEntity triggerEntity, const SEntity otherEntity);

	private:
		struct SChangePlayModeData
		{
			SChangePlayModeData() = delete;
			SChangePlayModeData(CScene* scene, const bool beganPlay)
				: Scene(scene)
				, BeganPlay(beganPlay)
			{}
			CScene* Scene = nullptr;
			bool BeganPlay = false;

			auto operator<=>(const SChangePlayModeData& other) const = default;
		};
		std::vector<SChangePlayModeData> PlayModeChanges;

		struct SOverlapData
		{
			SOverlapData() = delete;
			SOverlapData(CScene* scene, const SEntity triggerEntity, const SEntity otherEntity, const bool beganOverlapping)
				: Scene(scene)
				, TriggerEntity(triggerEntity)
				, OtherEntity(otherEntity)
				, BeganOverlap(beganOverlapping)
			{}

			CScene* Scene = nullptr;
			SEntity TriggerEntity = SEntity::Null;
			SEntity OtherEntity = SEntity::Null;
			bool BeganOverlap = false;

			auto operator<=>(const SOverlapData& other) const = default;
		};
		std::vector<SOverlapData> Overlaps;
	};
}
