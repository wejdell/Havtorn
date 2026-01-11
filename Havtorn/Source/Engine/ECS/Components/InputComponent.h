// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct ENGINE_API SInputComponent : SComponent
	{
		SInputComponent() = default;
		SInputComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{};

		~SInputComponent() override = default;

		//Enum Filter EInputActions (Vilka vi vill lyssna efter)
		//IsActive 
		//JAA Variant exakt, tänkte typ att man nästan ville återanvända Databinding klassen xD (så man får samma typer, fast kanske lite overkill)
		//Man behöver ju bara typ float, bool, char(?), Vector2

		SVector MoveInput = { 0.0f, 0.0f, 0.0f };
		bool IsActive = true; //<-- dethär är det enda som gör denhär till något annat än bara "cachea input i SystemX"

	};
}
