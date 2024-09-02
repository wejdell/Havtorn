// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "GameScene.h"
#include "Ghosty/GhostySystem.h"
#include "Ghosty/GhostyComponent.h"
#include "Ghosty/GhostyComponentView.h"

#include <Graphics/RenderManager.h>
#include <Scene/World.h>
#include <Scene/AssetRegistry.h>

namespace Havtorn
{
    bool CGameScene::Init3DDemoScene(CRenderManager* renderManager)
    {
        if (!CScene::Init3DDemoScene(renderManager))
            return false;

        return true;
    }

	std::vector<SVector4> CreateAnimationClip(const F32 width, const F32 height, const F32 frameSize, const U32 row, const U32 column, const U32 frameCount)
	{
		std::vector<SVector4> uvRects;
		F32 normalizedFrameSize = frameSize / width;

		for (U32 i = 0; i < frameCount; i++)
		{
			F32 x = (column + i) * normalizedFrameSize;
			F32 y = row * (frameSize / height);
			F32 z = x + normalizedFrameSize;
			F32 w = y + (frameSize / height);

			uvRects.push_back(SVector4{ x, y, z, w });
		}

		return uvRects;
	}

    bool CGameScene::Init2DDemoScene(CRenderManager* renderManager)
    {
		if (!CScene::Init2DDemoScene(renderManager))
			return false;

		const SEntity& ghosty = AddEntity("Ghosty");
		if (!ghosty.IsValid())
			return true;

		STransformComponent& spriteWStransform = *AddComponent<STransformComponent>(ghosty);
		AddView(ghosty, STransformComponentView::View);
		SSpriteComponent& spriteWSComp = *AddComponent<SSpriteComponent>(ghosty);
		AddView(ghosty, SSpriteComponentView::View);
		AddComponent<SGhostyComponent>(ghosty);
		AddView(ghosty, SGhostyComponentView::View);

		spriteWStransform.Transform.Move({ 0.0f, 0.0f, 0.0f });
		//F32 radians = UMath::DegToRad(45.0f);
		//spriteWStransform.Transform.Rotate({ radians, radians, radians });

		const std::string spritePath = "Assets/Textures/EllahSpriteSheet.hva";
		spriteWSComp.UVRect = { 0.0f, 0.0f, 0.125f, 0.125f };
		renderManager->LoadSpriteComponent(spritePath, &spriteWSComp);

		//Define UVRects for Animation Frames on row 0, 1, 2
		//F32 size = 32.0f / 256.0f;
		F32 width = 1152.0f;
		F32 height = 384.0f;
		F32 frameSize = 96.0f;
		std::vector<SVector4> uvRectsIdle = CreateAnimationClip(width, height, frameSize, 3, 6, 6);
		std::vector<SVector4> uvRectsMoveLeft = CreateAnimationClip(width, height, frameSize, 0, 0, 6);
		std::vector<SVector4> uvRectsMoveRight = CreateAnimationClip(width, height, frameSize, 1, 0, 6);
		//std::vector<SVector4> uvRectsIdle = {
		//	SVector4{ 0.0f,		0.0f,		size,			size },
		//	SVector4{ size,		0.0f,		size * 2,		size },
		//};
		//std::vector<SVector4> uvRectsMoveLeft = {
		//	SVector4{ 0.0f,		size,		size,		size * 2 },
		//	SVector4{ size,		size,		size * 2,	size * 2 },
		//	SVector4{ size * 2, size,		size * 3,	size * 2 },
		//	//SVector4{ size * 3, size,		size * 4,	size * 2 },
		//};
		//std::vector<SVector4> uvRectsMoveRight = {
		//	SVector4{ 0.0f,		size * 2,	size,		size * 3 },
		//	SVector4{ size,		size * 2,	size * 2,	size * 3 },
		//	SVector4{ size * 2, size * 2,	size * 3,	size * 3 },
		//	
		//	//SVector4{ size * 3, size * 2,	size * 4,	size * 3 },
		//};

		SSpriteAnimationClip idle;
		idle.UVRects = uvRectsIdle;
		idle.Durations.push_back(0.15f);
		idle.Durations.push_back(0.15f);

		SSpriteAnimationClip moveLeft;
		moveLeft.UVRects = uvRectsMoveLeft;
		moveLeft.Durations.push_back(0.15f);
		moveLeft.Durations.push_back(0.15f);
		moveLeft.Durations.push_back(0.15f);


		SSpriteAnimationClip moveRight
		{
			uvRectsMoveRight, //UVRects
			{ 0.15f, 0.15f, 0.15f }, //Duration per Frame
			true	//IsLooping
		};

		//moveRight.UVRects = uvRectsMoveRight;
		//moveRight.Durations.push_back(0.15f);
		//moveRight.Durations.push_back(0.15f);
		//moveRight.Durations.push_back(0.15f);

		CGhostySystem* ghostySystem = GEngine::GetWorld()->GetSystem<CGhostySystem>();
		SSpriteAnimatorGraphComponent& spriteAnimatorGraphComponent = *AddComponent<SSpriteAnimatorGraphComponent>(ghosty);
		AddView(ghosty, SSpriteAnimatorGraphComponentView::View);

		SSpriteAnimatorGraphNode& rootNode = spriteAnimatorGraphComponent.SetRoot(std::string("Idle | Locomotion"), ghostySystem->EvaluateIdleFunc);
		rootNode.AddClipNode(&spriteAnimatorGraphComponent, std::string("Idle"), idle);

		SSpriteAnimatorGraphNode& locomotionNode = rootNode.AddSwitchNode(std::string("Locomotion: Left | Right"), ghostySystem->EvaluateLocomotionFunc);
		locomotionNode.AddClipNode(&spriteAnimatorGraphComponent, std::string("Move Left"), moveLeft);
		locomotionNode.AddClipNode(&spriteAnimatorGraphComponent, std::string("Move Right"), moveRight);

		CAssetRegistry* assetRegistry = GEngine::GetWorld()->GetAssetRegistry();
		GetComponent<SSpriteComponent>(ghosty)->AssetRegistryKey = assetRegistry->Register(spritePath);

		//SSequencerComponent& sequencerComponent = AddSequencerComponentToEntity(*ghosty);
		//sequencerComponent.ComponentTracks.push_back({ EComponentType::TransformComponent });
		//sequencerComponent.ComponentTracks.push_back({ EComponentType::SpriteComponent });
		//sequencerComponent.ComponentTracks.push_back({ EComponentType::GhostyComponent });
		//sequencerComponent.ComponentTracks.push_back({ EComponentType::CameraComponent });

		return true;
    }

