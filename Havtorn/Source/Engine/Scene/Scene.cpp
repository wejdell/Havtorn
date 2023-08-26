// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Scene.h"

#include "ECS/ECSInclude.h"
#include "ECS/GUIDManager.h"
#include "Graphics/RenderManager.h"
#include "FileSystem/FileHeaderDeclarations.h"
#include "World.h"
#include "AssetRegistry.h"

namespace Havtorn
{
	CScene::CScene()
		: FirstUnusedEntityIndex(0)
	{
		Entities.resize(ENTITY_LIMIT);
		TransformComponents.resize(ENTITY_LIMIT);
		StaticMeshComponents.resize(ENTITY_LIMIT);
		CameraComponents.resize(ENTITY_LIMIT);
		CameraControllerComponents.resize(ENTITY_LIMIT);
		MaterialComponents.resize(ENTITY_LIMIT);
		EnvironmentLightComponents.resize(ENTITY_LIMIT);
		DirectionalLightComponents.resize(ENTITY_LIMIT);
		PointLightComponents.resize(ENTITY_LIMIT);
		SpotLightComponents.resize(ENTITY_LIMIT);
		VolumetricLightComponents.resize(ENTITY_LIMIT);
		DecalComponents.resize(ENTITY_LIMIT);
		SpriteComponents.resize(ENTITY_LIMIT);
		Transform2DComponents.resize(ENTITY_LIMIT);
		SpriteAnimatorGraphComponents.resize(ENTITY_LIMIT);
		SequencerComponents.resize(ENTITY_LIMIT);
		GhostyComponents.resize(ENTITY_LIMIT);
		DebugShapeComponents.resize(ENTITY_LIMIT);
		MetaDataComponents.resize(ENTITY_LIMIT);
	}

