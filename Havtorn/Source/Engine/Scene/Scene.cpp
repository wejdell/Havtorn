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

		RegisterComponent<STransformComponent>(50, &STransformComponentEditorContext::Context);
		RegisterComponent<SStaticMeshComponent>(40, &SStaticMeshComponentEditorContext::Context);
		RegisterComponent<SSkeletalMeshComponent>(40, &SSkeletalMeshComponentEditorContext::Context);
		RegisterComponent<SSkeletalAnimationComponent>(40, &SSkeletalAnimationComponentEditorContext::Context);
		RegisterComponent<SCameraComponent>(2, &SCameraComponentEditorContext::Context);
		RegisterComponent<SCameraControllerComponent>(2, &SCameraControllerComponentEditorContext::Context);
		RegisterComponent<SMaterialComponent>(40, &SMaterialComponentEditorContext::Context);
		RegisterComponent<SEnvironmentLightComponent>(1, &SEnvironmentLightComponentEditorContext::Context);
		RegisterComponent<SDirectionalLightComponent>(1, &SDirectionalLightComponentEditorContext::Context);
		RegisterComponent<SPointLightComponent>(1, &SPointLightComponentEditorContext::Context);
		RegisterComponent<SSpotLightComponent>(1, &SSpotLightComponentEditorContext::Context);
		RegisterComponent<SVolumetricLightComponent>(3, &SVolumetricLightComponentEditorContext::Context);
		RegisterComponent<SDecalComponent>(2, &SDecalComponentEditorContext::Context);
		RegisterComponent<SSpriteComponent>(10, &SSpriteComponentEditorContext::Context);
		RegisterComponent<STransform2DComponent>(10, &STransform2DComponentEditorContext::Context);
		RegisterComponent<SSpriteAnimatorGraphComponent>(2, &SSpriteAnimatorGraphComponentEditorContext::Context);
		RegisterComponent<SSequencerComponent>(0, &SSequencerComponentEditorContext::Context);
		RegisterComponent<SPhysics2DComponent>(10, &SPhysics2DComponentEditorContext::Context);
		RegisterComponent<SPhysics3DComponent>(40, &SPhysics3DComponentEditorContext::Context);
		RegisterComponent<SPhysics3DControllerComponent>(1, &SPhysics3DControllerComponentEditorContext::Context);

		return true;
	}

	bool CScene::Init3DDemoScene(CRenderManager* renderManager)
	{
		if (!Init(renderManager, "3DDemoScene"))		
			return false;

		// === Camera ===
		MainCameraEntity = AddEntity("Camera");
		
		if (!MainCameraEntity.IsValid())
			return false;

		CAssetRegistry* assetRegistry = GEngine::GetWorld()->GetAssetRegistry();

		// Setup entities (create components)
		STransformComponent& transform = *AddComponent<STransformComponent>(MainCameraEntity);
		AddComponentEditorContext(MainCameraEntity, &STransformComponentEditorContext::Context);

		transform.Transform.Translate({ 2.5f, 1.0f, -3.5f });
		transform.Transform.Rotate({ 0.0f, UMath::DegToRad(35.0f), 0.0f });
		transform.Transform.Translate(SVector::Right * 0.25f);

		SCameraComponent& camera = *AddComponent<SCameraComponent>(MainCameraEntity);
		AddComponentEditorContext(MainCameraEntity, &SCameraComponentEditorContext::Context);
		camera.ProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), (16.0f / 9.0f), 0.1f, 1000.0f);

		SCameraControllerComponent& controllerComp = *AddComponent<SCameraControllerComponent>(MainCameraEntity);
		AddComponentEditorContext(MainCameraEntity, &SCameraControllerComponentEditorContext::Context);
		controllerComp.CurrentYaw = -35.0f;
		// === !Camera ===

		// === Environment light ===
		const SEntity& environmentLightEntity = AddEntity("Environment Light");
		if (!environmentLightEntity.IsValid())
			return false;

		AddComponent<STransformComponent>(environmentLightEntity);
		AddComponentEditorContext(environmentLightEntity, &STransformComponentEditorContext::Context);
		renderManager->LoadEnvironmentLightComponent("Assets/Textures/Cubemaps/CubemapTheVisit.hva", AddComponent<SEnvironmentLightComponent>(environmentLightEntity));
		AddComponentEditorContext(environmentLightEntity, &SEnvironmentLightComponentEditorContext::Context);
		GetComponent<SEnvironmentLightComponent>(environmentLightEntity)->AssetRegistryKey = assetRegistry->Register("Assets/Textures/Cubemaps/CubemapTheVisit.hva");
		// === !Environment light ===

		// === Directional light ===
		const SEntity& directionalLightEntity = AddEntity("Directional Light");
		if (!directionalLightEntity.IsValid())
			return false;

		// NR: Add transform to directional light so it can filter environmental lights based on distance
		AddComponent<STransformComponent>(directionalLightEntity);
		AddComponentEditorContext(directionalLightEntity, &STransformComponentEditorContext::Context);

		SDirectionalLightComponent& directionalLight = *AddComponent<SDirectionalLightComponent>(directionalLightEntity);
		AddComponentEditorContext(directionalLightEntity, &SDirectionalLightComponentEditorContext::Context);
		directionalLight.Direction = { 1.0f, 1.0f, -1.0f, 0.0f };
		directionalLight.Color = { 212.0f / 255.0f, 175.0f / 255.0f, 55.0f / 255.0f, 0.25f };
		directionalLight.ShadowmapView.ShadowmapViewportIndex = 0;
		directionalLight.ShadowmapView.ShadowProjectionMatrix = SMatrix::OrthographicLH(directionalLight.ShadowViewSize.X, directionalLight.ShadowViewSize.Y, directionalLight.ShadowNearAndFarPlane.X, directionalLight.ShadowNearAndFarPlane.Y);

		SVolumetricLightComponent& volumetricLight = *AddComponent<SVolumetricLightComponent>(directionalLightEntity);
		AddComponentEditorContext(directionalLightEntity, &SVolumetricLightComponentEditorContext::Context);
		volumetricLight.IsActive = false;
		// === !Directional light ===

		// === Point light ===
		const SEntity& pointLightEntity = AddEntity("Point Light");
		if (!pointLightEntity.IsValid())
			return true; // From this point it's ok if we fail to load the rest of the demo scene

		STransformComponent& pointLightTransform = *AddComponent<STransformComponent>(pointLightEntity);
		AddComponentEditorContext(pointLightEntity, &STransformComponentEditorContext::Context);
		SMatrix pointLightMatrix = pointLightTransform.Transform.GetMatrix();
		pointLightMatrix.SetTranslation({ 1.75f, 0.35f, -2.15f });
		pointLightTransform.Transform.SetMatrix(pointLightMatrix);


		SPointLightComponent& pointLightComp = *AddComponent<SPointLightComponent>(pointLightEntity);
		AddComponentEditorContext(pointLightEntity, &SPointLightComponentEditorContext::Context);
		pointLightComp.ColorAndIntensity = { 0.0f, 1.0f, 1.0f, 10.0f };
		pointLightComp.Range = 1.0f;

		SVolumetricLightComponent& volumetricPointLight = *AddComponent<SVolumetricLightComponent>(pointLightEntity);
		AddComponentEditorContext(pointLightEntity, &SVolumetricLightComponentEditorContext::Context);
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
		AddComponentEditorContext(spotlight, &STransformComponentEditorContext::Context);
		SMatrix spotlightMatrix = spotlightTransform.GetMatrix();
		spotlightMatrix.SetTranslation({ 2.0f, 0.5f, -1.5f });
		spotlightTransform.SetMatrix(spotlightMatrix);

		SSpotLightComponent& spotlightComp = *AddComponent<SSpotLightComponent>(spotlight);
		AddComponentEditorContext(spotlight, &SSpotLightComponentEditorContext::Context);
		spotlightComp.Direction = SVector4::Forward;
		spotlightComp.DirectionNormal1 = SVector4::Right;
		spotlightComp.DirectionNormal2 = SVector4::Up;
		spotlightComp.ColorAndIntensity = { 0.0f, 1.0f, 0.0f, 5.0f };
		spotlightComp.OuterAngle = 25.0f;
		spotlightComp.InnerAngle = 5.0f;
		spotlightComp.Range = 3.0f;

		SVolumetricLightComponent& volumetricSpotLight = *AddComponent<SVolumetricLightComponent>(spotlight);
		AddComponentEditorContext(spotlight, &SVolumetricLightComponentEditorContext::Context);
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
		AddComponentEditorContext(decal, &STransformComponentEditorContext::Context);
		decalTransform.Translate({ 0.75f, 1.60f, 0.35f });

		SDecalComponent& decalComp = *AddComponent<SDecalComponent>(decal);
		AddComponentEditorContext(decal, &SDecalComponentEditorContext::Context);

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
		const std::string modelPath3 = "Assets/Tests/Plane.hva";
		const std::vector<std::string> materialNames3 = { "Assets/Materials/M_Quad.hva" };
		const std::string modelPath4 = "Assets/Tests/En_P_WallLamp.hva";
		const std::vector<std::string> materialNames4 = { "Assets/Materials/M_Quad.hva", "Assets/Materials/M_Emissive.hva", "Assets/Materials/M_Headlamp.hva" };
		const std::string modelPath5 = "Assets/Tests/Cube_1.hva";
		const std::vector<std::string> materialNames5 = { "Assets/Materials/M_Quad.hva" };

		// === Pendulum ===
		const SEntity& pendulum = AddEntity("Clock");
		if (!pendulum.IsValid())
			return false;

		STransform& transform1 = (*AddComponent<STransformComponent>(pendulum)).Transform;
		AddComponentEditorContext(pendulum, &STransformComponentEditorContext::Context);
		transform1.Translate({ 1.8f, 0.0f, -0.2f });

		renderManager->LoadStaticMeshComponent(modelPath1, AddComponent<SStaticMeshComponent>(pendulum));
		AddComponentEditorContext(pendulum, &SStaticMeshComponentEditorContext::Context);
		renderManager->LoadMaterialComponent(materialNames1, AddComponent<SMaterialComponent>(pendulum));
		AddComponentEditorContext(pendulum, &SMaterialComponentEditorContext::Context);

		GetComponent<SStaticMeshComponent>(pendulum)->AssetRegistryKey = assetRegistry->Register(modelPath1);
		GetComponent<SMaterialComponent>(pendulum)->AssetRegistryKeys = assetRegistry->Register(materialNames1);

		SPhysics3DComponent* clockPhysics = AddComponent<SPhysics3DComponent>(pendulum);
		AddComponentEditorContext(pendulum, &SPhysics3DComponentEditorContext::Context);

		clockPhysics->BodyType = EPhysics3DBodyType::Static;
		clockPhysics->ShapeType = EPhysics3DShapeType::Box;
		clockPhysics->ShapeLocalExtents = SVector(0.6f, 1.9f, 0.3f);
		clockPhysics->ShapeLocalOffset = SVector(0.0f, 0.95f, 0.0f);

		GEngine::GetWorld()->Initialize3DPhysicsData(pendulum);
		// === !Pendulum ===

		// === Bed ===
		const SEntity& bed = AddEntity("Bed");
		if (!bed.IsValid())
			return false;

		STransform& transform2 = (*AddComponent<STransformComponent>(bed)).Transform;
		AddComponentEditorContext(bed, &STransformComponentEditorContext::Context);
		transform2.Translate({ 0.2f, 0.0f, 0.0f });

		renderManager->LoadStaticMeshComponent(modelPath2, AddComponent<SStaticMeshComponent>(bed));
		AddComponentEditorContext(bed, &SStaticMeshComponentEditorContext::Context);
		renderManager->LoadMaterialComponent(materialNames2, AddComponent<SMaterialComponent>(bed));
		AddComponentEditorContext(bed, &SMaterialComponentEditorContext::Context);

		GetComponent<SStaticMeshComponent>(bed)->AssetRegistryKey = assetRegistry->Register(modelPath2);
		GetComponent<SMaterialComponent>(bed)->AssetRegistryKeys = assetRegistry->Register(materialNames2);

		SPhysics3DComponent* bedPhysics = AddComponent<SPhysics3DComponent>(bed);
		AddComponentEditorContext(bed, &SPhysics3DComponentEditorContext::Context);

		bedPhysics->BodyType = EPhysics3DBodyType::Static;
		bedPhysics->ShapeType = EPhysics3DShapeType::Box;
		bedPhysics->ShapeLocalExtents = SVector(1.8f, 0.7f, 2.5f);
		bedPhysics->ShapeLocalOffset = SVector(0.0f, bedPhysics->ShapeLocalExtents.Y * 0.5f, -bedPhysics->ShapeLocalExtents.Z * 0.5f);

		GEngine::GetWorld()->Initialize3DPhysicsData(bed);
		// === !Bed ===

		// === Lamp ===
		const SEntity& lamp = AddEntity("Lamp");
		if (!lamp.IsValid())
			return false;

		STransform& lampTransform = (*AddComponent<STransformComponent>(lamp)).Transform;
		AddComponentEditorContext(lamp, &STransformComponentEditorContext::Context);
		lampTransform.Translate({ -1.0f, 1.4f, -1.25f });
		lampTransform.Rotate({ 0.0f, UMath::DegToRad(90.0f), 0.0f });

		renderManager->LoadStaticMeshComponent(modelPath4, AddComponent<SStaticMeshComponent>(lamp));
		AddComponentEditorContext(lamp, &SStaticMeshComponentEditorContext::Context);
		renderManager->LoadMaterialComponent(materialNames4, AddComponent<SMaterialComponent>(lamp));
		AddComponentEditorContext(lamp, &SMaterialComponentEditorContext::Context);

		GetComponent<SStaticMeshComponent>(lamp)->AssetRegistryKey = assetRegistry->Register(modelPath4);
		GetComponent<SMaterialComponent>(lamp)->AssetRegistryKeys = assetRegistry->Register(materialNames4);
		// === !Lamp ===

		// === Player Proxy ===
		const SEntity& playerProxy = AddEntity("Player");
		if (!playerProxy.IsValid())
			return false;

		STransform& playerTransform = AddComponent<STransformComponent>(playerProxy)->Transform;
		AddComponentEditorContext(playerProxy, &STransformComponentEditorContext::Context);
		SMatrix playerMatrix = playerTransform.GetMatrix();
		playerMatrix.SetTranslation({ 2.6f, 0.0f, -0.24f });
		//playerMatrix.SetRotation();
		playerTransform.SetMatrix(playerMatrix);

		SPhysics3DControllerComponent* controllerComponent = AddComponent<SPhysics3DControllerComponent>(playerProxy);
		AddComponentEditorContext(playerProxy, &SPhysics3DControllerComponentEditorContext::Context);

		controllerComponent->ControllerType = EPhysics3DControllerType::Capsule;
		controllerComponent->ShapeLocalRadiusAndHeight = SVector2(0.25f, 1.0f);

		GEngine::GetWorld()->Initialize3DPhysicsData(playerProxy);

		//// Static Mesh
		//std::string staticMeshPath = "Assets/Tests/CH_Enemy.hva";
		//renderManager->LoadStaticMeshComponent(staticMeshPath, AddComponent<SStaticMeshComponent>(playerProxy));
		//AddComponentEditorContext(playerProxy, &SStaticMeshComponentEditorContext::Context);
		//GetComponent<SStaticMeshComponent>(playerProxy)->AssetRegistryKey = assetRegistry->Register(staticMeshPath);

		// Skeletal Mesh
		std::string meshPath = "Assets/Tests/CH_Enemy_SK.hva";
		//std::string meshPath = "Assets/Tests/MaleDefault.hva";
		//std::string meshPath = "Assets/Tests/DebugAnimMesh.hva";
		renderManager->LoadSkeletalMeshComponent(meshPath, AddComponent<SSkeletalMeshComponent>(playerProxy));
		AddComponentEditorContext(playerProxy, &SSkeletalMeshComponentEditorContext::Context);
		GetComponent<SSkeletalMeshComponent>(playerProxy)->AssetRegistryKey = assetRegistry->Register(meshPath);

		std::string animationPath = "Assets/Tests/CH_Enemy_Walk.hva";
		//std::string animationPath = "Assets/Tests/MaleWave.hva";
		//std::string animationPath = "Assets/Tests/TestWalk.hva";
		//std::string animationPath = "Assets/Tests/DebugAnimAnim.hva";
		renderManager->LoadSkeletalAnimationComponent(animationPath, AddComponent<SSkeletalAnimationComponent>(playerProxy));
		AddComponentEditorContext(playerProxy, &SSkeletalAnimationComponentEditorContext::Context);
		// TODO.NR
		//GetComponent<SSkeletalAnimationComponent>(playerProxy)->AssetRegistryKey = assetRegistry->Register("Assets/Tests/CH_Enemy_SK.hva");

		//std::vector<std::string> enemyMaterialPaths = { "Assets/Materials/M_Checkboard_128x128.hva" };
		std::vector<std::string> enemyMaterialPaths = { "Assets/Materials/M_Enemy.hva" };
		renderManager->LoadMaterialComponent(enemyMaterialPaths, AddComponent<SMaterialComponent>(playerProxy));
		AddComponentEditorContext(playerProxy, &SMaterialComponentEditorContext::Context);
		GetComponent<SMaterialComponent>(playerProxy)->AssetRegistryKeys = assetRegistry->Register(enemyMaterialPaths);
		
		// === !Player Proxy ===

		// === Crate ===
		const SEntity& crate = AddEntity("Crate");
		if (!crate.IsValid())
			return false;

		STransform& crateTransform = AddComponent<STransformComponent>(crate)->Transform;
		AddComponentEditorContext(crate, &STransformComponentEditorContext::Context);
		SMatrix crateMatrix = crateTransform.GetMatrix();
		SMatrix::Recompose(SVector(1.0f, 4.7f, -1.5f), SVector(45.0f, 0.0f, 45.0f), SVector(0.5f), crateMatrix);
		crateTransform.SetMatrix(crateMatrix);

		renderManager->LoadStaticMeshComponent(modelPath5, AddComponent<SStaticMeshComponent>(crate));
		AddComponentEditorContext(crate, &SStaticMeshComponentEditorContext::Context);
		renderManager->LoadMaterialComponent(materialNames5, AddComponent<SMaterialComponent>(crate));
		AddComponentEditorContext(crate, &SMaterialComponentEditorContext::Context);

		GetComponent<SStaticMeshComponent>(crate)->AssetRegistryKey = assetRegistry->Register(modelPath5);
		GetComponent<SMaterialComponent>(crate)->AssetRegistryKeys = assetRegistry->Register(materialNames5);

		SPhysics3DComponent* cratePhysics = AddComponent<SPhysics3DComponent>(crate);
		AddComponentEditorContext(crate, &SPhysics3DComponentEditorContext::Context);

		cratePhysics->BodyType = EPhysics3DBodyType::Dynamic;
		cratePhysics->ShapeType = EPhysics3DShapeType::Box;
		cratePhysics->ShapeLocalExtents = SVector(0.5f);

		GEngine::GetWorld()->Initialize3DPhysicsData(crate);
		// === !Crate ===

		// === Trigger ===
		const SEntity& trigger = AddEntity("Trigger");
		if (!trigger.IsValid())
			return false;

		STransform& triggerTransform = AddComponent<STransformComponent>(trigger)->Transform;
		AddComponentEditorContext(trigger, &STransformComponentEditorContext::Context);
		triggerTransform.Translate({ 0.2f, 1.0f, -1.25f });

		SPhysics3DComponent* triggerPhysics = AddComponent<SPhysics3DComponent>(trigger);
		AddComponentEditorContext(trigger, &SPhysics3DComponentEditorContext::Context);

		triggerPhysics->BodyType = EPhysics3DBodyType::Static;
		triggerPhysics->ShapeType = EPhysics3DShapeType::Box;
		triggerPhysics->ShapeLocalExtents = SVector(1.6f, 0.5f, 2.3f);
		triggerPhysics->IsTrigger = true;

		GEngine::GetWorld()->Initialize3DPhysicsData(trigger);
		// === !Trigger ===

		// === Floor/Walls ===
		struct SWallAndFloorInitData
		{
			SWallAndFloorInitData(const SVector& translation, const SVector& eulerAngles, std::string editorName)
				: Translation(translation), EulerAngles(eulerAngles), EditorName(std::move(editorName))
			{}

			SVector Translation = SVector::Zero;
			SVector EulerAngles = SVector::Zero;
			std::string EditorName;
		};

		std::vector<SWallAndFloorInitData> initData;
		SVector floorRotation = SVector{ 0.0f, 0.0f, 0.0f };
		SVector largeWallRotation = SVector{ -90.0f, 0.0f, 0.0f};
		// TODO.NR: There's still a singularity happening here, need to figure out why
		SVector smallWallRotation = SVector{ 0.0f, 0.0f, -90.0f };

		initData.emplace_back(SVector{ -0.5f, 0.0f, -2.5f }, floorRotation, std::string("Floor"));
		initData.emplace_back(SVector{ 0.5f, 0.0f, -2.5f }, floorRotation, std::string("Floor"));
		initData.emplace_back(SVector{ 1.5f, 0.0f, -2.5f }, floorRotation, std::string("Floor"));
		initData.emplace_back(SVector{ 2.5f, 0.0f, -2.5f }, floorRotation, std::string("Floor"));
		initData.emplace_back(SVector{ -0.5f, 0.0f, -1.5f }, floorRotation, std::string("Floor"));
		initData.emplace_back(SVector{ 0.5f, 0.0f, -1.5f }, floorRotation, std::string("Floor"));
		initData.emplace_back(SVector{ 1.5f, 0.0f, -1.5f }, floorRotation, std::string("Floor"));
		initData.emplace_back(SVector{ 2.5f, 0.0f, -1.5f }, floorRotation, std::string("Floor"));
		initData.emplace_back(SVector{ -0.5f, 0.0f, -0.5f }, floorRotation, std::string("Floor"));
		initData.emplace_back(SVector{ 0.5f, 0.0f, -0.5f }, floorRotation, std::string("Floor"));
		initData.emplace_back(SVector{ 1.5f, 0.0f, -0.5f }, floorRotation, std::string("Floor"));
		initData.emplace_back(SVector{ 2.5f, 0.0f, -0.5f }, floorRotation, std::string("Floor"));
		initData.emplace_back(SVector{ -0.5f, 0.5f, 0.0f }, largeWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ 0.5f, 0.5f, 0.0f }, largeWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ 1.5f, 0.5f, 0.0f }, largeWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ 2.5f, 0.5f, 0.0f }, largeWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ -0.5f, 1.5f, 0.0f }, largeWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ 0.5f, 1.5f, 0.0f }, largeWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ 1.5f, 1.5f, 0.0f }, largeWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ 2.5f, 1.5f, 0.0f }, largeWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ -0.5f, 2.5f, 0.0f }, largeWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ 0.5f, 2.5f, 0.0f }, largeWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ 1.5f, 2.5f, 0.0f }, largeWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ 2.5f, 2.5f, 0.0f }, largeWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ -1.0f, 0.5f, -2.5f }, smallWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ -1.0f, 0.5f, -1.5f }, smallWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ -1.0f, 0.5f, -0.5f }, smallWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ -1.0f, 1.5f, -2.5f }, smallWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ -1.0f, 1.5f, -1.5f }, smallWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ -1.0f, 1.5f, -0.5f }, smallWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ -1.0f, 2.5f, -2.5f }, smallWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ -1.0f, 2.5f, -1.5f }, smallWallRotation, std::string("Wall"));
		initData.emplace_back(SVector{ -1.0f, 2.5f, -0.5f }, smallWallRotation, std::string("Wall"));

		for (const SWallAndFloorInitData& data : initData)
		{
			const SEntity& entity = AddEntity(data.EditorName);
			if (!entity.IsValid())
				return false;

			STransform& transform3 = AddComponent<STransformComponent>(entity)->Transform;
			AddComponentEditorContext(entity, &STransformComponentEditorContext::Context);
			SMatrix matrix3 = transform3.GetMatrix();
			matrix3.SetTranslation(data.Translation);
			matrix3.SetRotation(data.EulerAngles);
			transform3.SetMatrix(matrix3);

			renderManager->LoadStaticMeshComponent(modelPath3, AddComponent<SStaticMeshComponent>(entity));
			AddComponentEditorContext(entity, &SStaticMeshComponentEditorContext::Context);
			renderManager->LoadMaterialComponent(materialNames3, AddComponent<SMaterialComponent>(entity));
			AddComponentEditorContext(entity, &SMaterialComponentEditorContext::Context);

			GetComponent<SStaticMeshComponent>(entity)->AssetRegistryKey = assetRegistry->Register(modelPath3);
			GetComponent<SMaterialComponent>(entity)->AssetRegistryKeys = assetRegistry->Register(materialNames3);

			SPhysics3DComponent* physicsComponent = AddComponent<SPhysics3DComponent>(entity);
			AddComponentEditorContext(entity, &SPhysics3DComponentEditorContext::Context);

			physicsComponent->BodyType = EPhysics3DBodyType::Static;
			physicsComponent->ShapeType = EPhysics3DShapeType::Box;
			physicsComponent->ShapeLocalExtents = SVector(1.0f, 0.1f, 1.f);

			GEngine::GetWorld()->Initialize3DPhysicsData(entity);
		}
		// === !Floor/Walls ===

		return true;
	}

	bool CScene::Init2DDemoScene(CRenderManager* renderManager)
	{
		if (!Init(renderManager, "2DDemoScene"))
			return false;

		// === Camera ===
		MainCameraEntity = AddEntity("Camera");
		if (!MainCameraEntity.IsValid())
			return false;

		CAssetRegistry* assetRegistry = GEngine::GetWorld()->GetAssetRegistry();

		// Setup entities (create components)
		STransformComponent& transform = (*AddComponent<STransformComponent>(MainCameraEntity));
		AddComponentEditorContext(MainCameraEntity, &STransformComponentEditorContext::Context);
		transform.Transform.Translate({ 0.0f, 1.0f, -5.0f });
		//transform.Transform.Rotate({ 0.0f, UMath::DegToRad(35.0f), 0.0f });
		transform.Transform.Translate(SVector::Right * 0.25f);

		SCameraComponent& camera = *AddComponent<SCameraComponent>(MainCameraEntity);
		AddComponentEditorContext(MainCameraEntity, &SCameraComponentEditorContext::Context);
		camera.ProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), (16.0f / 9.0f), 0.1f, 1000.0f);
		//camera.ProjectionType = ECameraProjectionType::Orthographic;
		//camera.ProjectionMatrix = SMatrix::OrthographicLH(5.0f, 5.0f, 0.1f, 1000.0f);

		//		SCameraControllerComponent& controllerComp = 
		AddComponent<SCameraControllerComponent>(MainCameraEntity);
		AddComponentEditorContext(MainCameraEntity, &SCameraControllerComponentEditorContext::Context);
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
		AddComponentEditorContext(environmentLightEntity, &STransformComponentEditorContext::Context);
		renderManager->LoadEnvironmentLightComponent("Assets/Textures/Cubemaps/CubemapTheVisit.hva", AddComponent<SEnvironmentLightComponent>(environmentLightEntity));
		AddComponentEditorContext(environmentLightEntity, &SEnvironmentLightComponentEditorContext::Context);
		GetComponent<SEnvironmentLightComponent>(environmentLightEntity)->AssetRegistryKey = assetRegistry->Register("Assets/Textures/Cubemaps/CubemapTheVisit.hva");
		// === !Environment light ===

		// === Directional light ===
		const SEntity& directionalLightEntity = AddEntity("Directional Light");
		if (!directionalLightEntity.IsValid())
			return false;

		// NR: Add transform to directional light so it can filter environmental lights based on distance
		AddComponent<STransformComponent>(directionalLightEntity);
		AddComponentEditorContext(directionalLightEntity, &STransformComponentEditorContext::Context);

		SDirectionalLightComponent& directionalLight = *AddComponent<SDirectionalLightComponent>(directionalLightEntity);
		AddComponentEditorContext(directionalLightEntity, &SDirectionalLightComponentEditorContext::Context);
		directionalLight.Direction = { 1.0f, 1.0f, -1.0f, 0.0f };
		directionalLight.Color = { 212.0f / 255.0f, 175.0f / 255.0f, 55.0f / 255.0f, 0.25f };
		directionalLight.ShadowmapView.ShadowmapViewportIndex = 0;
		directionalLight.ShadowmapView.ShadowProjectionMatrix = SMatrix::OrthographicLH(directionalLight.ShadowViewSize.X, directionalLight.ShadowViewSize.Y, directionalLight.ShadowNearAndFarPlane.X, directionalLight.ShadowNearAndFarPlane.Y);

		SVolumetricLightComponent& volumetricLight = *AddComponent<SVolumetricLightComponent>(directionalLightEntity);
		AddComponentEditorContext(directionalLightEntity, &SVolumetricLightComponentEditorContext::Context);
		volumetricLight.IsActive = false;
		// === !Directional light ===

		// === Spotlight ===
		const SEntity& spotlight = AddEntity("Spot Light");
		if (!spotlight.IsValid())
			return true;

		STransform& spotlightTransform = (*AddComponent<STransformComponent>(spotlight)).Transform;
		AddComponentEditorContext(spotlight, &STransformComponentEditorContext::Context);
		SMatrix spotlightMatrix = spotlightTransform.GetMatrix();
		spotlightMatrix.SetTranslation({ 0.0f, 0.0f, 0.0f });
		spotlightTransform.SetMatrix(spotlightMatrix);

		SSpotLightComponent& spotlightComp = *AddComponent<SSpotLightComponent>(spotlight);
		AddComponentEditorContext(spotlight, &SSpotLightComponentEditorContext::Context);
		spotlightComp.Direction = SVector4::Forward;
		spotlightComp.DirectionNormal1 = SVector4::Right;
		spotlightComp.DirectionNormal2 = SVector4::Up;
		spotlightComp.ColorAndIntensity = { 0.0f, 1.0f, 0.0f, 5.0f };
		spotlightComp.OuterAngle = 40.0f;
		spotlightComp.InnerAngle = 30.0f;
		spotlightComp.Range = 3.0f;

		SVolumetricLightComponent& volumetricSpotLight = *AddComponent<SVolumetricLightComponent>(spotlight);
		AddComponentEditorContext(spotlight, &SVolumetricLightComponentEditorContext::Context);
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

		//	U16 spriteIndex = STATIC_U16(GetSceneIndex(*spriteWS));
		//	assetRegistry->Register(spritePath, SAssetReferenceCounter(EComponentType::SpriteComponent, spriteIndex, 0, 0));
		//}
		//// === !World Space Sprite ===

		return true;
	}

	U32 CScene::GetSize() const
	{
		U32 size = 0;

		size += GetDataSize(SceneName);
		size += GetDataSize(MainCameraEntity);
		size += GetDataSize(Entities);

		size += DefaultSizeAllocator(GetComponents<STransformComponent>());		

		size += SpecializedSizeAllocator(GetComponents<SStaticMeshComponent>());
		size += SpecializedSizeAllocator(GetComponents<SSkeletalMeshComponent>());

		size += DefaultSizeAllocator(GetComponents<SCameraComponent>());
		size += DefaultSizeAllocator(GetComponents<SCameraControllerComponent>());
		
		size += SpecializedSizeAllocator(GetComponents<SMaterialComponent>());
		
		size += DefaultSizeAllocator(GetComponents<SEnvironmentLightComponent>());
		size += DefaultSizeAllocator(GetComponents<SDirectionalLightComponent>());
		size += DefaultSizeAllocator(GetComponents<SPointLightComponent>());
		size += DefaultSizeAllocator(GetComponents<SSpotLightComponent>());
		size += DefaultSizeAllocator(GetComponents<SVolumetricLightComponent>());

		size += SpecializedSizeAllocator(GetComponents<SDecalComponent>());
		
		size += DefaultSizeAllocator(GetComponents<SSpriteComponent>());
		size += DefaultSizeAllocator(GetComponents<STransform2DComponent>());

		size += SpecializedSizeAllocator(GetComponents<SSpriteAnimatorGraphComponent>());
		size += SpecializedSizeAllocator(GetComponents<SSkeletalAnimationComponent>());

		// TODO.NR: Implement GetSize (since the component is not trivially serializable)
		//size += DefaultSizeAllocator(GetComponents<SSequencerComponent>());
		size += GetDataSize(STATIC_U32(GetComponents<SSequencerComponent>().size()));

		size += DefaultSizeAllocator(GetComponents<SPhysics2DComponent>());
		size += DefaultSizeAllocator(GetComponents<SPhysics3DComponent>());
		size += DefaultSizeAllocator(GetComponents<SPhysics3DControllerComponent>());
		size += DefaultSizeAllocator(GetComponents<SMetaDataComponent>());

		return size;
	}

	void CScene::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(SceneName, toData, pointerPosition);
		SerializeData(MainCameraEntity, toData, pointerPosition);
		SerializeData(Entities, toData, pointerPosition);

		DefaultSerializer(GetComponents<STransformComponent>(), toData, pointerPosition);

		SpecializedSerializer(GetComponents<SStaticMeshComponent>(), toData, pointerPosition);
		SpecializedSerializer(GetComponents<SSkeletalMeshComponent>(), toData, pointerPosition);

		DefaultSerializer(GetComponents<SCameraComponent>(), toData, pointerPosition);
		DefaultSerializer(GetComponents<SCameraControllerComponent>(), toData, pointerPosition);

		SpecializedSerializer(GetComponents<SMaterialComponent>(), toData, pointerPosition);

		DefaultSerializer(GetComponents<SEnvironmentLightComponent>(), toData, pointerPosition);
		DefaultSerializer(GetComponents<SDirectionalLightComponent>(), toData, pointerPosition);
		DefaultSerializer(GetComponents<SPointLightComponent>(), toData, pointerPosition);
		DefaultSerializer(GetComponents<SSpotLightComponent>(), toData, pointerPosition);
		DefaultSerializer(GetComponents<SVolumetricLightComponent>(), toData, pointerPosition);

		// NR: Texture info Saved and Loaded using AssetRegistry
		SpecializedSerializer(GetComponents<SDecalComponent>(), toData, pointerPosition);

		DefaultSerializer(GetComponents<SSpriteComponent>(), toData, pointerPosition);
		DefaultSerializer(GetComponents<STransform2DComponent>(), toData, pointerPosition);

		SpecializedSerializer(GetComponents<SSpriteAnimatorGraphComponent>(), toData, pointerPosition);
		SpecializedSerializer(GetComponents<SSkeletalAnimationComponent>(), toData, pointerPosition);

		// TODO.NR: Implement Serialize (since the component is not trivially serializable)
		const auto& sequencerComponents = GetComponents<SSequencerComponent>();
		SerializeData(STATIC_U32(sequencerComponents.size()), toData, pointerPosition);

		DefaultSerializer(GetComponents<SPhysics2DComponent>(), toData, pointerPosition);
		DefaultSerializer(GetComponents<SPhysics3DComponent>(), toData, pointerPosition);
		DefaultSerializer(GetComponents<SPhysics3DControllerComponent>(), toData, pointerPosition);
		DefaultSerializer(GetComponents<SMetaDataComponent>(), toData, pointerPosition);
	}

	void CScene::Deserialize(const char* fromData, U64& pointerPosition, CAssetRegistry* assetRegistry)
	{
		DeserializeData(SceneName, fromData, pointerPosition);
		DeserializeData(MainCameraEntity, fromData, pointerPosition);
		DeserializeData(Entities, fromData, pointerPosition);

		{
			std::vector<STransformComponent> components;
			DefaultDeserializer(components, &STransformComponentEditorContext::Context, fromData, pointerPosition);
		}

		{
			U32 numberOfStaticMeshComponents = 0;
			DeserializeData(numberOfStaticMeshComponents, fromData, pointerPosition);
			std::vector<SStaticMeshComponent> staticMeshComponents;
			staticMeshComponents.resize(numberOfStaticMeshComponents);

			for (U64 index = 0; index < numberOfStaticMeshComponents; index++)
			{
				SStaticMeshComponent component;
				component.Deserialize(fromData, pointerPosition);
				RenderManager->LoadStaticMeshComponent(assetRegistry->GetAssetPath(component.AssetRegistryKey), AddComponent<SStaticMeshComponent>(component.Owner));
				AddComponentEditorContext(component.Owner, &SStaticMeshComponentEditorContext::Context);
			}
		}

		{
			U32 numberOfSkeletalMeshComponents = 0;
			DeserializeData(numberOfSkeletalMeshComponents, fromData, pointerPosition);
			std::vector<SSkeletalMeshComponent> skeletalMeshComponents;
			skeletalMeshComponents.resize(numberOfSkeletalMeshComponents);

			for (U64 index = 0; index < numberOfSkeletalMeshComponents; index++)
			{
				SSkeletalMeshComponent component;
				component.Deserialize(fromData, pointerPosition);
				auto comp = AddComponent<SSkeletalMeshComponent>(component.Owner);
				RenderManager->LoadSkeletalMeshComponent(assetRegistry->GetAssetPath(component.AssetRegistryKey), comp);
				AddComponentEditorContext(component.Owner, &SSkeletalMeshComponentEditorContext::Context);
			}
		}

		{
			std::vector<SCameraComponent> components;
			DefaultDeserializer(components, &SCameraComponentEditorContext::Context, fromData, pointerPosition);
		}

		{
			std::vector<SCameraControllerComponent> components;
			DefaultDeserializer(components, &SCameraControllerComponentEditorContext::Context, fromData, pointerPosition);
		}

		U32 numberOfMaterialComponents = 0;
		DeserializeData(numberOfMaterialComponents, fromData, pointerPosition);
		std::vector<SMaterialComponent> materialComponents;
		materialComponents.resize(numberOfMaterialComponents);
		
		for (U64 index = 0; index < numberOfMaterialComponents; index++)
		{
			SMaterialComponent component;
			component.Deserialize(fromData, pointerPosition);
			RenderManager->LoadMaterialComponent(assetRegistry->GetAssetPaths(component.AssetRegistryKeys), AddComponent<SMaterialComponent>(component.Owner));
			AddComponentEditorContext(component.Owner, &SMaterialComponentEditorContext::Context);
		}

		U32 numberOfEnvironmentLightComponents = 0;
		DeserializeData(numberOfEnvironmentLightComponents, fromData, pointerPosition);
		std::vector<SEnvironmentLightComponent> environmentLightComponent;
		environmentLightComponent.resize(numberOfEnvironmentLightComponents);
		
		for (U64 index = 0; index < numberOfEnvironmentLightComponents; index++)
		{
			SEnvironmentLightComponent component;
			DeserializeData(component, fromData, pointerPosition);
			RenderManager->LoadEnvironmentLightComponent(assetRegistry->GetAssetPath(component.AssetRegistryKey), AddComponent<SEnvironmentLightComponent>(component.Owner));
			AddComponentEditorContext(component.Owner, &SEnvironmentLightComponentEditorContext::Context);
		}

		{
			std::vector<SDirectionalLightComponent> components;
			DefaultDeserializer(components, &SDirectionalLightComponentEditorContext::Context, fromData, pointerPosition);
		}

		{
			std::vector<SPointLightComponent> components;
			DefaultDeserializer(components, &SPointLightComponentEditorContext::Context, fromData, pointerPosition);
		}

		{
			std::vector<SSpotLightComponent> components;
			DefaultDeserializer(components, &SSpotLightComponentEditorContext::Context, fromData, pointerPosition);
		}

		{
			std::vector<SVolumetricLightComponent> components;
			DefaultDeserializer(components, &SVolumetricLightComponentEditorContext::Context, fromData, pointerPosition);
		}

		U32 numberOfDecalComponents = 0;
		DeserializeData(numberOfDecalComponents, fromData, pointerPosition);
		std::vector<SDecalComponent> decalComponents;
		decalComponents.resize(numberOfDecalComponents);
		
		for (U64 index = 0; index < numberOfDecalComponents; index++)
		{
			SDecalComponent component;
			component.Deserialize(fromData, pointerPosition);
			RenderManager->LoadDecalComponent(assetRegistry->GetAssetPaths(component.AssetRegistryKeys), AddComponent<SDecalComponent>(component.Owner));
			AddComponentEditorContext(component.Owner, &SDecalComponentEditorContext::Context);
		}

		U32 numberOfSpriteComponents = 0;
		DeserializeData(numberOfSpriteComponents, fromData, pointerPosition);
		std::vector<SSpriteComponent> spriteComponents;
		spriteComponents.resize(numberOfSpriteComponents);
		
		for (U64 index = 0; index < numberOfSpriteComponents; index++)
		{
			SSpriteComponent component;
			DeserializeData(component, fromData, pointerPosition);
			RenderManager->LoadSpriteComponent(assetRegistry->GetAssetPath(component.AssetRegistryKey), AddComponent<SSpriteComponent>(component.Owner));
			AddComponentEditorContext(component.Owner, &SSpriteComponentEditorContext::Context);
		}

		{
			std::vector<STransform2DComponent> components;
			DefaultDeserializer(components, &STransform2DComponentEditorContext::Context, fromData, pointerPosition);
		}

		{
			std::vector<SSpriteAnimatorGraphComponent> components;
			SpecializedDeserializer(components, &SSpriteAnimatorGraphComponentEditorContext::Context, fromData, pointerPosition);
		}

		{
			std::vector<SSkeletalAnimationComponent> components;
			SpecializedDeserializer(components, &SSkeletalAnimationComponentEditorContext::Context, fromData, pointerPosition);
		}

		U32 numberOfSequencerComponents = 0;
		DeserializeData(numberOfSequencerComponents, fromData, pointerPosition);

		{
			std::vector<SPhysics2DComponent> components;
			DefaultDeserializer(components, &SPhysics2DComponentEditorContext::Context, fromData, pointerPosition);
		}

		{
			std::vector<SPhysics3DComponent> components;
			DefaultDeserializer(components, &SPhysics3DComponentEditorContext::Context, fromData, pointerPosition);
		}

		{
			std::vector<SPhysics3DControllerComponent> components;
			DefaultDeserializer(components, &SPhysics3DControllerComponentEditorContext::Context, fromData, pointerPosition);
		}

		{
			std::vector<SMetaDataComponent> componentVector;
			U32 numberOfComponents = 0;
			DeserializeData(numberOfComponents, fromData, pointerPosition);
			componentVector.resize(numberOfComponents);

			for (U64 index = 0; index < numberOfComponents; index++)
			{
				SMetaDataComponent component;
				DeserializeData(component, fromData, pointerPosition);
				AddComponent(component, component.Owner);
			}
		}
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

	void CScene::AddComponentEditorContext(const SEntity& owner, SComponentEditorContext* context)
	{
		if (!EntityComponentEditorContexts.contains(owner.GUID))
			EntityComponentEditorContexts.emplace(owner.GUID, std::vector<SComponentEditorContext*>());
		 
		auto& contexts = EntityComponentEditorContexts.at(owner.GUID);
		contexts.push_back(context);

		std::sort(contexts.begin(), contexts.end(), [](const SComponentEditorContext* a, const SComponentEditorContext* b) { return a->GetSortingPriority() < b->GetSortingPriority(); });
	}

	void CScene::RemoveComponentEditorContext(const SEntity& owner, SComponentEditorContext* context)
	{
		if (!EntityComponentEditorContexts.contains(owner.GUID))
			return;

		auto& contexts = EntityComponentEditorContexts.at(owner.GUID);
		auto it = std::find(contexts.begin(), contexts.end(), context);
		if (it != contexts.end())
			contexts.erase(it);
	}

	void CScene::RemoveComponentEditorContexts(const SEntity& owner)
	{
		if (!EntityComponentEditorContexts.contains(owner.GUID))
			return;

		EntityComponentEditorContexts.at(owner.GUID).clear();
		EntityComponentEditorContexts.erase(owner.GUID);
	}

	std::vector<SComponentEditorContext*> CScene::GetComponentEditorContexts(const SEntity& owner)
	{
		if (!EntityComponentEditorContexts.contains(owner.GUID))
			return {};

		return EntityComponentEditorContexts.at(owner.GUID);
	}

	const std::vector<SComponentEditorContext*>& CScene::GetComponentEditorContexts() const
	{
		return RegisteredComponentEditorContexts;
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
