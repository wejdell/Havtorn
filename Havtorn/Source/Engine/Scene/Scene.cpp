// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Scene.h"

#include "ECS/ECSInclude.h"
#include "ECS/GUIDManager.h"
#include "Graphics/RenderManager.h"
#include "Assets/FileHeaderDeclarations.h"
#include "World.h"
#include "Assets/AssetRegistry.h"

#include "../Game/GameScript.h"

#include <algorithm>

namespace Havtorn
{
	CScene::CScene()
	{}

	CScene::~CScene()
	{
		ClearScene();
		RegisteredComponentEditorContexts.clear();
	}

	bool CScene::Init(const std::string& sceneName)
	{
		SceneName = sceneName;

		TypeHashToTypeID.emplace(typeid(SMetaDataComponent).hash_code(), 0);
		RegisterNonTrivialComponent<STransformComponent, STransformComponentEditorContext>(10, 50);
		RegisterNonTrivialComponent<SStaticMeshComponent, SStaticMeshComponentEditorContext>(20, 40);
		RegisterNonTrivialComponent<SSkeletalMeshComponent, SSkeletalMeshComponentEditorContext>(30, 40);
		RegisterTrivialComponent<SCameraComponent, SCameraComponentEditorContext>(40, 2);
		RegisterTrivialComponent<SCameraControllerComponent, SCameraControllerComponentEditorContext>(50, 2);
		RegisterNonTrivialComponent<SMaterialComponent, SMaterialComponentEditorContext>(60, 40);
		RegisterNonTrivialComponent<SEnvironmentLightComponent, SEnvironmentLightComponentEditorContext>(70, 1);
		RegisterTrivialComponent<SDirectionalLightComponent, SDirectionalLightComponentEditorContext>(80, 1);
		RegisterTrivialComponent<SPointLightComponent, SPointLightComponentEditorContext>(90, 1);
		RegisterTrivialComponent<SSpotLightComponent, SSpotLightComponentEditorContext>(100, 1);
		RegisterTrivialComponent<SVolumetricLightComponent, SVolumetricLightComponentEditorContext>(110, 3);
		RegisterNonTrivialComponent<SDecalComponent, SDecalComponentEditorContext>(120, 2);
		RegisterNonTrivialComponent<SSpriteComponent, SSpriteComponentEditorContext>(130, 10);
		RegisterTrivialComponent<STransform2DComponent, STransform2DComponentEditorContext>(140, 10);
		RegisterNonTrivialComponent<SSpriteAnimatorGraphComponent, SSpriteAnimatorGraphComponentEditorContext>(150, 2);
		RegisterNonTrivialComponent<SSkeletalAnimationComponent, SSkeletalAnimationComponentEditorContext>(160, 40);
		RegisterNonTrivialComponent<SScriptComponent, SScriptComponentEditorContext>(170, 10);
		RegisterTrivialComponent<SPhysics2DComponent, SPhysics2DComponentEditorContext>(180, 10);
		RegisterTrivialComponent<SPhysics3DComponent, SPhysics3DComponentEditorContext>(190, 40);
		RegisterTrivialComponent<SPhysics3DControllerComponent, SPhysics3DControllerComponentEditorContext>(200, 1);
		RegisterNonTrivialComponent<SUICanvasComponent, SUICanvasComponentEditorContext>(210, 5);
		//RegisterTrivialComponent<SSequencerComponent, SSequencerComponentEditorContext>(typeID++, 0);

		return true;
	}

	bool CScene::Init3DDefaults()
	{
		// === Camera ===
		SEntity mainCamera = AddEntity("Camera");

		if (!mainCamera.IsValid())
			return false;

		// Setup entities (create components)
		{
			STransformComponent& transform = *AddComponent<STransformComponent>(mainCamera);
			AddComponentEditorContext(mainCamera, &STransformComponentEditorContext::Context);

			transform.Transform.Translate({ 2.5f, 1.0f, -3.5f });
			transform.Transform.Rotate({ 0.0f, UMath::DegToRad(35.0f), 0.0f });
			transform.Transform.Translate(SVector::Right * 0.25f);

			SCameraComponent& camera = *AddComponent<SCameraComponent>(mainCamera);
			AddComponentEditorContext(mainCamera, &SCameraComponentEditorContext::Context);
			camera.ProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), (16.0f / 9.0f), 0.1f, 1000.0f);
			camera.IsActive = true;

			SCameraControllerComponent& controllerComp = *AddComponent<SCameraControllerComponent>(mainCamera);
			AddComponentEditorContext(mainCamera, &SCameraControllerComponentEditorContext::Context);

