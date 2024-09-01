// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Scene.h"

#include "ECS/ECSInclude.h"
#include "ECS/GUIDManager.h"
#include "Graphics/RenderManager.h"
#include "FileSystem/FileHeaderDeclarations.h"
#include "World.h"
#include "AssetRegistry.h"

#include <algorithm>

namespace Havtorn
{
	CScene::CScene()
	{}

	CScene::~CScene()
	{
		Entities.clear();
		RenderManager = nullptr;
	}

	bool CScene::Init(CRenderManager* renderManager, const std::string& sceneName)
	{
		RenderManager = renderManager;
		SceneName = sceneName;
		return true;
	}

	bool CScene::Init3DDemoScene(CRenderManager* renderManager)
	{
		SceneName = std::string("3DDemoScene");

		// === Camera ===
		MainCameraEntity = AddEntity("Camera");
		
		if (!MainCameraEntity.IsValid())
			return false;

		CAssetRegistry* assetRegistry = GEngine::GetWorld()->GetAssetRegistry();

		// Setup entities (create components)
		STransformComponent& transform = *AddComponent<STransformComponent>(MainCameraEntity);
		//AddView<STransformComponentView>(MainCameraEntity);
		AddView(MainCameraEntity, STransformComponentView::View);

		transform.Transform.Translate({ 2.0f, 1.0f, -3.0f });
		transform.Transform.Rotate({ 0.0f, UMath::DegToRad(35.0f), 0.0f });
		transform.Transform.Translate(SVector::Right * 0.25f);

		SCameraComponent& camera = *AddComponent<SCameraComponent>(MainCameraEntity);
		//AddView<SCameraComponentView>(MainCameraEntity);
		AddView(MainCameraEntity, SCameraComponentView::View);
		camera.ProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), (16.0f / 9.0f), 0.1f, 1000.0f);
		camera.ViewMatrix = SMatrix::LookAtLH(SVector::Zero, SVector::Forward, SVector::Up);

		SCameraControllerComponent& controllerComp = *AddComponent<SCameraControllerComponent>(MainCameraEntity);
		//AddView<SCameraControllerComponentView>(MainCameraEntity);
		AddView(MainCameraEntity, SCameraControllerComponentView::View);
		controllerComp.CurrentYaw = UMath::DegToRad(-35.0f);
		// === !Camera ===

		// === Environment light ===
		const SEntity& environmentLightEntity = AddEntity("Environment Light");
		if (!environmentLightEntity.IsValid())
			return false;

		AddComponent<STransformComponent>(environmentLightEntity);
		//AddView<STransformComponentView>(environmentLightEntity);
		AddView(environmentLightEntity, STransformComponentView::View);
		renderManager->LoadEnvironmentLightComponent("Assets/Textures/Cubemaps/CubemapTheVisit.hva", AddComponent<SEnvironmentLightComponent>(environmentLightEntity));
		AddView(environmentLightEntity, SEnvironmentLightComponentView::View);
		GetComponent<SEnvironmentLightComponent>(environmentLightEntity)->AssetRegistryKey = assetRegistry->Register("Assets/Textures/Cubemaps/CubemapTheVisit.hva");
		// === !Environment light ===

		// === Directional light ===
		const SEntity& directionalLightEntity = AddEntity("Directional Light");
		if (!directionalLightEntity.IsValid())
			return false;

		// NR: Add transform to directional light so it can filter environmental lights based on distance
		AddComponent<STransformComponent>(directionalLightEntity);
		AddView(directionalLightEntity, STransformComponentView::View);

		SDirectionalLightComponent& directionalLight = *AddComponent<SDirectionalLightComponent>(directionalLightEntity);
		AddView(directionalLightEntity, SDirectionalLightComponentView::View);
		directionalLight.Direction = { 1.0f, 1.0f, -1.0f, 0.0f };
		directionalLight.Color = { 212.0f / 255.0f, 175.0f / 255.0f, 55.0f / 255.0f, 0.25f };
		directionalLight.ShadowmapView.ShadowmapViewportIndex = 0;
		directionalLight.ShadowmapView.ShadowProjectionMatrix = SMatrix::OrthographicLH(directionalLight.ShadowViewSize.X, directionalLight.ShadowViewSize.Y, directionalLight.ShadowNearAndFarPlane.X, directionalLight.ShadowNearAndFarPlane.Y);

		SVolumetricLightComponent& volumetricLight = *AddComponent<SVolumetricLightComponent>(directionalLightEntity);
		AddView(directionalLightEntity, SVolumetricLightComponentView::View);
		volumetricLight.IsActive = false;
		// === !Directional light ===

		// === Point light ===
		const SEntity& pointLightEntity = AddEntity("Point Light");
		if (!pointLightEntity.IsValid())
			return true; // From this point it's ok if we fail to load the rest of the demo scene

		STransformComponent& pointLightTransform = *AddComponent<STransformComponent>(pointLightEntity);
		AddView(pointLightEntity, STransformComponentView::View);
		SMatrix pointLightMatrix = pointLightTransform.Transform.GetMatrix();
		pointLightMatrix.SetTranslation({ 1.25f, 0.35f, -1.65f });
		pointLightTransform.Transform.SetMatrix(pointLightMatrix);


		SPointLightComponent& pointLightComp = *AddComponent<SPointLightComponent>(pointLightEntity);
		AddView(pointLightEntity, SPointLightComponentView::View);
		pointLightComp.ColorAndIntensity = { 0.0f, 1.0f, 1.0f, 10.0f };
		pointLightComp.Range = 1.0f;

		SVolumetricLightComponent& volumetricPointLight = *AddComponent<SVolumetricLightComponent>(pointLightEntity);
		AddView(pointLightEntity, SVolumetricLightComponentView::View);
		volumetricPointLight.IsActive = false;

		const SMatrix constantProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(90.0f), 1.0f, 0.001f, pointLightComp.Range);
		const SVector4 constantPosition = pointLightTransform.Transform.GetMatrix().GetTranslation4();

		// Forward
		SShadowmapViewData& view1 = pointLightComp.ShadowmapViews[0];
		view1.ShadowPosition = constantPosition;
		view1.ShadowmapViewportIndex = 1;
		view1.ShadowViewMatrix = SMatrix::LookAtLH(constantPosition.ToVector3(), (constantPosition + SVector4::Forward).ToVector3(), SVector::Up);
		view1.ShadowProjectionMatrix = constantProjectionMatrix;

		// Right
		SShadowmapViewData& view2 = pointLightComp.ShadowmapViews[1];
		view2.ShadowPosition = constantPosition;
		view2.ShadowmapViewportIndex = 2;
		view2.ShadowViewMatrix = SMatrix::LookAtLH(constantPosition.ToVector3(), (constantPosition + SVector4::Right).ToVector3(), SVector::Up);
		view2.ShadowProjectionMatrix = constantProjectionMatrix;

		// Backward
		SShadowmapViewData& view3 = pointLightComp.ShadowmapViews[2];
		view3.ShadowPosition = constantPosition;
		view3.ShadowmapViewportIndex = 3;
		view3.ShadowViewMatrix = SMatrix::LookAtLH(constantPosition.ToVector3(), (constantPosition + SVector4::Backward).ToVector3(), SVector::Up);
		view3.ShadowProjectionMatrix = constantProjectionMatrix;

		// Left
		SShadowmapViewData& view4 = pointLightComp.ShadowmapViews[3];
		view4.ShadowPosition = constantPosition;
		view4.ShadowmapViewportIndex = 4;
		view4.ShadowViewMatrix = SMatrix::LookAtLH(constantPosition.ToVector3(), (constantPosition + SVector4::Left).ToVector3(), SVector::Up);
		view4.ShadowProjectionMatrix = constantProjectionMatrix;

		// Up
		SShadowmapViewData& view5 = pointLightComp.ShadowmapViews[4];
		view5.ShadowPosition = constantPosition;
		view5.ShadowmapViewportIndex = 5;
		view5.ShadowViewMatrix = SMatrix::LookAtLH(constantPosition.ToVector3(), (constantPosition + SVector4::Up).ToVector3(), SVector::Backward);
		view5.ShadowProjectionMatrix = constantProjectionMatrix;

		// Down
		SShadowmapViewData& view6 = pointLightComp.ShadowmapViews[5];
		view6.ShadowPosition = constantPosition;
		view6.ShadowmapViewportIndex = 6;
		view6.ShadowViewMatrix = SMatrix::LookAtLH(constantPosition.ToVector3(), (constantPosition + SVector4::Down).ToVector3(), SVector::Forward);
		view6.ShadowProjectionMatrix = constantProjectionMatrix;
		// === !Point light ===

		// === Spotlight ===
		const SEntity& spotlight = AddEntity("Spot Light");
		if (!spotlight.IsValid())
			return true;

		STransform& spotlightTransform = (*AddComponent<STransformComponent>(spotlight)).Transform;
		AddView(spotlight, STransformComponentView::View);
		SMatrix spotlightMatrix = spotlightTransform.GetMatrix();
		spotlightMatrix.SetTranslation({ 1.5f, 0.5f, -1.0f });
		spotlightTransform.SetMatrix(spotlightMatrix);

		SSpotLightComponent& spotlightComp = *AddComponent<SSpotLightComponent>(spotlight);
		AddView(spotlight, SSpotLightComponentView::View);
		spotlightComp.Direction = SVector4::Forward;
		spotlightComp.DirectionNormal1 = SVector4::Right;
		spotlightComp.DirectionNormal2 = SVector4::Up;
		spotlightComp.ColorAndIntensity = { 0.0f, 1.0f, 0.0f, 5.0f };
		spotlightComp.OuterAngle = 25.0f;
		spotlightComp.InnerAngle = 5.0f;
		spotlightComp.Range = 3.0f;

		SVolumetricLightComponent& volumetricSpotLight = *AddComponent<SVolumetricLightComponent>(spotlight);
		AddView(spotlight, SVolumetricLightComponentView::View);
		volumetricSpotLight.IsActive = false;

		const SMatrix spotlightProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(90.0f), 1.0f, 0.001f, spotlightComp.Range);
		const SVector4 spotlightPosition = spotlightTransform.GetMatrix().GetTranslation4();

		spotlightComp.ShadowmapView.ShadowPosition = spotlightPosition;
		spotlightComp.ShadowmapView.ShadowmapViewportIndex = 7;
		spotlightComp.ShadowmapView.ShadowViewMatrix = SMatrix::LookAtLH(spotlightPosition.ToVector3(), (spotlightPosition + spotlightComp.Direction).ToVector3(), spotlightComp.DirectionNormal2.ToVector3());
		spotlightComp.ShadowmapView.ShadowProjectionMatrix = spotlightProjection;
		// === !Spotlight ===

		// === Decal ===
		const SEntity& decal = AddEntity("Decal");
		if (!decal.IsValid())
			return true;

		STransform& decalTransform = (*AddComponent<STransformComponent>(decal)).Transform;
		AddView(decal, STransformComponentView::View);
		decalTransform.Translate({ 0.45f, 1.60f, 0.85f });

		SDecalComponent& decalComp = *AddComponent<SDecalComponent>(decal);
		AddView(decal, SDecalComponentView::View);

		std::vector<std::string> decalTextures = { "Assets/Textures/T_noscare_AL_c.hva", "Assets/Textures/T_noscare_AL_m.hva", "Assets/Textures/T_noscare_AL_n.hva" };
		renderManager->LoadDecalComponent(decalTextures, &decalComp);
		decalComp.ShouldRenderAlbedo = true;
		decalComp.ShouldRenderMaterial = true;
		decalComp.ShouldRenderNormal = true;

		decalComp.AssetRegistryKeys = assetRegistry->Register(decalTextures);
		// === !Decal ===

		const std::string modelPath1 = "Assets/Tests/En_P_PendulumClock.hva";
		const std::vector<std::string> materialNames1 = { "Assets/Materials/M_PendulumClock.hva", "Assets/Materials/M_Checkboard_128x128.hva" };
		const std::string modelPath2 = "Assets/Tests/En_P_Bed.hva";
		const std::vector<std::string> materialNames2 = { "Assets/Materials/M_Bed.hva", "Assets/Materials/M_Bedsheet.hva" };
		const std::string modelPath3 = "Assets/Tests/Quad.hva";
		const std::vector<std::string> materialNames3 = { "Assets/Materials/M_Quad.hva" };
		const std::string modelPath4 = "Assets/Tests/En_P_WallLamp.hva";
		const std::vector<std::string> materialNames4 = { "Assets/Materials/M_Quad.hva", "Assets/Materials/M_Emissive.hva", "Assets/Materials/M_Headlamp.hva" };

		// === Pendulum ===

		// TODO: GetNewEntityIndex which provides comp index directly

		const SEntity& pendulum = AddEntity("Clock");
		if (!pendulum.IsValid())
			return false;

		STransform& transform1 = (*AddComponent<STransformComponent>(pendulum)).Transform;
		transform1.Translate({ 1.75f, 0.0f, 0.25f });

		renderManager->LoadStaticMeshComponent(modelPath1, AddComponent<SStaticMeshComponent>(pendulum));
		AddView(pendulum, SStaticMeshComponentView::View);
		renderManager->LoadMaterialComponent(materialNames1, AddComponent<SMaterialComponent>(pendulum));
		AddView(pendulum, SMaterialComponentView::View);

		GetComponent<SStaticMeshComponent>(pendulum)->AssetRegistryKey = assetRegistry->Register(modelPath1);
		GetComponent<SMaterialComponent>(pendulum)->AssetRegistryKeys = assetRegistry->Register(materialNames1);
		// === !Pendulum ===

		// === Bed ===
		const SEntity& bed = AddEntity("Bed");
		if (!bed.IsValid())
			return false;

		STransform& transform2 = (*AddComponent<STransformComponent>(bed)).Transform;
		AddView(bed, STransformComponentView::View);
		transform2.Translate({ 0.25f, 0.0f, 0.25f });

		renderManager->LoadStaticMeshComponent(modelPath2, AddComponent<SStaticMeshComponent>(bed));
		AddView(bed, SStaticMeshComponentView::View);
		renderManager->LoadMaterialComponent(materialNames2, AddComponent<SMaterialComponent>(bed));
		AddView(bed, SMaterialComponentView::View);

		GetComponent<SStaticMeshComponent>(bed)->AssetRegistryKey = assetRegistry->Register(modelPath2);
		GetComponent<SMaterialComponent>(bed)->AssetRegistryKeys = assetRegistry->Register(materialNames2);
		// === !Bed ===

		// === Lamp ===
		const SEntity& lamp = AddEntity("Lamp");
		if (!lamp.IsValid())
			return false;

		STransform& lampTransform = (*AddComponent<STransformComponent>(lamp)).Transform;
		lampTransform.Translate({ -1.0f, 1.4f, -0.75f });
		lampTransform.Rotate({ 0.0f, UMath::DegToRad(90.0f), 0.0f });

		renderManager->LoadStaticMeshComponent(modelPath4, AddComponent<SStaticMeshComponent>(lamp));
		AddView(lamp, SStaticMeshComponentView::View);
		renderManager->LoadMaterialComponent(materialNames4, AddComponent<SMaterialComponent>(lamp));
		AddView(lamp, SMaterialComponentView::View);

		GetComponent<SStaticMeshComponent>(lamp)->AssetRegistryKey = assetRegistry->Register(modelPath4);
		GetComponent<SMaterialComponent>(lamp)->AssetRegistryKeys = assetRegistry->Register(materialNames4);
		// === !Lamp ===

		// === Floor ===
		std::vector<SVector> translations;
		translations.emplace_back(-1.0f, 0.0f, -2.0f);
		translations.emplace_back(0.0f, 0.0f, -2.0f);
		translations.emplace_back(1.0f, 0.0f, -2.0f);
		translations.emplace_back(2.0f, 0.0f, -2.0f);
		translations.emplace_back(-1.0f, 0.0f, -1.0f);
		translations.emplace_back(0.0f, 0.0f, -1.0f);
		translations.emplace_back(1.0f, 0.0f, -1.0f);
		translations.emplace_back(2.0f, 0.0f, -1.0f);
		translations.emplace_back(-1.0f, 0.0f, 0.0f);
		translations.emplace_back(0.0f, 0.0f, 0.0f);
		translations.emplace_back(1.0f, 0.0f, 0.0f);
		translations.emplace_back(2.0f, 0.0f, 0.0f);

		for (U8 i = 0; i < 12; ++i)
		{
			const SEntity& floor = AddEntity("Floor");
			if (!floor.IsValid())
				return false;

			STransform& transform3 = (*AddComponent<STransformComponent>(floor)).Transform;
			AddView(floor, STransformComponentView::View);
			SMatrix matrix3 = transform3.GetMatrix();
			matrix3.SetTranslation(translations[i]);
			matrix3.SetRotation(SMatrix::CreateRotationAroundZ(UMath::DegToRad(-90.0f)));
			transform3.SetMatrix(matrix3);

			renderManager->LoadStaticMeshComponent(modelPath3, AddComponent<SStaticMeshComponent>(floor));
			AddView(floor, SMaterialComponentView::View);
			renderManager->LoadMaterialComponent(materialNames3, AddComponent<SMaterialComponent>(floor));
			AddView(floor, SMaterialComponentView::View);

			GetComponent<SStaticMeshComponent>(floor)->AssetRegistryKey = assetRegistry->Register(modelPath3);
			GetComponent<SMaterialComponent>(floor)->AssetRegistryKeys = assetRegistry->Register(materialNames3);
		}
		// === !Floor ===

		// === Wall ===
		translations.clear();
		translations.emplace_back(-1.0f, 0.5f, 0.5f);
		translations.emplace_back(0.0f, 0.5f, 0.5f);
		translations.emplace_back(1.0f, 0.5f, 0.5f);
		translations.emplace_back(2.0f, 0.5f, 0.5f);
		translations.emplace_back(-1.0f, 1.5f, 0.5f);
		translations.emplace_back(0.0f, 1.5f, 0.5f);
		translations.emplace_back(1.0f, 1.5f, 0.5f);
		translations.emplace_back(2.0f, 1.5f, 0.5f);
		translations.emplace_back(-1.0f, 2.5f, 0.5f);
		translations.emplace_back(0.0f, 2.5f, 0.5f);
		translations.emplace_back(1.0f, 2.5f, 0.5f);
		translations.emplace_back(2.0f, 2.5f, 0.5f);

		for (U8 i = 0; i < 12; ++i)
		{
			const SEntity& floor = AddEntity("Wall");
			if (!floor.IsValid())
				return false;

			STransform& transform3 = (*AddComponent<STransformComponent>(floor)).Transform;
			AddView(floor, STransformComponentView::View);
			SMatrix matrix3 = transform3.GetMatrix();
			matrix3.SetTranslation(translations[i]);
			matrix3.SetRotation(SMatrix::CreateRotationAroundZ(UMath::DegToRad(-90.0f)) * SMatrix::CreateRotationAroundX(UMath::DegToRad(-90.0f)));
			transform3.SetMatrix(matrix3);

			renderManager->LoadStaticMeshComponent(modelPath3, AddComponent<SStaticMeshComponent>(floor));
			AddView(floor, SMaterialComponentView::View);
			renderManager->LoadMaterialComponent(materialNames3, AddComponent<SMaterialComponent>(floor));
			AddView(floor, SMaterialComponentView::View);

			GetComponent<SStaticMeshComponent>(floor)->AssetRegistryKey = assetRegistry->Register(modelPath3);
			GetComponent<SMaterialComponent>(floor)->AssetRegistryKeys = assetRegistry->Register(materialNames3);
		}
		// === !Wall ===

		// === Other Wall ===
		translations.clear();
		translations.emplace_back(-1.0f, 0.5f, -2.5f);
		translations.emplace_back(-1.0f, 0.5f, -1.5f);
		translations.emplace_back(-1.0f, 0.5f, -0.5f);
		translations.emplace_back(-1.0f, 1.5f, -2.5f);
		translations.emplace_back(-1.0f, 1.5f, -1.5f);
		translations.emplace_back(-1.0f, 1.5f, -0.5f);
		translations.emplace_back(-1.0f, 2.5f, -2.5f);
		translations.emplace_back(-1.0f, 2.5f, -1.5f);
		translations.emplace_back(-1.0f, 2.5f, -0.5f);

		for (U8 i = 0; i < 9; ++i)
		{
			const SEntity& floor = AddEntity("Wall");
			if (!floor.IsValid())
				return false;

			STransform& transform3 = (*AddComponent<STransformComponent>(floor)).Transform;
			AddView(floor, STransformComponentView::View);
			SMatrix matrix3 = transform3.GetMatrix();
			matrix3.SetTranslation(translations[i]);
			matrix3.SetRotation(SMatrix::CreateRotationAroundZ(UMath::DegToRad(-90.0f)) * SMatrix::CreateRotationAroundX(UMath::DegToRad(-90.0f)) * SMatrix::CreateRotationAroundY(UMath::DegToRad(-90.0f)));
			transform3.SetMatrix(matrix3);

			renderManager->LoadStaticMeshComponent(modelPath3, AddComponent<SStaticMeshComponent>(floor));
			AddView(floor, SMaterialComponentView::View);
			renderManager->LoadMaterialComponent(materialNames3, AddComponent<SMaterialComponent>(floor));
			AddView(floor, SMaterialComponentView::View);

			GetComponent<SStaticMeshComponent>(floor)->AssetRegistryKey = assetRegistry->Register(modelPath3);
			GetComponent<SMaterialComponent>(floor)->AssetRegistryKeys = assetRegistry->Register(materialNames3);
		}
		// === !Other Wall ===

		return true;
	}

	bool CScene::Init2DDemoScene(CRenderManager* renderManager)
	{
		SceneName = std::string("2DDemoScene");

		// === Camera ===
		MainCameraEntity = AddEntity("Camera");
		if (!MainCameraEntity.IsValid())
			return false;

		CAssetRegistry* assetRegistry = GEngine::GetWorld()->GetAssetRegistry();

		// Setup entities (create components)
		STransformComponent& transform = (*AddComponent<STransformComponent>(MainCameraEntity));
		AddView(MainCameraEntity, STransformComponentView::View);
		transform.Transform.Translate({ 0.0f, 1.0f, -5.0f });
		//transform.Transform.Rotate({ 0.0f, UMath::DegToRad(35.0f), 0.0f });
		transform.Transform.Translate(SVector::Right * 0.25f);

		SCameraComponent& camera = *AddComponent<SCameraComponent>(MainCameraEntity);
		AddView(MainCameraEntity, SCameraComponentView::View);
		camera.ProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), (16.0f / 9.0f), 0.1f, 1000.0f);
		//camera.ProjectionType = ECameraProjectionType::Orthographic;
		//camera.ProjectionMatrix = SMatrix::OrthographicLH(5.0f, 5.0f, 0.1f, 1000.0f);
		camera.ViewMatrix = SMatrix::LookAtLH(SVector::Zero, SVector::Forward, SVector::Up);

		//		SCameraControllerComponent& controllerComp = 
		AddComponent<SCameraControllerComponent>(MainCameraEntity);
		AddView(MainCameraEntity, SCameraControllerComponentView::View);
		//controllerComp.CurrentYaw = UMath::DegToRad(-35.0f);
		// 
		//SSequencerComponent& cameraSequencerComponent = AddSequencerComponentToEntity(*cameraEntity);
		//cameraSequencerComponent.ComponentTracks.push_back({ EComponentType::TransformComponent });
		// === !Camera ===

		// === Environment light ===
		const SEntity& environmentLightEntity = AddEntity("Environment Light");
		if (!environmentLightEntity.IsValid())
			return false;

		AddComponent<STransformComponent>(environmentLightEntity);
		AddView(environmentLightEntity, STransformComponentView::View);
		renderManager->LoadEnvironmentLightComponent("Assets/Textures/Cubemaps/CubemapTheVisit.hva", AddComponent<SEnvironmentLightComponent>(environmentLightEntity));
		AddView(environmentLightEntity, SEnvironmentLightComponentView::View);
		GetComponent<SEnvironmentLightComponent>(environmentLightEntity)->AssetRegistryKey = assetRegistry->Register("Assets/Textures/Cubemaps/CubemapTheVisit.hva");
		// === !Environment light ===

		// === Directional light ===
		const SEntity& directionalLightEntity = AddEntity("Directional Light");
		if (!directionalLightEntity.IsValid())
			return false;

		// NR: Add transform to directional light so it can filter environmental lights based on distance
		AddComponent<STransformComponent>(directionalLightEntity);
		AddView(directionalLightEntity, STransformComponentView::View);

		SDirectionalLightComponent& directionalLight = *AddComponent<SDirectionalLightComponent>(directionalLightEntity);
		AddView(directionalLightEntity, SDirectionalLightComponentView::View);
		directionalLight.Direction = { 1.0f, 1.0f, -1.0f, 0.0f };
		directionalLight.Color = { 212.0f / 255.0f, 175.0f / 255.0f, 55.0f / 255.0f, 0.25f };
		directionalLight.ShadowmapView.ShadowmapViewportIndex = 0;
		directionalLight.ShadowmapView.ShadowProjectionMatrix = SMatrix::OrthographicLH(directionalLight.ShadowViewSize.X, directionalLight.ShadowViewSize.Y, directionalLight.ShadowNearAndFarPlane.X, directionalLight.ShadowNearAndFarPlane.Y);

		SVolumetricLightComponent& volumetricLight = *AddComponent<SVolumetricLightComponent>(directionalLightEntity);
		AddView(directionalLightEntity, SVolumetricLightComponentView::View);
		volumetricLight.IsActive = false;
		// === !Directional light ===

		// === Spotlight ===
		const SEntity& spotlight = AddEntity("Spot Light");
		if (!spotlight.IsValid())
			return true;

		STransform& spotlightTransform = (*AddComponent<STransformComponent>(spotlight)).Transform;
		AddView(spotlight, STransformComponentView::View);
		SMatrix spotlightMatrix = spotlightTransform.GetMatrix();
		spotlightMatrix.SetTranslation({ 0.0f, 0.0f, 0.0f });
		spotlightTransform.SetMatrix(spotlightMatrix);

		SSpotLightComponent& spotlightComp = *AddComponent<SSpotLightComponent>(spotlight);
		AddView(spotlight, SSpotLightComponentView::View);
		spotlightComp.Direction = SVector4::Forward;
		spotlightComp.DirectionNormal1 = SVector4::Right;
		spotlightComp.DirectionNormal2 = SVector4::Up;
		spotlightComp.ColorAndIntensity = { 0.0f, 1.0f, 0.0f, 5.0f };
		spotlightComp.OuterAngle = 40.0f;
		spotlightComp.InnerAngle = 30.0f;
		spotlightComp.Range = 3.0f;

		SVolumetricLightComponent& volumetricSpotLight = *AddComponent<SVolumetricLightComponent>(spotlight);
		AddView(spotlight, SVolumetricLightComponentView::View);
		volumetricSpotLight.IsActive = false;

		const SMatrix spotlightProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(90.0f), 1.0f, 0.001f, spotlightComp.Range);
		const SVector4 spotlightPosition = spotlightTransform.GetMatrix().GetTranslation4();

		spotlightComp.ShadowmapView.ShadowPosition = spotlightPosition;
		spotlightComp.ShadowmapView.ShadowmapViewportIndex = 7;
		spotlightComp.ShadowmapView.ShadowViewMatrix = SMatrix::LookAtLH(spotlightPosition.ToVector3(), (spotlightPosition + spotlightComp.Direction).ToVector3(), spotlightComp.DirectionNormal2.ToVector3());
		spotlightComp.ShadowmapView.ShadowProjectionMatrix = spotlightProjection;

		//SSequencerComponent& spotlightSequencerComponent = AddSequencerComponentToEntity(*spotlight);
		//spotlightSequencerComponent.ComponentTracks.push_back({ EComponentType::SpotLightComponent });
		//spotlightSequencerComponent.ComponentTracks.push_back({ EComponentType::DecalComponent });
		// === !Spotlight ===

		//{
		//	SEntity* spriteWS = GetNewEntity("SpriteWS");
		//	if (!spriteWS)
		//		return true;

		//	STransformComponent& spriteWStransform = AddTransformComponentToEntity(*spriteWS);
		//	spriteWStransform.Transform.Move({ -2.0f, 0.0f, 2.0f });
		//	F32 radians = UMath::DegToRad(180.0f);
		//	spriteWStransform.Transform.Rotate({ radians, radians, radians });

		//	const std::string& spritePath = "Assets/Textures/T_Checkboard_128x128_c.hva";
		//	SSpriteComponent& spriteWSComp = AddSpriteComponentToEntity(*spriteWS);
		//	spriteWSComp.UVRect = { 0.0f, 0.0f, 2.0f, 2.0f };
		//	renderManager->LoadSpriteComponent(spritePath, &spriteWSComp);

		//	U16 spriteIndex = static_cast<U16>(GetSceneIndex(*spriteWS));
		//	assetRegistry->Register(spritePath, SAssetReferenceCounter(EComponentType::SpriteComponent, spriteIndex, 0, 0));
		//}
		//// === !World Space Sprite ===

		return true;
	}

	U32 CScene::GetSize() const
	{
		U32 size = 0;

		size += sizeof(CHavtornStaticString<255>);

		size += sizeof(U64);
		size += sizeof(SEntity) * static_cast<U32>(Entities.size());

		size += sizeof(U64);
		size += sizeof(STransformComponent) * static_cast<U32>(GetComponents<STransformComponent>().size());

		size += sizeof(U64);
		for (const auto component : GetComponents<SStaticMeshComponent>())
			size += component->GetSize();

		size += sizeof(U64);
		size += sizeof(SCameraComponent) * static_cast<U32>(GetComponents<SCameraComponent>().size());

		size += sizeof(U64);
		size += sizeof(SCameraControllerComponent) * static_cast<U32>(GetComponents<SCameraControllerComponent>().size());

		size += sizeof(U64);
		for (const auto component : GetComponents<SMaterialComponent>())
			size += component->GetSize();

		size += sizeof(U64);
		size += sizeof(SEnvironmentLightComponent) * static_cast<U32>(GetComponents<SEnvironmentLightComponent>().size());

		size += sizeof(U64);
		size += sizeof(SDirectionalLightComponent) * static_cast<U32>(GetComponents<SDirectionalLightComponent>().size());

		size += sizeof(U64);
		size += sizeof(SPointLightComponent) * static_cast<U32>(GetComponents<SPointLightComponent>().size());

		size += sizeof(U64);
		size += sizeof(SSpotLightComponent) * static_cast<U32>(GetComponents<SSpotLightComponent>().size());

		size += sizeof(U64);
		size += sizeof(SVolumetricLightComponent) * static_cast<U32>(GetComponents<SVolumetricLightComponent>().size());

		size += sizeof(U64);
		for (const auto component : GetComponents<SDecalComponent>())
			size += component->GetSize();

		size += sizeof(U64);
		size += sizeof(SSpriteComponent) * static_cast<U32>(GetComponents<SSpriteComponent>().size());

		size += sizeof(U64);
		size += sizeof(STransform2DComponent) * static_cast<U32>(GetComponents<STransform2DComponent>().size());

		// TODO.AS: Implement GetSize (since the component is not trivially serializable)
		size += sizeof(U64);
		//size += sizeof(STransform2DComponent) * static_cast<U32>(GetComponents<SSpriteAnimatorGraphComponent>().size());

		// TODO.NR: Implement GetSize (since the component is not trivially serializable)
		size += sizeof(U64);
		//size += sizeof(STransform2DComponent) * static_cast<U32>(GetComponents<SSequencerComponent>().size());

		size += sizeof(U64);
		size += sizeof(SMetaDataComponent) * static_cast<U32>(GetComponents<SMetaDataComponent>().size());

		return size;
	}

	void CScene::Serialize(char* toData, U64& pointerPosition) const
	{
		// Specialize Scene into CScene and CGameScene : public CScene.
		// When serializing, always follow strict protocol. Start with:
		// 1) Number of entities. This is trivial as they are only wrappers for U64s.
		// 2) A protocol over which and how many components are in the scene. E.g.
		//		Number of STransformComponents. Serialize as needed
		//		Number of SStaticMeshComponents. Serialize as needed
		// After all engine components are serialized, the overloaded function in CGameScene
		// should continue with its components according to a similar protocol, serializing
		// whole collections of its own components. This way we do not need to care about 
		// serializing an identifier for every specific component.

		// Mockup for how we imagine the saved data. Will use simplified examples of components (e.g Tranform is only a vec3 position)
		/*
			4								// There are 4 entities
			1987654
			1234567
			2345678
			5788653
			3								// There are 3 STransformComponent(s)
			0,0,0, 1987654
			1,1,1, 1234567
			2,2,2, 5788653
			2
			sprites/square.png, 1987654
			sprites/circle.png, 5788653

		*/
		SerializeData(SceneName, toData, pointerPosition);

		SerializeData(Entities.size(), toData, pointerPosition);
		for (const auto& entity : Entities)
			SerializeData(entity.GUID, toData, pointerPosition);

		const auto& transformComponents = GetComponents<STransformComponent>();
		SerializeData(transformComponents.size(), toData, pointerPosition);
		for (const auto component : transformComponents)
			SerializeData(*component, toData, pointerPosition);

		// NR: Saved and loaded using AssetRegistry
		const auto& staticMeshComponents = GetComponents<SStaticMeshComponent>();
		SerializeData(staticMeshComponents.size(), toData, pointerPosition);
		for (auto component : staticMeshComponents)
			component->Serialize(toData, pointerPosition);

		const auto& cameraComponents = GetComponents<SCameraComponent>();
		SerializeData(cameraComponents.size(), toData, pointerPosition);
		for (const auto component : cameraComponents)
			SerializeData(*component, toData, pointerPosition);

		const auto& cameraControllerComponents = GetComponents<SCameraControllerComponent>();
		SerializeData(cameraControllerComponents.size(), toData, pointerPosition);
		for (const auto component : cameraControllerComponents)
			SerializeData(*component, toData, pointerPosition);

		const auto& materialComponents = GetComponents<SMaterialComponent>();
		SerializeData(materialComponents.size(), toData, pointerPosition);
		for (auto component : materialComponents)
			component->Serialize(toData, pointerPosition);

		const auto& environmentLightComponents = GetComponents<SEnvironmentLightComponent>();
		SerializeData(environmentLightComponents.size(), toData, pointerPosition);
		for (auto component : environmentLightComponents)
			SerializeData(*component, toData, pointerPosition);

		const auto& directionalLightComponents = GetComponents<SDirectionalLightComponent>();
		SerializeData(directionalLightComponents.size(), toData, pointerPosition);
		for (const auto component : directionalLightComponents)
			SerializeData(*component, toData, pointerPosition);

		const auto& pointLightComponents = GetComponents<SPointLightComponent>();
		SerializeData(pointLightComponents.size(), toData, pointerPosition);
		for (const auto component : pointLightComponents)
			SerializeData(*component, toData, pointerPosition);

		const auto& spotLightComponents = GetComponents<SSpotLightComponent>();
		SerializeData(spotLightComponents.size(), toData, pointerPosition);
		for (const auto component : spotLightComponents)
			SerializeData(*component, toData, pointerPosition);

		const auto& volumetricLightComponents = GetComponents<SVolumetricLightComponent>();
		SerializeData(volumetricLightComponents.size(), toData, pointerPosition);
		for (const auto component : volumetricLightComponents)
			SerializeData(*component, toData, pointerPosition);

		// NR: Texture info Saved and Loaded using AssetRegistry
		const auto& decalComponents = GetComponents<SDecalComponent>();
		SerializeData(decalComponents.size(), toData, pointerPosition);
		for (auto component : decalComponents)
			component->Serialize(toData, pointerPosition);

		// NR: Texture info Saved and Loaded using AssetRegistry
		const auto& spriteComponents = GetComponents<SSpriteComponent>();
		SerializeData(spriteComponents.size(), toData, pointerPosition);
		for (auto component : spriteComponents)
			SerializeData(*component, toData, pointerPosition);

		const auto& transform2DComponents = GetComponents<STransform2DComponent>();
		SerializeData(transform2DComponents.size(), toData, pointerPosition);
		for (auto component : transform2DComponents)
			SerializeData(*component, toData, pointerPosition);
		
		// TODO.AS: Implement Serialize (since the component is not trivially serializable)
		const auto& spriteAnimatorGraphComponents = GetComponents<SSpriteAnimatorGraphComponent>();
		SerializeData(spriteAnimatorGraphComponents.size(), toData, pointerPosition);
		//for (auto component : spriteAnimatorGraphComponents)
		//	component->Serialize(toData, pointerPosition);

		// TODO.NR: Implement Serialize (since the component is not trivially serializable)
		const auto& sequencerComponents = GetComponents<SSequencerComponent>();
		SerializeData(sequencerComponents.size(), toData, pointerPosition);
		//for (auto component : sequencerComponents)
		//	component->Serialize(toData, pointerPosition);

		const auto& metaDataComponents = GetComponents<SMetaDataComponent>();
		SerializeData(metaDataComponents.size(), toData, pointerPosition);
		for (auto component : metaDataComponents)
			SerializeData(*component, toData, pointerPosition);
	}

	void CScene::Deserialize(const char* fromData, U64& pointerPosition, CAssetRegistry* assetRegistry)
	{
		DeserializeData(SceneName, fromData, pointerPosition);

		U64 numberOfEntities = 0;
		DeserializeData(numberOfEntities, fromData, pointerPosition);
		for (U64 index = 0; index < numberOfEntities; index++)
		{
			U64 guid = 0;
			DeserializeData(guid, fromData, pointerPosition);

			AddEntity(guid);
		}

		U64 numberOfTransformComponents = 0;
		DeserializeData(numberOfTransformComponents, fromData, pointerPosition);
		std::vector<STransformComponent> transformComponents;
		//std::vector<STransformComponent> transformComponents = std::vector<STransformComponent>(100, STransformComponent(SEntity::Null));
		//memcpy(transformComponents.data(), &fromData[pointerPosition], sizeof(STransformComponent) * static_cast<U32>(numberOfTransformComponents));
		DeserializeData(transformComponents, fromData, static_cast<U32>(numberOfTransformComponents), pointerPosition);
		for (const auto& component : transformComponents)
			AddComponent<STransformComponent>(component.EntityOwner);

		U64 numberOfStaticMeshComponents = 0;
		DeserializeData(numberOfStaticMeshComponents, fromData, pointerPosition);
		std::vector<SStaticMeshComponent> staticMeshComponents;
		staticMeshComponents.resize(numberOfStaticMeshComponents);
		for (auto& component : staticMeshComponents)
		{
			component.Deserialize(fromData, pointerPosition);
			RenderManager->LoadStaticMeshComponent(assetRegistry->GetAssetPath(component.AssetRegistryKey), AddComponent<SStaticMeshComponent>(component.EntityOwner));
		}

		U64 numberOfCameraComponents = 0;
		DeserializeData(numberOfCameraComponents, fromData, pointerPosition);
		std::vector<SCameraComponent> cameraComponents;
		DeserializeData(cameraComponents, fromData, static_cast<U32>(numberOfCameraComponents), pointerPosition);
		for (const auto& component : cameraComponents)
			AddComponent<SCameraComponent>(component.EntityOwner);

		U64 numberOfCameraControllerComponents = 0;
		DeserializeData(numberOfCameraControllerComponents, fromData, pointerPosition);
		std::vector<SCameraControllerComponent> cameraControllerComponents;
		DeserializeData(cameraControllerComponents, fromData, static_cast<U32>(numberOfCameraControllerComponents), pointerPosition);
		for (const auto& component : cameraControllerComponents)
			AddComponent<SCameraControllerComponent>(component.EntityOwner);

		U64 numberOfMaterialComponents = 0;
		DeserializeData(numberOfMaterialComponents, fromData, pointerPosition);
		std::vector<SMaterialComponent> materialComponents;
		materialComponents.resize(numberOfMaterialComponents);
		for (auto& component : materialComponents)
		{
			component.Deserialize(fromData, pointerPosition);
			RenderManager->LoadMaterialComponent(assetRegistry->GetAssetPaths(component.AssetRegistryKeys), AddComponent<SMaterialComponent>(component.EntityOwner));
		}

		U64 numberOfEnvironmentLightComponents = 0;
		DeserializeData(numberOfEnvironmentLightComponents, fromData, pointerPosition);
		std::vector<SEnvironmentLightComponent> environmentLightComponent;
		environmentLightComponent.resize(numberOfEnvironmentLightComponents);
		for (auto& component : environmentLightComponent)
		{
			DeserializeData(component, fromData, pointerPosition);
			RenderManager->LoadEnvironmentLightComponent(assetRegistry->GetAssetPath(component.AssetRegistryKey), AddComponent<SEnvironmentLightComponent>(component.EntityOwner));
		}

		U64 numberOfDirectionalLightComponents = 0;
		DeserializeData(numberOfDirectionalLightComponents, fromData, pointerPosition);
		std::vector<SDirectionalLightComponent> directionalLightComponent;
		DeserializeData(directionalLightComponent, fromData, static_cast<U32>(numberOfDirectionalLightComponents), pointerPosition);
		for (const auto& component : directionalLightComponent)
			AddComponent<SDirectionalLightComponent>(component.EntityOwner);

		U64 numberOfPointLightComponents = 0;
		DeserializeData(numberOfPointLightComponents, fromData, pointerPosition);
		std::vector<SPointLightComponent> pointLightComponents;
		DeserializeData(pointLightComponents, fromData, static_cast<U32>(numberOfPointLightComponents), pointerPosition);
		for (const auto& component : pointLightComponents)
			AddComponent<SPointLightComponent>(component.EntityOwner);

		U64 numberOfSpotLightComponents = 0;
		DeserializeData(numberOfSpotLightComponents, fromData, pointerPosition);
		std::vector<SSpotLightComponent> spotLightComponents;
		DeserializeData(spotLightComponents, fromData, static_cast<U32>(numberOfSpotLightComponents), pointerPosition);
		for (const auto& component : spotLightComponents)
			AddComponent<SSpotLightComponent>(component.EntityOwner);

		U64 numberOfVolumetricLightComponents = 0;
		DeserializeData(numberOfVolumetricLightComponents, fromData, pointerPosition);
		std::vector<SVolumetricLightComponent> volumetricLightComponents;
		DeserializeData(volumetricLightComponents, fromData, static_cast<U32>(numberOfVolumetricLightComponents), pointerPosition);
		for (const auto& component : volumetricLightComponents)
			AddComponent<SVolumetricLightComponent>(component.EntityOwner);

		U64 numberOfDecalComponents = 0;
		DeserializeData(numberOfDecalComponents, fromData, pointerPosition);
		std::vector<SDecalComponent> decalComponents;
		decalComponents.resize(numberOfDecalComponents);
		for (auto& component : decalComponents)
		{
			DeserializeData(component, fromData, pointerPosition);
			RenderManager->LoadDecalComponent(assetRegistry->GetAssetPaths(component.AssetRegistryKeys), AddComponent<SDecalComponent>(component.EntityOwner));
		}

		U64 numberOfSpriteComponents = 0;
		DeserializeData(numberOfSpriteComponents, fromData, pointerPosition);
		std::vector<SSpriteComponent> spriteComponents;
		spriteComponents.resize(numberOfSpriteComponents);
		for (auto& component : spriteComponents)
		{
			DeserializeData(component, fromData, pointerPosition);
			RenderManager->LoadSpriteComponent(assetRegistry->GetAssetPath(component.AssetRegistryKey), AddComponent<SSpriteComponent>(component.EntityOwner));
		}

		U64 numberOfTransform2DComponents = 0;
		DeserializeData(numberOfTransform2DComponents, fromData, pointerPosition);
		std::vector<STransform2DComponent> transform2DComponents;
		DeserializeData(transform2DComponents, fromData, static_cast<U32>(numberOfTransform2DComponents), pointerPosition);
		for (const auto& component : transform2DComponents)
			AddComponent<STransform2DComponent>(component.EntityOwner);

		U64 numberOfSpriteAnimatorGraphComponents = 0;
		DeserializeData(numberOfSpriteAnimatorGraphComponents, fromData, pointerPosition);
		//std::vector<SSpriteAnimatorGraphComponent> spriteAnimatorGraphComponents;
		//DeserializeData(spriteAnimatorGraphComponents, fromData, static_cast<U32>(numberOfSpriteAnimatorGraphComponents), pointerPosition);
		//for (const auto& component : spriteAnimatorGraphComponents)
		//	AddComponent<SSpriteAnimatorGraphComponent>(component.EntityOwner);

		U64 numberOfSequencerComponents = 0;
		DeserializeData(numberOfSequencerComponents, fromData, pointerPosition);
		//std::vector<SSequencerComponent> sequencerComponents;
		//DeserializeData(sequencerComponents, fromData, static_cast<U32>(numberOfSequencerComponents), pointerPosition);
		//for (const auto& component : sequencerComponents)
		//	AddComponent<SSequencerComponent>(component.EntityOwner);

		U64 numberOfMetaDataComponents = 0;
		DeserializeData(numberOfMetaDataComponents, fromData, pointerPosition);
		std::vector<SMetaDataComponent> metaDataComponents;
		DeserializeData(metaDataComponents, fromData, static_cast<U32>(numberOfMetaDataComponents), pointerPosition);
		for (const auto& component : metaDataComponents)
			AddComponent<SMetaDataComponent>(component.EntityOwner, component.Name.AsString());
	}

	std::string CScene::GetSceneName() const
	{
		return SceneName.AsString();
	}

	const SEntity& CScene::AddEntity(U64 guid)
	{
		if (EntityIndices.contains(guid))
		{
			HV_LOG_WARN("__FUNC__: Tried to add entity with GUID: %i that already exists. Returning existing Entity.", guid);
			return Entities[EntityIndices.at(guid)];
		}

		SEntity newEntity = { guid };
		if (!newEntity.IsValid())
			newEntity.GUID = UGUIDManager::Generate();

		EntityIndices.emplace(newEntity.GUID, Entities.size());
		Entities.push_back(newEntity);

		return Entities.back();
	}

	const SEntity& CScene::AddEntity(const std::string& nameInEditor, U64 guid)
	{
		const SEntity& outEntity = AddEntity(guid);
		AddComponent<SMetaDataComponent>(outEntity, nameInEditor);

		return outEntity;
	}

	void CScene::RemoveEntity(SEntity& entity)
	{
		if (!entity.IsValid())
		{
			HV_LOG_ERROR("__FUNCTION__: Tried to remove an invalid Entity.");
			return;
		}

		if (!EntityIndices.contains(entity.GUID))
		{
			HV_LOG_ERROR("__FUNCTION__: Tried to remove entity with GUID: %i from a scene that does not contain it.", entity.GUID);
			return;
		}

		for (SComponentStorage& storage : Storages)
		{
			if (storage.EntityIndices.contains(entity.GUID))
			{
				auto& endValue = *(--storage.EntityIndices.end());
				storage.EntityIndices.at(endValue.first) = storage.EntityIndices.at(entity.GUID);

				std::swap(storage.Components[storage.EntityIndices.at(entity.GUID)], storage.Components.back());

				SComponent*& componentToBeRemoved = storage.Components.back();
				delete componentToBeRemoved;
				componentToBeRemoved = nullptr;

				storage.Components.pop_back();
				storage.EntityIndices.erase(entity.GUID);
			}
		}

		auto& endValue = *(--EntityIndices.end());
		EntityIndices.at(endValue.first) = EntityIndices.at(entity.GUID);

		std::swap(Entities[EntityIndices.at(entity.GUID)], Entities.back());

		Entities.pop_back();
		EntityIndices.erase(entity.GUID);
	}

	void CScene::AddView(const SEntity& entityOwner, SViewFunctionPointer function)
	{
		if (!ComponentViews.contains(entityOwner.GUID))
			ComponentViews.emplace(entityOwner.GUID, std::vector<SViewFunctionPointer>());
		 
		auto& functionPointers = ComponentViews.at(entityOwner.GUID);
		//if (std::find(functionPointers.begin(), functionPointers.end(), function) == functionPointers.end())
			functionPointers.push_back(function);
	}

	void CScene::RemoveView(const SEntity& entityOwner, SViewFunctionPointer function)
	{
		if (!ComponentViews.contains(entityOwner.GUID))
			return;

		//auto& functionPointers = ComponentViews.at(entityOwner.GUID);
		//auto it = std::find(functionPointers.begin(), functionPointers.end(), function);
		//if (it != functionPointers.end())
		//	functionPointers.erase(it);
	}

	void CScene::RemoveViews(const SEntity& entityOwner)
	{
		//if (!ComponentViews.contains(entityOwner.GUID))
		//	return;

		//for (auto& [typeHashID, componentView] : ComponentViews.at(entityOwner.GUID))
		//{
		//	delete componentView;
		//	componentView = nullptr;
		//}

		//ComponentViews.erase(entityOwner.GUID);

		if (!ComponentViews.contains(entityOwner.GUID))
			return;

		ComponentViews.at(entityOwner.GUID).clear();
		ComponentViews.erase(entityOwner.GUID);
	}

	//std::vector<SComponentView*> CScene::GetViews(const SEntity& entityOwner)
	std::vector<CScene::SViewFunctionPointer> CScene::GetViews(const SEntity& entityOwner)
	{
		if (!ComponentViews.contains(entityOwner.GUID))
			return {};

		//std::vector<SComponentView*> viewsToReturn = {};
		//for (auto& [typeHashID, componentView] : ComponentViews.at(entityOwner.GUID))
		//	viewsToReturn.push_back(componentView);

		//return viewsToReturn;

		return ComponentViews.at(entityOwner.GUID);
	}

	U64 CScene::GetSceneIndex(const SEntity& entity) const
	{
		return GetSceneIndex(entity.GUID);
	}

	U64 CScene::GetSceneIndex(const U64 entityGUID) const
	{
		if (!EntityIndices.contains(entityGUID))
		{
			HV_LOG_ERROR("%s could not resolve entity %u's GUID to a scene index! Returning 0.", __FUNCTION__, entityGUID);
			return 0;
		}

		return EntityIndices.at(entityGUID);
	}
}
