// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "NodeSystem.h"

#include "Engine.h"
#include "Scene/Scene.h"
#include "Graphics/RenderManager.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/NodeComponent.h"

namespace Havtorn
{
	CNodeSystem::CNodeSystem(CScene* /*scene*/, CRenderManager* /*renderManager*/)
		: ISystem()
	{

	}

	CNodeSystem::~CNodeSystem()
	{
	}

	void CNodeSystem::Update(CScene* scene)
	{
		auto& nodeComponents = scene->GetNodeComponents();
		for (auto& node : nodeComponents)
		{
			node->Time += GTimer::Dt();
			
			const auto& transformIndex = node->Entity->GetComponentIndex(EComponentType::TransformComponent);
			auto& transform = scene->GetTransformComponents()[transformIndex];
			auto& matrix = transform->Transform.GetMatrix();
			auto currentPosition = matrix.GetTranslation();
			currentPosition.X = UMath::Cos(node->Time);
			currentPosition.Z = UMath::Sin(node->Time);
			//currentPosition.Z = UMath::Sin(node->Time);
			matrix.SetTranslation(currentPosition);
		}
	}
}