			SVector currentEuler = transform.Transform.GetMatrix().GetEuler();
			controllerComp.CurrentPitch = UMath::Clamp(currentEuler.X, -SCameraControllerComponent::MaxPitchDegrees + 0.01f, SCameraControllerComponent::MaxPitchDegrees - 0.01f);;
			controllerComp.CurrentYaw = UMath::WrapAngle(currentEuler.Y);
		}
		// === !Camera ===

		// === Game Camera ===
		{
			SEntity gameCamera = AddEntity("Game Camera");

			// Setup entities (create components)
			STransformComponent& transform = *AddComponent<STransformComponent>(gameCamera);
			AddComponentEditorContext(gameCamera, &STransformComponentEditorContext::Context);

			transform.Transform.Translate({ 2.6f, 1.0f, -1.6f });
			transform.Transform.Rotate({ 0.0f, UMath::DegToRad(70.0f), 0.0f });

			SCameraComponent& camera = *AddComponent<SCameraComponent>(gameCamera);
			AddComponentEditorContext(gameCamera, &SCameraComponentEditorContext::Context);
			camera.ProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), (16.0f / 9.0f), 0.1f, 6.0f);
			camera.IsStartingCamera = true;
			camera.FarClip = 6.0f;

			SCameraControllerComponent& controllerComp = *AddComponent<SCameraControllerComponent>(gameCamera);
			AddComponentEditorContext(gameCamera, &SCameraControllerComponentEditorContext::Context);
			
			SVector currentEuler = transform.Transform.GetMatrix().GetEuler();
			controllerComp.CurrentPitch = UMath::Clamp(currentEuler.X, -SCameraControllerComponent::MaxPitchDegrees + 0.01f, SCameraControllerComponent::MaxPitchDegrees - 0.01f);;
			controllerComp.CurrentYaw = UMath::WrapAngle(currentEuler.Y);
		}

		// === Environment light ===
		const SEntity& environmentLightEntity = AddEntity("Environment Light");
		if (!environmentLightEntity.IsValid())
			return false;

		AddComponent<STransformComponent>(environmentLightEntity);
		AddComponentEditorContext(environmentLightEntity, &STransformComponentEditorContext::Context);
		AddComponent<SEnvironmentLightComponent>(environmentLightEntity, "Resources/DefaultSkybox.hva");
		AddComponentEditorContext(environmentLightEntity, &SEnvironmentLightComponentEditorContext::Context);
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

		return true;
	}

	bool CScene::Init3DDemoScene()
	{
		if (!Init("3DDemoScene"))
			return false;

		if (!Init3DDefaults())
			return false;

		SEnvironmentLightComponent* environmentLightComponent = GetComponents<SEnvironmentLightComponent>()[0];
		environmentLightComponent->AssetReference = SAssetReference("Assets/Textures/Cubemaps/CubemapTheVisit.hva");

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

		// TODO.NW: Still want to use a Material in decals instead of this texture setup
		std::vector<std::string> decalTextures = { "Assets/Textures/T_noscare_AL_c.hva", "Assets/Textures/T_noscare_AL_m.hva", "Assets/Textures/T_noscare_AL_n.hva" };
		SDecalComponent& decalComp = *AddComponent<SDecalComponent>(decal, decalTextures);
		AddComponentEditorContext(decal, &SDecalComponentEditorContext::Context);

		decalComp.ShouldRenderAlbedo = true;
		decalComp.ShouldRenderMaterial = true;
		decalComp.ShouldRenderNormal = true;

		// === !Decal ===

		const std::string modelPath1 = "Assets/Meshes/En_P_PendulumClock.hva";
		const std::vector<std::string> materialNames1 = { "Assets/Materials/M_PendulumClock.hva", "Assets/Materials/M_Misc.hva" };
		const std::string modelPath2 = "Assets/Meshes/En_P_Bed.hva";
		const std::vector<std::string> materialNames2 = { "Assets/Materials/M_Bed.hva", "Assets/Materials/M_Bedsheet.hva" };
		const std::string modelPath3 = "Assets/Meshes/Plane.hva";
		const std::vector<std::string> materialNames3 = { "Assets/Materials/M_Quad.hva" };
		const std::string modelPath4 = "Assets/Meshes/En_P_WallLamp.hva";
		const std::vector<std::string> materialNames4 = { "Assets/Materials/M_Quad.hva", "Assets/Materials/M_Emissive.hva", "Assets/Materials/M_Headlamp.hva" };
		const std::string modelPath5 = "Assets/Meshes/Cube_1.hva";
		const std::vector<std::string> materialNames5 = { "Assets/Materials/M_Quad.hva" };

		// === Pendulum ===
		const SEntity& pendulum = AddEntity("Clock");
		if (!pendulum.IsValid())
			return false;

		STransform& transform1 = (*AddComponent<STransformComponent>(pendulum)).Transform;
		AddComponentEditorContext(pendulum, &STransformComponentEditorContext::Context);
		transform1.Translate({ 1.8f, 0.0f, -0.2f });

		AddComponent<SStaticMeshComponent>(pendulum, modelPath1);
		AddComponentEditorContext(pendulum, &SStaticMeshComponentEditorContext::Context);
		AddComponent<SMaterialComponent>(pendulum, materialNames1);
		AddComponentEditorContext(pendulum, &SMaterialComponentEditorContext::Context);

		SPhysics3DComponent* clockPhysics = AddComponent<SPhysics3DComponent>(pendulum);
		AddComponentEditorContext(pendulum, &SPhysics3DComponentEditorContext::Context);

		clockPhysics->BodyType = EPhysics3DBodyType::Static;
		clockPhysics->ShapeType = EPhysics3DShapeType::Box;
		clockPhysics->ShapeLocalExtents = SVector(0.6f, 1.9f, 0.3f);
		clockPhysics->ShapeLocalOffset = SVector(0.0f, 0.95f, 0.0f);

		GEngine::GetWorld()->Initialize3DPhysicsData(pendulum);
		//GEngine::GetWorld()->Initialize3DPhysicsData(pendulum);
		// === !Pendulum ===

		// === Bed ===
		const SEntity& bed = AddEntity("Bed");
		if (!bed.IsValid())
			return false;

		STransform& transform2 = (*AddComponent<STransformComponent>(bed)).Transform;
		AddComponentEditorContext(bed, &STransformComponentEditorContext::Context);
		transform2.Translate({ 0.2f, 0.0f, 0.0f });

		AddComponent<SStaticMeshComponent>(bed, modelPath2);
		AddComponentEditorContext(bed, &SStaticMeshComponentEditorContext::Context);
		AddComponent<SMaterialComponent>(bed, materialNames2);
		AddComponentEditorContext(bed, &SMaterialComponentEditorContext::Context);

		SPhysics3DComponent* bedPhysics = AddComponent<SPhysics3DComponent>(bed);
		AddComponentEditorContext(bed, &SPhysics3DComponentEditorContext::Context);

		bedPhysics->BodyType = EPhysics3DBodyType::Static;
		bedPhysics->ShapeType = EPhysics3DShapeType::Box;
		bedPhysics->ShapeLocalExtents = SVector(1.8f, 0.7f, 2.5f);
		bedPhysics->ShapeLocalOffset = SVector(0.0f, bedPhysics->ShapeLocalExtents.Y * 0.5f, -bedPhysics->ShapeLocalExtents.Z * 0.5f);

		GEngine::GetWorld()->Initialize3DPhysicsData(bed);
		//GEngine::GetWorld()->Initialize3DPhysicsData(bed);
		// === !Bed ===

		// === Lamp ===
		const SEntity& lamp = AddEntity("Lamp");
		if (!lamp.IsValid())
			return false;

		STransform& lampTransform = (*AddComponent<STransformComponent>(lamp)).Transform;
		AddComponentEditorContext(lamp, &STransformComponentEditorContext::Context);
		lampTransform.Translate({ -1.0f, 1.4f, -1.25f });
		lampTransform.Rotate({ 0.0f, UMath::DegToRad(90.0f), 0.0f });

		AddComponent<SStaticMeshComponent>(lamp, modelPath4);
		AddComponentEditorContext(lamp, &SStaticMeshComponentEditorContext::Context);
		AddComponent<SMaterialComponent>(lamp, materialNames4);
		AddComponentEditorContext(lamp, &SMaterialComponentEditorContext::Context);
		// === !Lamp ===

			// === Player Proxy ===
		const SEntity& playerProxy = AddEntity("Player");
		if (!playerProxy.IsValid())
			return false;

		STransform& playerTransform = AddComponent<STransformComponent>(playerProxy)->Transform;
		AddComponentEditorContext(playerProxy, &STransformComponentEditorContext::Context);
		SMatrix playerMatrix = playerTransform.GetMatrix();
		//playerMatrix.SetScale(SVector(0.01f));
		playerMatrix.SetTranslation({ 2.6f, 0.0f, -0.24f });
		playerTransform.SetMatrix(playerMatrix);

		SPhysics3DControllerComponent* controllerComponent = AddComponent<SPhysics3DControllerComponent>(playerProxy);
		AddComponentEditorContext(playerProxy, &SPhysics3DControllerComponentEditorContext::Context);

		controllerComponent->ControllerType = EPhysics3DControllerType::Capsule;
		controllerComponent->ShapeLocalRadiusAndHeight = SVector2(0.25f, 1.0f);

		GEngine::GetWorld()->Initialize3DPhysicsData(playerProxy);
		//GEngine::GetWorld()->Initialize3DPhysicsData(playerProxy);

		// Skeletal Mesh
		std::string meshPath = "Assets/Meshes/CH_Enemy_SK.hva";
		AddComponent<SSkeletalMeshComponent>(playerProxy, meshPath);
		AddComponentEditorContext(playerProxy, &SSkeletalMeshComponentEditorContext::Context);

		const std::vector<std::string> animationPaths = { "Assets/Meshes/CH_Enemy_Walk.hva", "Assets/Meshes/CH_Enemy_Chase.hva" };
		SSkeletalAnimationComponent* animationComponent = AddComponent<SSkeletalAnimationComponent>(playerProxy, animationPaths);
		AddComponentEditorContext(playerProxy, &SSkeletalAnimationComponentEditorContext::Context);

		animationComponent->PlayData.emplace_back(SSkeletalAnimationPlayData());
		animationComponent->PlayData.emplace_back(SSkeletalAnimationPlayData(1));

		std::vector<std::string> enemyMaterialPaths = { "Assets/Materials/M_Enemy.hva" };
		AddComponent<SMaterialComponent>(playerProxy, enemyMaterialPaths);
		AddComponentEditorContext(playerProxy, &SMaterialComponentEditorContext::Context);
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

		AddComponent<SStaticMeshComponent>(crate, modelPath5);
		AddComponentEditorContext(crate, &SStaticMeshComponentEditorContext::Context);
		AddComponent<SMaterialComponent>(crate, materialNames5);
		AddComponentEditorContext(crate, &SMaterialComponentEditorContext::Context);

		SPhysics3DComponent* cratePhysics = AddComponent<SPhysics3DComponent>(crate);
		AddComponentEditorContext(crate, &SPhysics3DComponentEditorContext::Context);

		cratePhysics->BodyType = EPhysics3DBodyType::Dynamic;
		cratePhysics->ShapeType = EPhysics3DShapeType::Box;
		cratePhysics->ShapeLocalExtents = SVector(0.5f);

		GEngine::GetWorld()->Initialize3DPhysicsData(crate);
		//GEngine::GetWorld()->Initialize3DPhysicsData(crate);
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
		//GEngine::GetWorld()->Initialize3DPhysicsData(trigger);
		// === !Trigger ===

		// === Room ===
		const SEntity& room = AddEntity("Room");
		STransformComponent* roomTransform = AddComponent<STransformComponent>(room);
		AddComponentEditorContext(room, &STransformComponentEditorContext::Context);
		// === !Room ===

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
		SVector floorRotation = SVector{ 90.0f, 0.0f, 0.0f };
		SVector largeWallRotation = SVector{ 0.0f, 0.0f, 0.0f };
		// TODO.NW: There's still a singularity happening here, need to figure out why
		SVector smallWallRotation = SVector{ 0.0f, -90.0f, 0.0f };

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

			STransformComponent* transformComponent = AddComponent<STransformComponent>(entity);
			STransform& transform3 = transformComponent->Transform;
			AddComponentEditorContext(entity, &STransformComponentEditorContext::Context);
			SMatrix matrix3 = transform3.GetMatrix();
			matrix3.SetTranslation(data.Translation);
			matrix3.SetRotation(data.EulerAngles);
			transform3.SetMatrix(matrix3);

			AddComponent<SStaticMeshComponent>(entity, modelPath3);
			AddComponentEditorContext(entity, &SStaticMeshComponentEditorContext::Context);
			AddComponent<SMaterialComponent>(entity, materialNames3);
			AddComponentEditorContext(entity, &SMaterialComponentEditorContext::Context);

			SPhysics3DComponent* physicsComponent = AddComponent<SPhysics3DComponent>(entity);
			AddComponentEditorContext(entity, &SPhysics3DComponentEditorContext::Context);

			physicsComponent->BodyType = EPhysics3DBodyType::Static;
			physicsComponent->ShapeType = EPhysics3DShapeType::Box;
			physicsComponent->ShapeLocalExtents = SVector(1.0f, 0.1f, 1.f);

			roomTransform->Attach(transformComponent);

			GEngine::GetWorld()->Initialize3DPhysicsData(entity);
			//GEngine::GetWorld()->Initialize3DPhysicsData(entity);
		}
		// === !Floor/Walls ===

		// === Main Menu ===
		const SEntity& mainMenuCanvas = AddEntity("Main Menu Canvas");
		const SEntity& settingsMenuCanvas = AddEntity("Settings Canvas");

		if (!mainMenuCanvas.IsValid() || !settingsMenuCanvas.IsValid())
			return false;

		STransform2DComponent& mainMenuCanvasTransformComp = *AddComponent<STransform2DComponent>(mainMenuCanvas);
		AddComponentEditorContext(mainMenuCanvas, &STransform2DComponentEditorContext::Context);

		mainMenuCanvasTransformComp.Position = { 0.5f, 0.5f };

		SUICanvasComponent& mainMenuCanvasComponent = *AddComponent<SUICanvasComponent>(mainMenuCanvas);
		AddComponentEditorContext(mainMenuCanvas, &SUICanvasComponentEditorContext::Context);
		mainMenuCanvasComponent.IsActive = true;

		const std::vector<SAssetReference> assets = { SAssetReference("Assets/Textures/UITextures.hva"), SAssetReference("Assets/Textures/UITextures.hva"), SAssetReference("Assets/Textures/UITextures.hva") };
		const SVector2<F32> buttonScale = SVector2<F32>(0.3f, 0.1f);
		const F32 halfHeight = 0.5f * buttonScale.Y;
		const F32 halfWidth = 0.5f * buttonScale.X * (9.0f / 16.0f);
		const SVector4 collisionRect = { -halfWidth, -halfHeight, halfWidth, halfHeight };

		SUIElement& playButton = mainMenuCanvasComponent.Elements.emplace_back();
		playButton.StateAssetReferences = assets;
		playButton.LocalPosition = SVector2<F32>(0.0f, 0.3f);
		playButton.LocalScale = buttonScale;
		playButton.CollisionRect = collisionRect;
		playButton.UVRects = { SVector4(0.0f, 0.0f, 0.5f, 1 / 8.0f), SVector4(0.5f, 0.0f, 1.0f, 1 / 8.0f), SVector4(0.0f, 1 / 8.0f, 0.5f, 2 / 8.0f) };
		
		SUIElement& settingsButton = mainMenuCanvasComponent.Elements.emplace_back();
		settingsButton.StateAssetReferences = assets;
		settingsButton.LocalPosition = SVector2<F32>(0.0f, 0.1f);
		settingsButton.LocalScale = buttonScale;
		settingsButton.CollisionRect = collisionRect;
		settingsButton.UVRects = { SVector4(0.5f, 1 / 8.0f, 1.0f, 2 / 8.0f), SVector4(0.0f, 2 / 8.0f, 0.5f, 3 / 8.0f), SVector4(0.5f, 2 / 8.0f, 1.0f, 3 / 8.0f) };
		settingsButton.BindingType = EUIBindingType::OtherCanvas;
		settingsButton.BoundData = settingsMenuCanvas.GUID;

		SUIElement& quitButton = mainMenuCanvasComponent.Elements.emplace_back();
		quitButton.StateAssetReferences = assets;
		quitButton.LocalPosition = SVector2<F32>(0.0f, -0.1f);
		quitButton.LocalScale = buttonScale;
		quitButton.CollisionRect = collisionRect;
		quitButton.UVRects = { SVector4(0.0f, 3 / 8.0f, 0.5f, 4 / 8.0f), SVector4(0.5f, 3 / 8.0f, 1.0f, 4 / 8.0f), SVector4(0.0f, 4 / 8.0f, 0.5f, 5 / 8.0f) };
		quitButton.BindingType = EUIBindingType::GenericFunction;
		quitButton.BoundData = std::hash<std::string>{}("CGameManager::QuitGame");
		// === !Main Menu ===

		// === Settings Menu ===
		if (!settingsMenuCanvas.IsValid())
			return false;

		STransform2DComponent& settingsMenuCanvasTransformComp = *AddComponent<STransform2DComponent>(settingsMenuCanvas);
		AddComponentEditorContext(settingsMenuCanvas, &STransform2DComponentEditorContext::Context);

		settingsMenuCanvasTransformComp.Position = { 0.5f, 0.5f };

		SUICanvasComponent& settingsMenuCanvasComponent = *AddComponent<SUICanvasComponent>(settingsMenuCanvas);
		AddComponentEditorContext(settingsMenuCanvas, &SUICanvasComponentEditorContext::Context);

		SUIElement& muteButton = settingsMenuCanvasComponent.Elements.emplace_back();
		muteButton.StateAssetReferences = assets;
		muteButton.LocalPosition = SVector2<F32>(0.0f, 0.3f);
		muteButton.LocalScale = buttonScale;
		muteButton.CollisionRect = collisionRect;
		muteButton.UVRects = { SVector4(0.5f, 4 / 8.0f, 1.0f, 5 / 8.0f), SVector4(0.0f, 5 / 8.0f, 0.5f, 6 / 8.0f), SVector4(0.5f, 5 / 8.0f, 1.0f, 6 / 8.0f) };

		SUIElement& backButton = settingsMenuCanvasComponent.Elements.emplace_back();
		backButton.StateAssetReferences = assets;
		backButton.LocalPosition = SVector2<F32>(0.0f, 0.1f);
		backButton.LocalScale = buttonScale;
		backButton.CollisionRect = collisionRect;
		backButton.UVRects = { SVector4(0.0f, 6 / 8.0f, 0.5f, 7 / 8.0f), SVector4(0.5f, 6 / 8.0f, 1.0f, 7 / 8.0f), SVector4(0.0f, 7 / 8.0f, 0.5f, 8 / 8.0f) };
		backButton.BindingType = EUIBindingType::OtherCanvas;
		backButton.BoundData = mainMenuCanvas.GUID;
		// === !Settings Menu ===

		return true;
	}

	bool CScene::Init2DDemoScene()
	{
		if (!Init("2DDemoScene"))
			return false;

		// === Camera ===
		{
			SEntity mainCamera = AddEntity("Camera");
			if (!mainCamera.IsValid())
				return false;

			// Setup entities (create components)
			STransformComponent& transform = (*AddComponent<STransformComponent>(mainCamera));
			AddComponentEditorContext(mainCamera, &STransformComponentEditorContext::Context);
			transform.Transform.Translate({ 0.0f, 1.0f, -5.0f });
			//transform.Transform.Rotate({ 0.0f, UMath::DegToRad(35.0f), 0.0f });
			transform.Transform.Translate(SVector::Right * 0.25f);

			SCameraComponent& camera = *AddComponent<SCameraComponent>(mainCamera);
			AddComponentEditorContext(mainCamera, &SCameraComponentEditorContext::Context);
			camera.ProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), (16.0f / 9.0f), 0.1f, 1000.0f);
			//camera.ProjectionType = ECameraProjectionType::Orthographic;
			//camera.ProjectionMatrix = SMatrix::OrthographicLH(5.0f, 5.0f, 0.1f, 1000.0f);
			camera.IsActive = true;

			//		SCameraControllerComponent& controllerComp = 
			AddComponent<SCameraControllerComponent>(mainCamera);
			AddComponentEditorContext(mainCamera, &SCameraControllerComponentEditorContext::Context);

			//SVector currentEuler = transform.Transform.GetMatrix().GetEuler();
			//controllerComp.CurrentPitch = UMath::Clamp(currentEuler.X, -SCameraControllerComponent::MaxPitchDegrees + 0.01f, SCameraControllerComponent::MaxPitchDegrees - 0.01f);;
			//controllerComp.CurrentYaw = UMath::WrapAngle(currentEuler.Y);
			//controllerComp.CurrentYaw = UMath::DegToRad(-35.0f);
			// 
			//SSequencerComponent& cameraSequencerComponent = AddSequencerComponentToEntity(*cameraEntity);
			//cameraSequencerComponent.ComponentTracks.push_back({ EComponentType::TransformComponent });
		}
		// === !Camera ===

		// === Game Camera ===
		{
			SEntity gameCamera = AddEntity("Game Camera");

			// Setup entities (create components)
			STransformComponent& transform = *AddComponent<STransformComponent>(gameCamera);
			AddComponentEditorContext(gameCamera, &STransformComponentEditorContext::Context);

			transform.Transform.Translate({ 1.5f, 1.0f, -3.0f });

			SCameraComponent& camera = *AddComponent<SCameraComponent>(gameCamera);
			AddComponentEditorContext(gameCamera, &SCameraComponentEditorContext::Context);
			camera.ProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), (16.0f / 9.0f), 0.1f, 6.0f);
			camera.FarClip = 6.0f;

			AddComponent<SCameraControllerComponent>(gameCamera);
			AddComponentEditorContext(gameCamera, &SCameraControllerComponentEditorContext::Context);
		}

		// === Environment light ===
		const SEntity& environmentLightEntity = AddEntity("Environment Light");
		if (!environmentLightEntity.IsValid())
			return false;

		AddComponent<STransformComponent>(environmentLightEntity);
		AddComponentEditorContext(environmentLightEntity, &STransformComponentEditorContext::Context);
		AddComponent<SEnvironmentLightComponent>(environmentLightEntity, "Assets/Textures/Cubemaps/CubemapTheVisit.hva");
		AddComponentEditorContext(environmentLightEntity, &SEnvironmentLightComponentEditorContext::Context);
		// === !Environment light ===

		// === Directional light ===
		const SEntity& directionalLightEntity = AddEntity("Directional Light");
		if (!directionalLightEntity.IsValid())
			return false;

		// NW: Add transform to directional light so it can filter environmental lights based on distance
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

		// Scene File Size
		size += GetDataSize(U32());

		size += GetDataSize(SceneName);
		size += GetDataSize(Entities);

		// MetaData typeID and blob size 
		size += GetDataSize(U32());
		size += GetDataSize(U32());
		size += DefaultSizeAllocator(GetComponents<SMetaDataComponent>());

		for (auto [key, val] : ComponentSerializers)
		{
			size += val.SizeAllocator(this);
		}

		return size;
	}

	void CScene::Serialize(char* toData, U64& pointerPosition) const
	{
		// Make sure to add pointerPosition here, to take outer data members into account
		SerializeData(GetSize() + STATIC_U32(pointerPosition), toData, pointerPosition);

		SerializeData(SceneName, toData, pointerPosition);
		SerializeData(Entities, toData, pointerPosition);

		U32 typeID = TypeHashToTypeID.at(typeid(SMetaDataComponent).hash_code());
		SerializeData(typeID, toData, pointerPosition);
		U32 size = DefaultSizeAllocator(GetComponents<SMetaDataComponent>());
		SerializeData(size, toData, pointerPosition);
		DefaultSerializer(GetComponents<SMetaDataComponent>(), toData, pointerPosition);

		for (auto [key, val] : ComponentSerializers)
		{
			val.Serializer(this, toData, pointerPosition);
		}
	}

	void CScene::Deserialize(const char* fromData, U64& pointerPosition)
	{
		U32 sceneFileSize = 0;
		DeserializeData(sceneFileSize, fromData, pointerPosition);

		DeserializeData(SceneName, fromData, pointerPosition);
		
		std::vector<SEntity> entities;
		DeserializeData(entities, fromData, pointerPosition);

		for (SEntity& entity : entities)
			AddEntity(entity.GUID);

		{
			U32 savedTypeID = 0;
			DeserializeData(savedTypeID, fromData, pointerPosition);
			U32 savedBlobSize = 0;
			DeserializeData(savedBlobSize, fromData, pointerPosition);

			U32 metaDataTypeID = UMath::MaxU32;
			if (TypeHashToTypeID.contains(typeid(SMetaDataComponent).hash_code()))
				metaDataTypeID = TypeHashToTypeID.at(typeid(SMetaDataComponent).hash_code());

			if (metaDataTypeID == savedTypeID) 
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
			else
			{
				pointerPosition += savedBlobSize;
			}
		}

		while (pointerPosition < sceneFileSize)
		{
			U32 savedTypeID = 0;
			DeserializeData(savedTypeID, fromData, pointerPosition);
			U32 savedBlobSize = 0;
			DeserializeData(savedBlobSize, fromData, pointerPosition);

			if (!ComponentSerializers.contains(savedTypeID))
			{
				pointerPosition += savedBlobSize;
				continue;
			} 
			// TODO.NW: Add check in VersioningService, which can take the blob and convert it from an old version to a new one

			ComponentSerializers.at(savedTypeID).Deserializer(this, fromData, pointerPosition);
		}

		 // Post pass to set up inter-entity connections
		for (STransformComponent* transformComponent : GetComponents<STransformComponent>())
		{
			for (const SEntity& serializationAttachedEntity : transformComponent->AttachedEntities)
				transformComponent->Attach(GetComponent<STransformComponent>(serializationAttachedEntity));
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

	bool CScene::HasEntity(U64 guid) const
	{
		return EntityIndices.contains(guid);
	}

	void CScene::RemoveEntity(const SEntity entity)
	{
		if (!entity.IsValid())
		{
			HV_LOG_ERROR("__FUNCTION__: Tried to remove an invalid Entity.");
			return;
		}

		if (!HasEntity(entity.GUID))
		{
			HV_LOG_ERROR("__FUNCTION__: Tried to remove entity with GUID: %u from a scene that does not contain it.", entity.GUID);
			return;
		}

		OnEntityPreDestroy.Broadcast(entity);

		for (SComponentStorage& storage : Storages)
		{
			if (storage.EntityIndices.contains(entity.GUID))
			{
				SComponent*& componentToBeRemoved = storage.Components.back();
				if (componentToBeRemoved != nullptr)
				{
					storage.EntityIndices.at(componentToBeRemoved->Owner.GUID) = storage.EntityIndices.at(entity.GUID);

					std::swap(storage.Components[storage.EntityIndices.at(entity.GUID)], storage.Components.back());

					componentToBeRemoved->IsDeleted(this);
					delete componentToBeRemoved;
					componentToBeRemoved = nullptr;
				}

				storage.Components.pop_back();
				storage.EntityIndices.erase(entity.GUID);
			}
		}

		RemoveComponentEditorContexts(entity);

		SEntity& entityAtBack = Entities.back();
		EntityIndices.at(entityAtBack.GUID) = EntityIndices.at(entity.GUID);

		std::swap(Entities[EntityIndices.at(entity.GUID)], entityAtBack);

		Entities.pop_back();
		EntityIndices.erase(entity.GUID);
	}

	void CScene::ClearScene()
	{
		std::vector<SEntity> copy = Entities;
		for (const SEntity entity : copy)
			RemoveEntity(entity);
	}

	void CScene::MoveEntityToScene(const SEntity& entity, CScene* fromScene)
	{
		if (!entity.IsValid() || fromScene == nullptr)
		{
			if (entity.IsValid())
				HV_LOG_ERROR("CScene::MoveEntityToScene: Could not move entity %ull to %s from other scene.", entity.GUID, SceneName.AsString().c_str());
			else
				HV_LOG_ERROR("CScene::MoveEntityToScene: Could not move entity to %s from other scene.", SceneName.AsString().c_str());

			return;
		}

		if (fromScene == this)
			return;

		if (HasEntity(entity.GUID))
		{
			HV_LOG_ERROR("CScene::MoveEntityToScene: %s already has entity %ull", SceneName.AsString().c_str(), entity.GUID);
			return;
		}

		AddEntity(entity.GUID);

		if (SMetaDataComponent* metaDataComponent = fromScene->GetComponent<SMetaDataComponent>(entity))
			AddComponent<SMetaDataComponent>(*metaDataComponent, entity);

		for (auto& [typeID, storageIndex] : fromScene->ComponentTypeIndices)
		{
			SComponentStorage& storage = fromScene->Storages[storageIndex];
			if (!storage.EntityIndices.contains(entity.GUID))
				continue;

			if (!ComponentSerializers.contains(typeID))
				continue;

			U32 size = ComponentSerializers.at(typeID).SingleSizeAllocator(entity, fromScene);
			const auto data = new char[size];
			U64 pointerPosition = 0;
			ComponentSerializers.at(typeID).SingleSerializer(entity, fromScene, data, pointerPosition);

			pointerPosition = 0;
			ComponentSerializers.at(typeID).SingleDeserializer(entity, this, data, pointerPosition);

			delete[] data;
		}

		fromScene->RemoveEntity(entity);
	}

	SEntity CScene::CopyEntity(const SEntity& fromEntity)
	{
		std::string newEntityName = "UNNAMED";
		if (SMetaDataComponent* metaDataComponent = GetComponent<SMetaDataComponent>(fromEntity))
		{
			newEntityName = UGeneralUtils::GetNonCollidingString(metaDataComponent->Name.AsString(), Entities, [this](const SEntity& entity)
				{
					const SMetaDataComponent* metaDataComp = GetComponent<SMetaDataComponent>(entity);
					return SComponent::IsValid(metaDataComp) ? metaDataComp->Name.AsString() : "UNNAMED";
				}
			);
		}

		SEntity newEntity = AddEntity(newEntityName);

		for (auto& [typeID, storageIndex] : ComponentTypeIndices)
		{
			SComponentStorage& storage = Storages[storageIndex];
			if (!storage.EntityIndices.contains(fromEntity.GUID))
				continue;

			if (!ComponentSerializers.contains(typeID))
				continue;

			U32 size = ComponentSerializers.at(typeID).SingleSizeAllocator(fromEntity, this);
			const auto data = new char[size];
			U64 pointerPosition = 0;
			ComponentSerializers.at(typeID).SingleSerializer(fromEntity, this, data, pointerPosition);

			pointerPosition = 0;
			ComponentSerializers.at(typeID).SingleDeserializer(newEntity, this, data, pointerPosition);

			delete[] data;
		}

		return newEntity;
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
}
