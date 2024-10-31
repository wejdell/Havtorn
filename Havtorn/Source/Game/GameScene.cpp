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

		STransformComponent& ghostyTransform = *AddComponent<STransformComponent>(ghosty);
		AddView(ghosty, STransformComponentView::View);
		SSpriteComponent& spriteWSComp = *AddComponent<SSpriteComponent>(ghosty);
		AddView(ghosty, SSpriteComponentView::View);
		AddComponent<SGhostyComponent>(ghosty);
		AddView(ghosty, SGhostyComponentView::View);

		ghostyTransform.Transform.Move({ 0.0f, 2.0f, 0.0f });

		const std::string spritePath = "Assets/Textures/Circle_c.hva";
		//spriteWSComp.UVRect = { 0.0f, 0.0f, 0.125f, 0.125f };
		renderManager->LoadSpriteComponent(spritePath, &spriteWSComp);

		//Define UVRects for Animation Frames on row 0, 1, 2
		//F32 width = 1152.0f;
		//F32 height = 384.0f;
		//F32 frameSize = 96.0f;
		F32 width = 128.0f;
		F32 height = 128.0f;
		F32 frameSize = 128.0f;
		std::vector<SVector4> uvRectsIdle = CreateAnimationClip(width, height, frameSize, 3, 6, 6);
		std::vector<SVector4> uvRectsMoveLeft = CreateAnimationClip(width, height, frameSize, 0, 0, 6);
		std::vector<SVector4> uvRectsMoveRight = CreateAnimationClip(width, height, frameSize, 1, 0, 6);

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

		SSpriteAnimatorGraphComponent& spriteAnimatorGraphComponent = *AddComponent<SSpriteAnimatorGraphComponent>(ghosty);
		AddView(ghosty, SSpriteAnimatorGraphComponentView::View);

		SSpriteAnimatorGraphNode& rootNode = spriteAnimatorGraphComponent.SetRoot(std::string("Idle | Locomotion"), "CGhostySystem::EvaluateIdle");
		rootNode.AddClipNode(&spriteAnimatorGraphComponent, std::string("Idle"), idle);

		SSpriteAnimatorGraphNode& locomotionNode = rootNode.AddSwitchNode(std::string("Locomotion: Left | Right"), "CGhostySystem::EvaluateLocomotion");
		locomotionNode.AddClipNode(&spriteAnimatorGraphComponent, std::string("Move Left"), moveLeft);
		locomotionNode.AddClipNode(&spriteAnimatorGraphComponent, std::string("Move Right"), moveRight);

		CAssetRegistry* assetRegistry = GEngine::GetWorld()->GetAssetRegistry();
		GetComponent<SSpriteComponent>(ghosty)->AssetRegistryKey = assetRegistry->Register(spritePath);

		//SSequencerComponent& sequencerComponent = AddSequencerComponentToEntity(*ghosty);
		//sequencerComponent.ComponentTracks.push_back({ EComponentType::TransformComponent });
		//sequencerComponent.ComponentTracks.push_back({ EComponentType::SpriteComponent });
		//sequencerComponent.ComponentTracks.push_back({ EComponentType::GhostyComponent });
		//sequencerComponent.ComponentTracks.push_back({ EComponentType::CameraComponent });

		SPhysics2DComponent& phys2DComponent = *AddComponent<SPhysics2DComponent>(ghosty);
		AddView(ghosty, SPhysics2DComponentView::View);
		phys2DComponent.BodyType = EPhysics2DBodyType::Dynamic;
		phys2DComponent.ShapeType = EPhysics2DShapeType::Circle;
		phys2DComponent.ShapeLocalExtents = { ghostyTransform.Transform.GetMatrix().GetScale().X, ghostyTransform.Transform.GetMatrix().GetScale().Y };
		//phys2DComponent.ConstrainRotation = true;
		GEngine::GetWorld()->Initialize2DPhysicsData(ghosty);

		const SEntity& floor = AddEntity("Floor");
		STransformComponent& floorTransform = *AddComponent<STransformComponent>(floor);	
		AddView(floor, STransformComponentView::View);
		floorTransform.Transform.Move({ 0.f, -2.f, 0.f });
		floorTransform.Transform.Scale({ 0.4f, 0.5f, 1.f });
		SSpriteComponent& floorSprite = *AddComponent<SSpriteComponent>(floor);
		AddView(floor, SSpriteComponentView::View);

		const std::string floorSpritePath = "Assets/Textures/T_Checkboard_128x128_c.hva";
		floorSprite.UVRect = { 0.0f, 0.0f, 1.f, 1.f };
		renderManager->LoadSpriteComponent(floorSpritePath, &floorSprite);

		SPhysics2DComponent& floorPhys2DComponent = *AddComponent<SPhysics2DComponent>(floor);
		AddView(floor, SPhysics2DComponentView::View);
		floorPhys2DComponent.BodyType = EPhysics2DBodyType::Static;
		floorPhys2DComponent.ShapeType = EPhysics2DShapeType::Capsule;
		floorPhys2DComponent.ShapeLocalExtents = { floorTransform.Transform.GetMatrix().GetScale().X, floorTransform.Transform.GetMatrix().GetScale().Y };
		GEngine::GetWorld()->Initialize2DPhysicsData(floor);

		return true;
    }

    U32 CGameScene::GetSize() const
    {
		U32 size = 0;
		size += CScene::GetSize();

		auto defaultSizeAllocator = [&]<typename T>(const std::vector<T*>&componentVector)
		{
			size += GetDataSize(static_cast<U32>(componentVector.size()));
			for (const auto component : componentVector)
			{
				auto& componentRef = *component;
				size += GetDataSize(componentRef);
			}
		};

		defaultSizeAllocator(GetComponents<SGhostyComponent>());

        return size;
    }

    void CGameScene::Serialize(char* toData, U64& pointerPosition) const
    {
		CScene::Serialize(toData, pointerPosition);

		DefaultSerializer(GetComponents<SGhostyComponent>(), toData, pointerPosition);
    }

    void CGameScene::Deserialize(const char* fromData, U64& pointerPosition, CAssetRegistry* assetRegistry)
    {
		CScene::Deserialize(fromData, pointerPosition, assetRegistry);

		{
			std::vector<SGhostyComponent> components;
			DefaultDeserializer(components, SGhostyComponentView::View, fromData, pointerPosition);
		}
    }
}