	CScene::~CScene()
	{
		EntityVectorIndices.clear();
		Entities.clear();
		TransformComponents.clear();
		StaticMeshComponents.clear();
		CameraComponents.clear();
		CameraControllerComponents.clear();
		MaterialComponents.clear();
		EnvironmentLightComponents.clear();
		DirectionalLightComponents.clear();
		PointLightComponents.clear();
		SpotLightComponents.clear();
		VolumetricLightComponents.clear();
		DecalComponents.clear();
		SpriteComponents.clear();
		Transform2DComponents.clear();
		SpriteAnimatorGraphComponents.clear();
		SequencerComponents.clear();
		GhostyComponents.clear();
		DebugShapeComponents.clear();
		MetaDataComponents.clear();
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
		SEntity* cameraEntity = GetNewEntity("Camera");
		if (!cameraEntity)
			return false;

		MainCameraIndex = GetSceneIndex(*cameraEntity);
		CAssetRegistry* assetRegistry = GEngine::GetWorld()->GetAssetRegistry();

		// Setup entities (create components)
		STransformComponent& transform = AddTransformComponentToEntity(*cameraEntity);
		transform.Transform.Translate({ 2.0f, 1.0f, -3.0f });
		transform.Transform.Rotate({ 0.0f, UMath::DegToRad(35.0f), 0.0f });
		transform.Transform.Translate(SVector::Right * 0.25f);

		SCameraComponent& camera = AddCameraComponentToEntity(*cameraEntity);
		camera.ProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), (16.0f / 9.0f), 0.1f, 1000.0f);
		camera.ViewMatrix = SMatrix::LookAtLH(SVector::Zero, SVector::Forward, SVector::Up);

		SCameraControllerComponent& controllerComp = AddCameraControllerComponentToEntity(*cameraEntity);
		controllerComp.CurrentYaw = UMath::DegToRad(-35.0f);
		// === !Camera ===

		// === Environment light ===
		SEntity* environmentLightEntity = GetNewEntity("Environment Light");
		if (!environmentLightEntity)
			return false;

		AddTransformComponentToEntity(*environmentLightEntity);
		renderManager->LoadEnvironmentLightComponent("Assets/Textures/Cubemaps/CubemapTheVisit.hva", &AddEnvironmentLightComponentToEntity(*environmentLightEntity));
		U16 environmentLightEntitySceneIndex = static_cast<U16>(GetSceneIndex(*environmentLightEntity));
		assetRegistry->Register("Assets/Textures/Cubemaps/CubemapTheVisit.hva", SAssetReferenceCounter(EComponentType::EnvironmentLightComponent, environmentLightEntitySceneIndex, 0, 0));
		// === !Environment light ===

		// === Directional light ===
		SEntity* directionalLightEntity = GetNewEntity("Directional Light");
		if (!directionalLightEntity)
			return false;

		AddDirectionalLightComponentToEntity(*directionalLightEntity);
		SDirectionalLightComponent& directionalLight = DirectionalLightComponents[GetSceneIndex(*directionalLightEntity)];
		directionalLight.Direction = { 1.0f, 1.0f, -1.0f, 0.0f };
		directionalLight.Color = { 212.0f / 255.0f, 175.0f / 255.0f, 55.0f / 255.0f, 0.25f };
		directionalLight.ShadowmapView.ShadowmapViewportIndex = 0;
		directionalLight.ShadowmapView.ShadowProjectionMatrix = SMatrix::OrthographicLH(directionalLight.ShadowViewSize.X, directionalLight.ShadowViewSize.Y, directionalLight.ShadowNearAndFarPlane.X, directionalLight.ShadowNearAndFarPlane.Y);

		SVolumetricLightComponent& volumetricLight = AddVolumetricLightComponentToEntity(*directionalLightEntity);
		volumetricLight.IsActive = false;
		// === !Directional light ===

		// === Point light ===
		SEntity* pointLightEntity = GetNewEntity("Point Light");
		if (!pointLightEntity)
			return true; // From this point it's ok if we fail to load the rest of the demo scene

		STransformComponent& pointLightTransform = AddTransformComponentToEntity(*pointLightEntity);
		SMatrix pointLightMatrix = pointLightTransform.Transform.GetMatrix();
		pointLightMatrix.SetTranslation({ 1.25f, 0.35f, -1.65f });
		pointLightTransform.Transform.SetMatrix(pointLightMatrix);


		SPointLightComponent& pointLightComp = AddPointLightComponentToEntity(*pointLightEntity);
		pointLightComp.ColorAndIntensity = { 0.0f, 1.0f, 1.0f, 10.0f };
		pointLightComp.Range = 1.0f;

		SVolumetricLightComponent& volumetricPointLight = AddVolumetricLightComponentToEntity(*pointLightEntity);
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
		SEntity* spotlight = GetNewEntity("Spot Light");
		if (!spotlight)
			return true;

		STransform& spotlightTransform = AddTransformComponentToEntity(*spotlight).Transform;
		SMatrix spotlightMatrix = spotlightTransform.GetMatrix();
		spotlightMatrix.SetTranslation({ 1.5f, 0.5f, -1.0f });
		spotlightTransform.SetMatrix(spotlightMatrix);

		SSpotLightComponent& spotlightComp = AddSpotLightComponentToEntity(*spotlight);
		spotlightComp.Direction = SVector4::Forward;
		spotlightComp.DirectionNormal1 = SVector4::Right;
		spotlightComp.DirectionNormal2 = SVector4::Up;
		spotlightComp.ColorAndIntensity = { 0.0f, 1.0f, 0.0f, 5.0f };
		spotlightComp.OuterAngle = 25.0f;
		spotlightComp.InnerAngle = 5.0f;
		spotlightComp.Range = 3.0f;

		SVolumetricLightComponent& volumetricSpotLight = AddVolumetricLightComponentToEntity(*spotlight);
		volumetricSpotLight.IsActive = false;

		const SMatrix spotlightProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(90.0f), 1.0f, 0.001f, spotlightComp.Range);
		const SVector4 spotlightPosition = spotlightTransform.GetMatrix().GetTranslation4();

		spotlightComp.ShadowmapView.ShadowPosition = spotlightPosition;
		spotlightComp.ShadowmapView.ShadowmapViewportIndex = 7;
		spotlightComp.ShadowmapView.ShadowViewMatrix = SMatrix::LookAtLH(spotlightPosition.ToVector3(), (spotlightPosition + spotlightComp.Direction).ToVector3(), spotlightComp.DirectionNormal2.ToVector3());
		spotlightComp.ShadowmapView.ShadowProjectionMatrix = spotlightProjection;
		// === !Spotlight ===

		// === Decal ===
		SEntity* decal = GetNewEntity("Decal");
		if (!decal)
			return true;

		STransform& decalTransform = AddTransformComponentToEntity(*decal).Transform;
		decalTransform.Translate({ 0.45f, 1.60f, 0.85f });

		SDecalComponent& decalComp = AddDecalComponentToEntity(*decal);

		std::vector<std::string> decalTextures = { "Assets/Textures/T_noscare_AL_c.hva", "Assets/Textures/T_noscare_AL_m.hva", "Assets/Textures/T_noscare_AL_n.hva" };
		renderManager->LoadDecalComponent(decalTextures, &decalComp);
		decalComp.ShouldRenderAlbedo = true;
		decalComp.ShouldRenderMaterial = true;
		decalComp.ShouldRenderNormal = true;

		U16 decalEntitySceneIndex = static_cast<U16>(GetSceneIndex(*decal));
		assetRegistry->Register(decalTextures, SAssetReferenceCounter(EComponentType::DecalComponent, decalEntitySceneIndex, 0, 0));
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

		SEntity* pendulum = GetNewEntity("Clock");
		if (!pendulum)
			return false;

		STransform& transform1 = AddTransformComponentToEntity(*pendulum).Transform;
		transform1.Translate({ 1.75f, 0.0f, 0.25f });

		renderManager->LoadStaticMeshComponent(modelPath1, &AddStaticMeshComponentToEntity(*pendulum));
		renderManager->LoadMaterialComponent(materialNames1, &AddMaterialComponentToEntity(*pendulum));

		U16 pendulumIndex = static_cast<U16>(GetSceneIndex(*pendulum));
		assetRegistry->Register(modelPath1, SAssetReferenceCounter(EComponentType::StaticMeshComponent, pendulumIndex, 0, 0));
		assetRegistry->Register(materialNames1, SAssetReferenceCounter(EComponentType::MaterialComponent, pendulumIndex, 0, 0));
		// === !Pendulum ===

		// === Bed ===
		SEntity* bed = GetNewEntity("Bed");
		if (!bed)
			return false;

		STransform& transform2 = AddTransformComponentToEntity(*bed).Transform;
		transform2.Translate({ 0.25f, 0.0f, 0.25f });

		renderManager->LoadStaticMeshComponent(modelPath2, &AddStaticMeshComponentToEntity(*bed));
		renderManager->LoadMaterialComponent(materialNames2, &AddMaterialComponentToEntity(*bed));

		U16 bedIndex = static_cast<U16>(GetSceneIndex(*bed));
		assetRegistry->Register(modelPath2, SAssetReferenceCounter(EComponentType::StaticMeshComponent, bedIndex, 0, 0));
		assetRegistry->Register(materialNames2, SAssetReferenceCounter(EComponentType::MaterialComponent, bedIndex, 0, 0));
		// === !Bed ===

		// === Lamp ===
		SEntity* lamp = GetNewEntity("Lamp");
		if (!lamp)
			return false;

		STransform& lampTransform = AddTransformComponentToEntity(*lamp).Transform;
		lampTransform.Translate({ -1.0f, 1.4f, -0.75f });
		lampTransform.Rotate({ 0.0f, UMath::DegToRad(90.0f), 0.0f });

		renderManager->LoadStaticMeshComponent(modelPath4, &AddStaticMeshComponentToEntity(*lamp));
		renderManager->LoadMaterialComponent(materialNames4, &AddMaterialComponentToEntity(*lamp));

		U16 lampIndex = static_cast<U16>(GetSceneIndex(*lamp));
		assetRegistry->Register(modelPath4, SAssetReferenceCounter(EComponentType::StaticMeshComponent, lampIndex, 0, 0));
		assetRegistry->Register(materialNames4, SAssetReferenceCounter(EComponentType::MaterialComponent, lampIndex, 0, 0));
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
			SEntity* floor = GetNewEntity("Floor");
			if (!floor)
				return false;

			STransform& transform3 = AddTransformComponentToEntity(*floor).Transform;
			SMatrix matrix3 = transform3.GetMatrix();
			matrix3.SetTranslation(translations[i]);
			matrix3.SetRotation(SMatrix::CreateRotationAroundZ(UMath::DegToRad(-90.0f)));
			transform3.SetMatrix(matrix3);

			renderManager->LoadStaticMeshComponent(modelPath3, &AddStaticMeshComponentToEntity(*floor));
			renderManager->LoadMaterialComponent(materialNames3, &AddMaterialComponentToEntity(*floor));

			U16 floorSceneIndex = static_cast<U16>(GetSceneIndex(*floor));
			assetRegistry->Register(modelPath3, SAssetReferenceCounter(EComponentType::StaticMeshComponent, floorSceneIndex, 0, 0));
			assetRegistry->Register(materialNames3, SAssetReferenceCounter(EComponentType::MaterialComponent, floorSceneIndex, 0, 0));
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
			SEntity* floor = GetNewEntity("Wall");
			if (!floor)
				return false;

			STransform& transform3 = AddTransformComponentToEntity(*floor).Transform;
			SMatrix matrix3 = transform3.GetMatrix();
			matrix3.SetTranslation(translations[i]);
			matrix3.SetRotation(SMatrix::CreateRotationAroundZ(UMath::DegToRad(-90.0f)) * SMatrix::CreateRotationAroundX(UMath::DegToRad(-90.0f)));
			transform3.SetMatrix(matrix3);

			renderManager->LoadStaticMeshComponent(modelPath3, &AddStaticMeshComponentToEntity(*floor));
			renderManager->LoadMaterialComponent(materialNames3, &AddMaterialComponentToEntity(*floor));

			U16 floorSceneIndex = static_cast<U16>(GetSceneIndex(*floor));
			assetRegistry->Register(modelPath3, SAssetReferenceCounter(EComponentType::StaticMeshComponent, floorSceneIndex, 0, 0));
			assetRegistry->Register(materialNames3, SAssetReferenceCounter(EComponentType::MaterialComponent, floorSceneIndex, 0, 0));
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
			SEntity* floor = GetNewEntity("Wall");
			if (!floor)
				return false;

			STransform& transform3 = AddTransformComponentToEntity(*floor).Transform;
			SMatrix matrix3 = transform3.GetMatrix();
			matrix3.SetTranslation(translations[i]);
			matrix3.SetRotation(SMatrix::CreateRotationAroundZ(UMath::DegToRad(-90.0f)) * SMatrix::CreateRotationAroundX(UMath::DegToRad(-90.0f)) * SMatrix::CreateRotationAroundY(UMath::DegToRad(-90.0f)));
			transform3.SetMatrix(matrix3);

			renderManager->LoadStaticMeshComponent(modelPath3, &AddStaticMeshComponentToEntity(*floor));
			renderManager->LoadMaterialComponent(materialNames3, &AddMaterialComponentToEntity(*floor));

			U16 floorSceneIndex = static_cast<U16>(GetSceneIndex(*floor));
			assetRegistry->Register(modelPath3, SAssetReferenceCounter(EComponentType::StaticMeshComponent, floorSceneIndex, 0, 0));
			assetRegistry->Register(materialNames3, SAssetReferenceCounter(EComponentType::MaterialComponent, floorSceneIndex, 0, 0));
		}
		// === !Other Wall ===

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

	bool CScene::Init2DDemoScene(CRenderManager* renderManager)
	{
		SceneName = std::string("2DDemoScene");

		// === Camera ===
		SEntity* cameraEntity = GetNewEntity("Camera");
		if (!cameraEntity)
			return false;

		MainCameraIndex = GetSceneIndex(*cameraEntity);
		CAssetRegistry* assetRegistry = GEngine::GetWorld()->GetAssetRegistry();

		// Setup entities (create components)
		STransformComponent& transform = AddTransformComponentToEntity(*cameraEntity);
		transform.Transform.Translate({ 0.0f, 1.0f, -5.0f });
		//transform.Transform.Rotate({ 0.0f, UMath::DegToRad(35.0f), 0.0f });
		transform.Transform.Translate(SVector::Right * 0.25f);

		SCameraComponent& camera = AddCameraComponentToEntity(*cameraEntity);
		camera.ProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), (16.0f / 9.0f), 0.1f, 1000.0f);
		//camera.ProjectionType = ECameraProjectionType::Orthographic;
		//camera.ProjectionMatrix = SMatrix::OrthographicLH(5.0f, 5.0f, 0.1f, 1000.0f);
		camera.ViewMatrix = SMatrix::LookAtLH(SVector::Zero, SVector::Forward, SVector::Up);

		//		SCameraControllerComponent& controllerComp = 
		AddCameraControllerComponentToEntity(*cameraEntity);
		//controllerComp.CurrentYaw = UMath::DegToRad(-35.0f);
		// === !Camera ===

		// === Environment light ===
		SEntity* environmentLightEntity = GetNewEntity("Environment Light");
		if (!environmentLightEntity)
			return false;

		AddTransformComponentToEntity(*environmentLightEntity);
		renderManager->LoadEnvironmentLightComponent("Assets/Textures/Cubemaps/CubemapTheVisit.hva", &AddEnvironmentLightComponentToEntity(*environmentLightEntity));
		U16 environmentLightEntitySceneIndex = static_cast<U16>(GetSceneIndex(*environmentLightEntity));
		assetRegistry->Register("Assets/Textures/Cubemaps/CubemapTheVisit.hva", SAssetReferenceCounter(EComponentType::EnvironmentLightComponent, environmentLightEntitySceneIndex, 0, 0));
		// === !Environment light ===

		// === Directional light ===
		SEntity* directionalLightEntity = GetNewEntity("Directional Light");
		if (!directionalLightEntity)
			return false;

		AddDirectionalLightComponentToEntity(*directionalLightEntity);
		SDirectionalLightComponent& directionalLight = DirectionalLightComponents[GetSceneIndex(*directionalLightEntity)];
		directionalLight.Direction = { 1.0f, 1.0f, -1.0f, 0.0f };
		directionalLight.Color = { 212.0f / 255.0f, 175.0f / 255.0f, 55.0f / 255.0f, 0.25f };
		directionalLight.ShadowmapView.ShadowmapViewportIndex = 0;
		directionalLight.ShadowmapView.ShadowProjectionMatrix = SMatrix::OrthographicLH(directionalLight.ShadowViewSize.X, directionalLight.ShadowViewSize.Y, directionalLight.ShadowNearAndFarPlane.X, directionalLight.ShadowNearAndFarPlane.Y);

		SVolumetricLightComponent& volumetricLight = AddVolumetricLightComponentToEntity(*directionalLightEntity);
		volumetricLight.IsActive = false;
		// === !Directional light ===

		// === Spotlight ===
		SEntity* spotlight = GetNewEntity("Spot Light");
		if (!spotlight)
			return true;

		STransform& spotlightTransform = AddTransformComponentToEntity(*spotlight).Transform;
		SMatrix spotlightMatrix = spotlightTransform.GetMatrix();
		spotlightMatrix.SetTranslation({ 0.0f, 0.0f, 0.0f });
		spotlightTransform.SetMatrix(spotlightMatrix);

		SSpotLightComponent& spotlightComp = AddSpotLightComponentToEntity(*spotlight);
		spotlightComp.Direction = SVector4::Forward;
		spotlightComp.DirectionNormal1 = SVector4::Right;
		spotlightComp.DirectionNormal2 = SVector4::Up;
		spotlightComp.ColorAndIntensity = { 0.0f, 1.0f, 0.0f, 5.0f };
		spotlightComp.OuterAngle = 40.0f;
		spotlightComp.InnerAngle = 30.0f;
		spotlightComp.Range = 3.0f;

		SVolumetricLightComponent& volumetricSpotLight = AddVolumetricLightComponentToEntity(*spotlight);
		volumetricSpotLight.IsActive = false;

		const SMatrix spotlightProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(90.0f), 1.0f, 0.001f, spotlightComp.Range);
		const SVector4 spotlightPosition = spotlightTransform.GetMatrix().GetTranslation4();

		spotlightComp.ShadowmapView.ShadowPosition = spotlightPosition;
		spotlightComp.ShadowmapView.ShadowmapViewportIndex = 7;
		spotlightComp.ShadowmapView.ShadowViewMatrix = SMatrix::LookAtLH(spotlightPosition.ToVector3(), (spotlightPosition + spotlightComp.Direction).ToVector3(), spotlightComp.DirectionNormal2.ToVector3());
		spotlightComp.ShadowmapView.ShadowProjectionMatrix = spotlightProjection;
		// === !Spotlight ===

		{
			SEntity* ghosty = GetNewEntity("Ghosty");
			if (!ghosty)
				return true;
			STransformComponent& spriteWStransform = AddTransformComponentToEntity(*ghosty);
			SSpriteComponent& spriteWSComp = AddSpriteComponentToEntity(*ghosty);
			AddGhostyComponentToEntity(*ghosty);

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
			SSpriteAnimatorGraphComponent& spriteAnimatorGraphComponent = AddSpriteAnimatorGraphComponentToEntity(*ghosty);

			SSpriteAnimatorGraphNode& rootNode = spriteAnimatorGraphComponent.SetRoot(std::string("Idle | Locomotion"), ghostySystem->EvaluateIdleFunc);
			rootNode.AddClipNode(&spriteAnimatorGraphComponent, std::string("Idle"), idle);

			SSpriteAnimatorGraphNode& locomotionNode = rootNode.AddSwitchNode(std::string("Locomotion: Left | Right"), ghostySystem->EvaluateLocomotionFunc);
			locomotionNode.AddClipNode(&spriteAnimatorGraphComponent, std::string("Move Left"), moveLeft);
			locomotionNode.AddClipNode(&spriteAnimatorGraphComponent, std::string("Move Right"), moveRight);

			U16 spriteIndex = static_cast<U16>(GetSceneIndex(*ghosty));
			assetRegistry->Register(spritePath, SAssetReferenceCounter(EComponentType::SpriteComponent, spriteIndex, 0, 0));

			SSequencerComponent& sequencerComponent = AddSequencerComponentToEntity(*ghosty);
			sequencerComponent.ComponentTracks.push_back({ EComponentType::TransformComponent });
			sequencerComponent.ComponentTracks.push_back({ EComponentType::SpriteComponent });
		}

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

		for (auto& entity : Entities)
		{
			size += sizeof(U64);

			size += sizeof(I64) * static_cast<size_t>(EComponentType::Count);

			if (entity.HasComponent(EComponentType::TransformComponent))
				size += sizeof(STransformComponent);

			if (entity.HasComponent(EComponentType::StaticMeshComponent))
				size += StaticMeshComponents[GetSceneIndex(entity)].GetSize();

			if (entity.HasComponent(EComponentType::CameraComponent))
				size += sizeof(SCameraComponent);

			if (entity.HasComponent(EComponentType::CameraControllerComponent))
				size += sizeof(SCameraControllerComponent);

			if (entity.HasComponent(EComponentType::MaterialComponent))
				size += MaterialComponents[GetSceneIndex(entity)].GetSize();

			if (entity.HasComponent(EComponentType::EnvironmentLightComponent))
				size += sizeof(SEnvironmentLightComponent);

			if (entity.HasComponent(EComponentType::DirectionalLightComponent))
				size += sizeof(SDirectionalLightComponent);

			if (entity.HasComponent(EComponentType::PointLightComponent))
				size += sizeof(SPointLightComponent);

			if (entity.HasComponent(EComponentType::SpotLightComponent))
				size += sizeof(SSpotLightComponent);

			if (entity.HasComponent(EComponentType::VolumetricLightComponent))
				size += sizeof(SVolumetricLightComponent);

			if (entity.HasComponent(EComponentType::DecalComponent))
				size += DecalComponents[GetSceneIndex(entity)].GetSize();

			if (entity.HasComponent(EComponentType::SpriteComponent))
				size += sizeof(SSpriteComponent);

			if (entity.HasComponent(EComponentType::Transform2DComponent))
				size += sizeof(STransform2DComponent);

			// TODO.AS: Implement GetSize (since the component is not trivially serializable)
			if (entity.HasComponent(EComponentType::SpriteAnimatorGraphComponent))
				size += sizeof(STransform2DComponent);

			// TODO.NR: Implement GetSize (since the component is not trivially serializable)
			if (entity.HasComponent(EComponentType::SequencerComponent))
				size += sizeof(STransform2DComponent);

			if (entity.HasComponent(EComponentType::GhostyComponent))
				size += sizeof(STransform2DComponent);

			if (entity.HasComponent(EComponentType::MetaDataComponent))
				size += sizeof(STransform2DComponent);
		}

		return size;
	}

	void CScene::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeSimple(static_cast<U32>(GetNumberOfValidEntities()), toData, pointerPosition);

		for (auto& entity : Entities)
		{
			if (!entity.IsValid())
				continue;

			SerializeSimple(entity.GUID, toData, pointerPosition);

			SerializeSimple(entity.GetComponentMask(), toData, pointerPosition);
			U64 entitySceneIndex = GetSceneIndex(entity);

			if (entity.HasComponent(EComponentType::TransformComponent))
			{
				SerializeSimple(TransformComponents[entitySceneIndex], toData, pointerPosition);
			}

			if (entity.HasComponent(EComponentType::StaticMeshComponent))
			{
				// Saved and Loaded using AssetRegistry
			}

			if (entity.HasComponent(EComponentType::CameraComponent))
			{
				SerializeSimple(CameraComponents[entitySceneIndex], toData, pointerPosition);
			}

			if (entity.HasComponent(EComponentType::CameraControllerComponent))
			{
				SerializeSimple(CameraControllerComponents[entitySceneIndex], toData, pointerPosition);
			}

			if (entity.HasComponent(EComponentType::MaterialComponent))
			{
				// Saved and Loaded using AssetRegistry
			}

			if (entity.HasComponent(EComponentType::EnvironmentLightComponent))
			{
				// Saved and Loaded using AssetRegistry
			}

			if (entity.HasComponent(EComponentType::DirectionalLightComponent))
			{
				SerializeSimple(DirectionalLightComponents[entitySceneIndex], toData, pointerPosition);
			}

			if (entity.HasComponent(EComponentType::PointLightComponent))
			{
				SerializeSimple(PointLightComponents[entitySceneIndex], toData, pointerPosition);
			}

			if (entity.HasComponent(EComponentType::SpotLightComponent))
			{
				SerializeSimple(SpotLightComponents[entitySceneIndex], toData, pointerPosition);
			}

			if (entity.HasComponent(EComponentType::VolumetricLightComponent))
			{
				SerializeSimple(VolumetricLightComponents[entitySceneIndex], toData, pointerPosition);
			}

			if (entity.HasComponent(EComponentType::DecalComponent))
			{
				// NR: Texture info Saved and Loaded using AssetRegistry
				DecalComponents[GetSceneIndex(entity)].Serialize(toData, pointerPosition);
			}

			if (entity.HasComponent(EComponentType::SpriteComponent))
			{
				// NR: Texture info Saved and Loaded using AssetRegistry
				SerializeSimple(SpriteComponents[entitySceneIndex], toData, pointerPosition);
			}

			if (entity.HasComponent(EComponentType::Transform2DComponent))
			{
				SerializeSimple(Transform2DComponents[entitySceneIndex], toData, pointerPosition);
			}

			if (entity.HasComponent(EComponentType::SpriteAnimatorGraphComponent))
			{
				// TODO.AS: Implement Serialize (since the component is not trivially serializable)
				SerializeSimple(SpriteAnimatorGraphComponents[entitySceneIndex], toData, pointerPosition);
			}

			if (entity.HasComponent(EComponentType::SequencerComponent))
			{
				// TODO.NR: Implement Serialize (since the component is not trivially serializable)
				SerializeSimple(SequencerComponents[entitySceneIndex], toData, pointerPosition);
			}

			if (entity.HasComponent(EComponentType::GhostyComponent))
			{
				SerializeSimple(GhostyComponents[entitySceneIndex], toData, pointerPosition);
			}

			if (entity.HasComponent(EComponentType::MetaDataComponent))
			{
				SerializeSimple(MetaDataComponents[entitySceneIndex], toData, pointerPosition);
			}
		}

		SerializeSimple(&SceneName, toData, pointerPosition);
	}

	void CScene::Deserialize(const char* fromData, U64& pointerPosition, CAssetRegistry* assetRegistry)
	{
		U32 numberOfEntities = 0;
		DeserializeSimple(numberOfEntities, fromData, pointerPosition);

		for (U32 i = 0; i < numberOfEntities; i++)
		{
			U64 guid = 0;
			DeserializeSimple(guid, fromData, pointerPosition);

			SEntity* entity = GetNewEntity(guid);

			CBitSet<STATIC_U64(EComponentType::Count)> componentMask;
			DeserializeSimple(componentMask, fromData, pointerPosition);

			if (componentMask.Test(STATIC_U64(EComponentType::TransformComponent)))
			{
				STransformComponent& transform = AddTransformComponentToEntity(*entity);
				// NR: Write to a copy then assign to avoid breaking vtable ptr
				STransformComponent dataCopy;
				DeserializeSimple(dataCopy, fromData, pointerPosition);
				transform = dataCopy;
			}

			if (componentMask.Test(STATIC_U64(EComponentType::StaticMeshComponent)))
			{
				// TODO.NR/AG: Fix hard coded scene index
				SAssetReferenceCounter counter = { EComponentType::StaticMeshComponent, static_cast<U16>(i), 0, 0 };
				RenderManager->LoadStaticMeshComponent(assetRegistry->GetAssetPath(counter), &AddStaticMeshComponentToEntity(*entity));
			}

			if (componentMask.Test(STATIC_U64(EComponentType::CameraComponent)))
			{
				SCameraComponent& camera = AddCameraComponentToEntity(*entity);
				SCameraComponent dataCopy;
				DeserializeSimple(dataCopy, fromData, pointerPosition);
				camera = dataCopy;
			}

			if (componentMask.Test(STATIC_U64(EComponentType::CameraControllerComponent)))
			{
				SCameraControllerComponent& controller = AddCameraControllerComponentToEntity(*entity);
				SCameraControllerComponent dataCopy;
				DeserializeSimple(dataCopy, fromData, pointerPosition);
				controller = dataCopy;
			}

			if (componentMask.Test(STATIC_U64(EComponentType::MaterialComponent)))
			{
				// TODO.NR/AG: Fix hard coded scene index
				SAssetReferenceCounter counter = { EComponentType::MaterialComponent, static_cast<U16>(i), 0, 0 };
				RenderManager->LoadMaterialComponent(assetRegistry->GetAssetPaths(counter), &AddMaterialComponentToEntity(*entity));
			}

			if (componentMask.Test(STATIC_U64(EComponentType::EnvironmentLightComponent)))
			{
				// TODO.NR/AG: Fix hard coded scene index
				SAssetReferenceCounter counter = { EComponentType::EnvironmentLightComponent, static_cast<U16>(i), 0, 0 };
				RenderManager->LoadEnvironmentLightComponent(assetRegistry->GetAssetPath(counter), &AddEnvironmentLightComponentToEntity(*entity));
			}

			if (componentMask.Test(STATIC_U64(EComponentType::DirectionalLightComponent)))
			{
				SDirectionalLightComponent& directionalLight = AddDirectionalLightComponentToEntity(*entity);
				SDirectionalLightComponent dataCopy;
				DeserializeSimple(dataCopy, fromData, pointerPosition);
				directionalLight = dataCopy;
			}

			if (componentMask.Test(STATIC_U64(EComponentType::PointLightComponent)))
			{
				SPointLightComponent& pointLight = AddPointLightComponentToEntity(*entity);
				SPointLightComponent dataCopy;
				DeserializeSimple(dataCopy, fromData, pointerPosition);
				pointLight = dataCopy;
			}

			if (componentMask.Test(STATIC_U64(EComponentType::SpotLightComponent)))
			{
				SSpotLightComponent& spotLight = AddSpotLightComponentToEntity(*entity);
				SSpotLightComponent dataCopy;
				DeserializeSimple(dataCopy, fromData, pointerPosition);
				spotLight = dataCopy;
			}

			if (componentMask.Test(STATIC_U64(EComponentType::VolumetricLightComponent)))
			{
				SVolumetricLightComponent& volumetric = AddVolumetricLightComponentToEntity(*entity);
				SVolumetricLightComponent dataCopy;
				DeserializeSimple(dataCopy, fromData, pointerPosition);
				volumetric = dataCopy;
			}

			if (componentMask.Test(STATIC_U64(EComponentType::DecalComponent)))
			{
				// TODO.NR/AG: Fix hard coded scene index
				SAssetReferenceCounter counter = { EComponentType::DecalComponent, static_cast<U16>(i), 0, 0 };
				RenderManager->LoadDecalComponent(assetRegistry->GetAssetPaths(counter), &AddDecalComponentToEntity(*entity));

				DecalComponents[GetSceneIndex(*entity)].Deserialize(fromData, pointerPosition);
			}

			if (componentMask.Test(STATIC_U64(EComponentType::SpriteComponent)))
			{
				SSpriteComponent& spriteComponent = AddSpriteComponentToEntity(*entity);
				SSpriteComponent dataCopy;
				DeserializeSimple(dataCopy, fromData, pointerPosition);
				spriteComponent = dataCopy;

				SAssetReferenceCounter counter = { EComponentType::SpriteComponent, static_cast<U16>(i), 0, 0 };
				RenderManager->LoadSpriteComponent(assetRegistry->GetAssetPath(counter), &spriteComponent);
			}

			if (componentMask.Test(STATIC_U64(EComponentType::Transform2DComponent)))
			{
				STransform2DComponent& transform = AddTransform2DComponentToEntity(*entity);
				STransform2DComponent dataCopy;
				DeserializeSimple(dataCopy, fromData, pointerPosition);
				transform = dataCopy;
			}

			if (componentMask.Test(STATIC_U64(EComponentType::SpriteAnimatorGraphComponent)))
			{
				// TODO.AS: Implement Deserialize (since the component is not trivially serializable)
			}

			if (componentMask.Test(STATIC_U64(EComponentType::SequencerComponent)))
			{
				// TODO.NR: Implement Deserialize (since the component is not trivially serializable)
			}

			if (componentMask.Test(STATIC_U64(EComponentType::GhostyComponent)))
			{
				SGhostyComponent& ghostyComponent = AddGhostyComponentToEntity(*entity);
				SGhostyComponent dataCopy;
				DeserializeSimple(dataCopy, fromData, pointerPosition);
				ghostyComponent = dataCopy;
			}

			if (componentMask.Test(STATIC_U64(EComponentType::MetaDataComponent)))
			{
				SMetaDataComponent& metaData = AddMetaDataComponentToEntity(*entity);
				SMetaDataComponent dataCopy;
				DeserializeSimple(dataCopy, fromData, pointerPosition);
				metaData = dataCopy;
			}
		}

		DeserializeSimple(SceneName, fromData, pointerPosition);
	}

	std::string CScene::GetSceneName() const
	{
		return SceneName.AsString();
	}

	std::vector<SEntity>& CScene::GetEntities()
	{
		return Entities;
	}

	SEntity* CScene::GetNewEntity(U64 guid)
	{
		// TODO: Figure out Tombstone solution

		if (FirstUnusedEntityIndex >= ENTITY_LIMIT)
		{
			HV_ASSERT(false, "Reached ENTITY_LIMIT.");
			return nullptr;
		}

		SEntity* outEntity = &Entities[FirstUnusedEntityIndex];
		outEntity->GUID = guid != 0 ? guid : UGUIDManager::Generate();

		EntityVectorIndices.emplace(outEntity->GUID, FirstUnusedEntityIndex);
		FirstUnusedEntityIndex++;

		return outEntity;
	}

	SEntity* CScene::GetNewEntity(const std::string& nameInEditor, U64 guid)
	{
		SEntity* outEntity = GetNewEntity(guid);
		SMetaDataComponent& metaDataComp = AddMetaDataComponentToEntity(*outEntity);
		metaDataComp.Name = nameInEditor;

		return outEntity;
	}

	bool CScene::TryRemoveEntity(SEntity& entity)
	{
		if (!entity.IsValid())
		{
			HV_LOG_ERROR("Tried to remove invalid Entity.");
			return false;
		}

		if (FirstUnusedEntityIndex <= 0)
		{
			HV_LOG_ERROR("Tried to remove an entity from an empty scene.");
			return false;
		}

		if (!EntityVectorIndices.contains(entity.GUID))
		{
			HV_LOG_ERROR("Tried to remove an entity from a scene other than its own.");
			return false;
		}

		U64 entityIndex = EntityVectorIndices[entity.GUID];

		if (entityIndex != (FirstUnusedEntityIndex - 1))
		{
			std::swap(entity, Entities[FirstUnusedEntityIndex - 1]);
		}

		UpdateComponentVector(TransformComponents, entityIndex);
		UpdateComponentVector(StaticMeshComponents, entityIndex);
		UpdateComponentVector(CameraComponents, entityIndex);
		UpdateComponentVector(CameraControllerComponents, entityIndex);
		UpdateComponentVector(MaterialComponents, entityIndex);
		UpdateComponentVector(EnvironmentLightComponents, entityIndex);
		UpdateComponentVector(DirectionalLightComponents, entityIndex);
		UpdateComponentVector(PointLightComponents, entityIndex);
		UpdateComponentVector(SpotLightComponents, entityIndex);
		UpdateComponentVector(VolumetricLightComponents, entityIndex);
		UpdateComponentVector(DecalComponents, entityIndex);
		UpdateComponentVector(SpriteComponents, entityIndex);
		UpdateComponentVector(Transform2DComponents, entityIndex);
		UpdateComponentVector(SpriteAnimatorGraphComponents, entityIndex);
		UpdateComponentVector(SequencerComponents, entityIndex);
		UpdateComponentVector(GhostyComponents, entityIndex);
		UpdateComponentVector(DebugShapeComponents, entityIndex);
		UpdateComponentVector(MetaDataComponents, entityIndex);

		Entities[FirstUnusedEntityIndex - 1].GUID = 0;
		FirstUnusedEntityIndex--;

		return true;
	}

	U64 CScene::GetSceneIndex(const SEntity& entity) const
	{
		return EntityVectorIndices.at(entity.GUID);
	}

	U64 CScene::GetSceneIndex(const U64 entityGUID) const
	{
		return EntityVectorIndices.at(entityGUID);
	}

	U64 CScene::GetMainCameraIndex() const
	{
		return MainCameraIndex;
	}

	U64 CScene::GetNumberOfValidEntities() const
	{
		return FirstUnusedEntityIndex;
	}

	void CScene::AddComponentToEntity(EComponentType componentType, SEntity& entity)
	{
		switch (componentType)
		{
		case Havtorn::EComponentType::TransformComponent:
			AddTransformComponentToEntity(entity);
			break;
		case Havtorn::EComponentType::StaticMeshComponent:
			AddStaticMeshComponentToEntity(entity);
			break;
		case Havtorn::EComponentType::CameraComponent:
			AddCameraComponentToEntity(entity);
			break;
		case Havtorn::EComponentType::CameraControllerComponent:
			AddCameraControllerComponentToEntity(entity);
			break;
		case Havtorn::EComponentType::MaterialComponent:
			AddMaterialComponentToEntity(entity);
			break;
		case Havtorn::EComponentType::EnvironmentLightComponent:
			AddEnvironmentLightComponentToEntity(entity);
			break;
		case Havtorn::EComponentType::DirectionalLightComponent:
			AddDirectionalLightComponentToEntity(entity);
			break;
		case Havtorn::EComponentType::PointLightComponent:
			AddPointLightComponentToEntity(entity);
			break;
		case Havtorn::EComponentType::SpotLightComponent:
			AddSpotLightComponentToEntity(entity);
			break;
		case Havtorn::EComponentType::VolumetricLightComponent:
			AddVolumetricLightComponentToEntity(entity);
			break;
		case Havtorn::EComponentType::DecalComponent:
			AddDecalComponentToEntity(entity);
			break;
		case Havtorn::EComponentType::SpriteComponent:
			AddSpriteComponentToEntity(entity);
			break;
		case Havtorn::EComponentType::Transform2DComponent:
			AddTransform2DComponentToEntity(entity);
			break;
		case Havtorn::EComponentType::SpriteAnimatorGraphComponent:
			AddSpriteAnimatorGraphComponentToEntity(entity);
			break;
		case Havtorn::EComponentType::SequencerComponent:
			AddSequencerComponentToEntity(entity);
			break;

		case Havtorn::EComponentType::DebugShapeComponent:
		case Havtorn::EComponentType::MetaDataComponent:
		case Havtorn::EComponentType::Count:
		default:
			break;
		}
	}

	void CScene::RemoveComponentFromEntity(EComponentType componentType, SEntity& entity)
	{
		switch (componentType)
		{
		case Havtorn::EComponentType::TransformComponent:
			RemoveTransformComponentFromEntity(entity);
			break;
		case Havtorn::EComponentType::StaticMeshComponent:
			RemoveStaticMeshComponentFromEntity(entity);
			break;
		case Havtorn::EComponentType::CameraComponent:
			RemoveCameraComponentFromEntity(entity);
			break;
		case Havtorn::EComponentType::CameraControllerComponent:
			RemoveCameraControllerComponentFromEntity(entity);
			break;
		case Havtorn::EComponentType::MaterialComponent:
			RemoveMaterialComponentFromEntity(entity);
			break;
		case Havtorn::EComponentType::EnvironmentLightComponent:
			RemoveEnvironmentLightComponentFromEntity(entity);
			break;
		case Havtorn::EComponentType::DirectionalLightComponent:
			RemoveDirectionalLightComponentFromEntity(entity);
			break;
		case Havtorn::EComponentType::PointLightComponent:
			RemovePointLightComponentFromEntity(entity);
			break;
		case Havtorn::EComponentType::SpotLightComponent:
			RemoveSpotLightComponentFromEntity(entity);
			break;
		case Havtorn::EComponentType::VolumetricLightComponent:
			RemoveVolumetricLightComponentFromEntity(entity);
			break;
		case Havtorn::EComponentType::DecalComponent:
			RemoveDecalComponentFromEntity(entity);
			break;
		case Havtorn::EComponentType::SpriteComponent:
			RemoveSpriteComponentFromEntity(entity);
			break;
		case Havtorn::EComponentType::Transform2DComponent:
			RemoveTransform2DComponentFromEntity(entity);
			break;
		case Havtorn::EComponentType::SpriteAnimatorGraphComponent:
			RemoveSpriteAnimatorGraphComponentFromEntity(entity);
			break;
		case Havtorn::EComponentType::SequencerComponent:
			RemoveSequencerComponentFromEntity(entity);
			break;

		case Havtorn::EComponentType::DebugShapeComponent:
		case Havtorn::EComponentType::MetaDataComponent:
		case Havtorn::EComponentType::Count:
		default:
			break;
		}
	}

	COMPONENT_ADDER_DEFINITION(TransformComponent)
	COMPONENT_ADDER_DEFINITION(StaticMeshComponent)
	COMPONENT_ADDER_DEFINITION(CameraComponent)
	COMPONENT_ADDER_DEFINITION(CameraControllerComponent)
	COMPONENT_ADDER_DEFINITION(MaterialComponent)
	COMPONENT_ADDER_DEFINITION(EnvironmentLightComponent)
	COMPONENT_ADDER_DEFINITION(DirectionalLightComponent)
	COMPONENT_ADDER_DEFINITION(PointLightComponent)
	COMPONENT_ADDER_DEFINITION(SpotLightComponent)
	COMPONENT_ADDER_DEFINITION(VolumetricLightComponent)
	COMPONENT_ADDER_DEFINITION(DecalComponent)
	COMPONENT_ADDER_DEFINITION(SpriteComponent)
	COMPONENT_ADDER_DEFINITION(Transform2DComponent)
	COMPONENT_ADDER_DEFINITION(SpriteAnimatorGraphComponent)
	COMPONENT_ADDER_DEFINITION(SequencerComponent)
	COMPONENT_ADDER_DEFINITION(GhostyComponent);
	COMPONENT_ADDER_DEFINITION(DebugShapeComponent)
	COMPONENT_ADDER_DEFINITION(MetaDataComponent)

	COMPONENT_REMOVER_DEFINITION(TransformComponent)
	COMPONENT_REMOVER_DEFINITION(StaticMeshComponent)
	COMPONENT_REMOVER_DEFINITION(CameraComponent)
	COMPONENT_REMOVER_DEFINITION(CameraControllerComponent)
	COMPONENT_REMOVER_DEFINITION(MaterialComponent)
	COMPONENT_REMOVER_DEFINITION(EnvironmentLightComponent)
	COMPONENT_REMOVER_DEFINITION(DirectionalLightComponent)
	COMPONENT_REMOVER_DEFINITION(PointLightComponent)
	COMPONENT_REMOVER_DEFINITION(SpotLightComponent)
	COMPONENT_REMOVER_DEFINITION(VolumetricLightComponent)
	COMPONENT_REMOVER_DEFINITION(DecalComponent)
	COMPONENT_REMOVER_DEFINITION(SpriteComponent)
	COMPONENT_REMOVER_DEFINITION(Transform2DComponent)
	COMPONENT_REMOVER_DEFINITION(SpriteAnimatorGraphComponent)
	COMPONENT_REMOVER_DEFINITION(SequencerComponent)
	COMPONENT_REMOVER_DEFINITION(GhostyComponent)
	COMPONENT_REMOVER_DEFINITION(DebugShapeComponent)
	COMPONENT_REMOVER_DEFINITION(MetaDataComponent)
}