    U32 CGameScene::GetSize() const
    {
		U32 size = 0;
		size += CScene::GetSize();

		size += sizeof(U64);
		size += sizeof(SGhostyComponent) * static_cast<U32>(GetComponents<SGhostyComponent>().size());

        return U32();
    }

    void CGameScene::Serialize(char* toData, U64& pointerPosition) const
    {
		CScene::Serialize(toData, pointerPosition);

		const auto& ghostyComponents = GetComponents<SGhostyComponent>();
		SerializeData(ghostyComponents.size(), toData, pointerPosition);
		for (auto component : ghostyComponents)
			SerializeData(*component, toData, pointerPosition);
    }

    void CGameScene::Deserialize(const char* fromData, U64& pointerPosition, CAssetRegistry* assetRegistry)
    {
		CScene::Deserialize(fromData, pointerPosition, assetRegistry);

		U64 numberOfGhostyComponents = 0;
		DeserializeData(numberOfGhostyComponents, fromData, pointerPosition);
		std::vector<SGhostyComponent> ghostyComponents;
		DeserializeData(ghostyComponents, fromData, static_cast<U32>(numberOfGhostyComponents), pointerPosition);
		for (const auto& component : ghostyComponents)
			AddComponent<SGhostyComponent>(component.Owner);
    }
}
