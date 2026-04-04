// Copyright 2025 Team Havtorn. All Rights Reserved.
#pragma once
#include "ECS/System.h"
#include "Components/AbilityStateMachineComponent.h"


namespace Havtorn
{

	//struct SAbilityStateMachineComponent;
	//struct SAbility;

	struct SAbility
	{
		SAbility();
		virtual ~SAbility();

		virtual void Init(SAbilityStateMachineComponent* component) = 0;
		virtual void DeInit(SAbilityStateMachineComponent* component) = 0;
		virtual bool ShouldRun(SAbilityStateMachineComponent* component) = 0;
		virtual void Execute() = 0;

		U64 Guid;
		std::vector<EAbilityTag> Tags = {};
	};

	struct Locomotion : SAbility
	{
		Locomotion();
		~Locomotion() override;
		void Init(SAbilityStateMachineComponent* component) override;
		void DeInit(SAbilityStateMachineComponent* component) override;
		bool ShouldRun(SAbilityStateMachineComponent* component) override;
		void Execute() override;
	};

	struct CameraControl : SAbility
	{
		CameraControl() : SAbility() {}
		~CameraControl() override {};
		void Init(SAbilityStateMachineComponent* component) override;
		void DeInit(SAbilityStateMachineComponent* /*component*/) override {};
		bool ShouldRun(SAbilityStateMachineComponent* /*component*/) override { return true; };
		void Execute() override {};
	};

	struct Jump : SAbility
	{
		Jump() : SAbility() {}
		~Jump() override {};
		void Init(SAbilityStateMachineComponent* component) override;
		void DeInit(SAbilityStateMachineComponent* /*component*/) override {};
		bool ShouldRun(SAbilityStateMachineComponent* component) override;
		void Execute() override {};
	};

	struct Falling : SAbility
	{
		Falling() : SAbility() {}
		~Falling() override {};
		void Init(SAbilityStateMachineComponent* component) override;
		void DeInit(SAbilityStateMachineComponent* component) override;
		bool ShouldRun(SAbilityStateMachineComponent* component) override;
		void Execute() override {};
	};



	class CAbilityStateMachineSystem : public ISystem
	{
	public:
		CAbilityStateMachineSystem();
		~CAbilityStateMachineSystem() override = default;
		GAME_API void Update(std::vector<Ptr<CScene>>& scenes) override;

	private:
		bool IsBlocked(const SAbility& ability, const std::vector<EAbilityTag>& blockedTags) const;
		bool IsActive(const SAbility& ability, SAbilityStateMachineComponent* component) const;
		void DeActivateAbility(SAbility& ability, SAbilityStateMachineComponent* component);
		void ActivateAbility(SAbility& ability, SAbilityStateMachineComponent* component);
	};
	
}
