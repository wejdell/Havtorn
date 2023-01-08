// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Scene.h"

#include "ECS/ECSInclude.h"
#include "Graphics/RenderManager.h"
#include "FileSystem/FileHeaderDeclarations.h"
#include "World.h"
#include "AssetRegistry.h"

namespace Havtorn
{
	U64 gEntityGUID = 0;

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
		DebugShapeComponents.resize(ENTITY_LIMIT);
		MetaDataComponents.resize(ENTITY_LIMIT);
	}

	bool CScene::Init(CRenderManager* /*renderManager*/, CAssetRegistry* /*assetRegistry*/, U8 /*sceneIndex*/)
	{
		// NR: This function assumes that the scene (with correponding sceneIndex) has been loaded from file
		// and thus has entries in the AssetRegistry

		//for (const Ref<SEntity>& entity : Entities)
		//for (const SEntity* entity : Entities)
		//{
		//	if (entity->HasComponent(EComponentType::EnvironmentLightComponent))
		//	{
		//		U16 componentIndex = static_cast<U16>(entity->GetComponentIndex(EComponentType::EnvironmentLightComponent)); 
		//		SAssetReferenceCounter counter = { EComponentType::EnvironmentLightComponent, componentIndex, 0, sceneIndex };
		//		const Ref<SEnvironmentLightComponent>& environmentLightComponent = EnvironmentLightComponents[componentIndex];
		//		
		//		renderManager->LoadEnvironmentLightComponent(assetRegistry->GetAssetPath(counter), environmentLightComponent.get());
		//	}

		//	if (entity->HasComponent(EComponentType::DecalComponent))
		//	{
		//		U16 componentIndex = static_cast<U16>(entity->GetComponentIndex(EComponentType::DecalComponent));
		//		SAssetReferenceCounter counter = { EComponentType::DecalComponent, componentIndex, 0, sceneIndex };
		//		const Ref<SDecalComponent>& decalComponent = DecalComponents[componentIndex];

		//		renderManager->LoadDecalComponent(assetRegistry->GetAssetPaths(counter), decalComponent.get());
		//	}

		//	if (entity->HasComponent(EComponentType::StaticMeshComponent))
		//	{
		//		U16 componentIndex = static_cast<U16>(entity->GetComponentIndex(EComponentType::StaticMeshComponent));
		//		SAssetReferenceCounter counter = { EComponentType::StaticMeshComponent, componentIndex, 0, sceneIndex };
		//		const Ref<SStaticMeshComponent>& staticMeshComponent = StaticMeshComponents[componentIndex];

		//		renderManager->LoadStaticMeshComponent(assetRegistry->GetAssetPath(counter), staticMeshComponent.get());
		//	}

		//	if (entity->HasComponent(EComponentType::MaterialComponent))
		//	{
		//		U16 componentIndex = static_cast<U16>(entity->GetComponentIndex(EComponentType::MaterialComponent));
		//		SAssetReferenceCounter counter = { EComponentType::MaterialComponent, componentIndex, 0, sceneIndex };
		//		const Ref<SMaterialComponent>& materialComponent = MaterialComponents[componentIndex];

		//		renderManager->LoadMaterialComponent(assetRegistry->GetAssetPaths(counter), materialComponent.get());
		//	}
		//}

		return true;
	}

	bool CScene::InitDemoScene(CRenderManager* renderManager)
	{
		// Create entities
		SEntity cameraEntity;
		TryGetNewEntity(cameraEntity);

		// Setup entities (create components)
		STransformComponent& transform = AddTransformComponentToEntity(cameraEntity);
		transform.Transform.GetMatrix().SetTranslation({ 2.0f, 1.0f, -3.0f });
		transform.Transform.Rotate({ 0.0f, UMath::DegToRad(35.0f), 0.0f });
		transform.Transform.Translate(SVector::Right * 0.25f);

		SCameraComponent& camera = AddCameraComponentToEntity(cameraEntity);
		camera.ProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), (16.0f / 9.0f), 0.1f, 1000.0f);
		camera.ViewMatrix = SMatrix::LookAtLH(SVector::Zero, SVector::Forward, SVector::Up);

		SCameraControllerComponent& controllerComp = AddCameraControllerComponentToEntity(cameraEntity);
		controllerComp.CurrentYaw = UMath::DegToRad(-35.0f);

		//auto environmentLightEntity = CreateEntity("Environment Light");
		//AddTransformComponentToEntity(environmentLightEntity);
		//renderManager->LoadEnvironmentLightComponent("Assets/Textures/Cubemaps/CubemapTheVisit.hva", AddEnvironmentLightComponentToEntity(environmentLightEntity).get());
		//GEngine::GetWorld()->GetAssetRegistry()->Register("Assets/Textures/Cubemaps/CubemapTheVisit.hva", SAssetReferenceCounter(EComponentType::EnvironmentLightComponent, static_cast<U16>(EnvironmentLightComponents.size()) - 1, 0, 0));

		//auto directionalLightEntity = CreateEntity("Directional Light");
		//AddTransformComponentToEntity(directionalLightEntity);
		//auto directionalLight = AddDirectionalLightComponentToEntity(directionalLightEntity);
		//directionalLight->Direction = { 1.0f, 1.0f, -1.0f, 0.0f };
		//directionalLight->Color = { 212.0f / 255.0f, 175.0f / 255.0f, 55.0f / 255.0f, 0.25f };
		//directionalLight->ShadowmapView.ShadowmapViewportIndex = 0;
		//directionalLight->ShadowmapView.ShadowProjectionMatrix = SMatrix::OrthographicLH(directionalLight->ShadowViewSize.X, directionalLight->ShadowViewSize.Y, directionalLight->ShadowNearAndFarPlane.X, directionalLight->ShadowNearAndFarPlane.Y);
				
		auto directionalLightEntity = CreateEntity("Directional Light");
		auto directionalLight = DirectionalLightComponents[directionalLightEntity->ID];
		directionalLight->Direction = { 1.0f, 1.0f, -1.0f, 0.0f };
		directionalLight->Color = { 212.0f / 255.0f, 175.0f / 255.0f, 55.0f / 255.0f, 0.25f };
		directionalLight->ShadowmapView.ShadowmapViewportIndex = 0;
		directionalLight->ShadowmapView.ShadowProjectionMatrix = SMatrix::OrthographicLH(directionalLight->ShadowViewSize.X, directionalLight->ShadowViewSize.Y, directionalLight->ShadowNearAndFarPlane.X, directionalLight->ShadowNearAndFarPlane.Y);

		//auto volumetricLight = AddVolumetricLightComponentToEntity(directionalLightEntity);
		//volumetricLight->IsActive = true;

		//// === Point light ===
		//auto pointLightEntity = CreateEntity("Point Light");

		//auto pointLightTransform = AddTransformComponentToEntity(pointLightEntity);
		//pointLightTransform->Transform.GetMatrix().SetTranslation({ 1.25f, 0.35f, -1.65f });
		//
		//auto pointLightComp = AddPointLightComponentToEntity(pointLightEntity);
		//pointLightComp->ColorAndIntensity = { 0.0f, 1.0f, 1.0f, 10.0f };
		//pointLightComp->Range = 1.0f;

		//auto volumetricPointLight = AddVolumetricLightComponentToEntity(pointLightEntity);
		////volumetricPointLight->IsActive = true;

		//const SMatrix constantProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(90.0f), 1.0f, 0.001f, pointLightComp->Range);
		//const SVector4 constantPosition = pointLightTransform->Transform.GetMatrix().GetTranslation4();

		//// Forward
		//SShadowmapViewData& view1 = pointLightComp->ShadowmapViews[0];
		//view1.ShadowPosition = constantPosition;
		//view1.ShadowmapViewportIndex = 1;
		//view1.ShadowViewMatrix = SMatrix::LookAtLH(constantPosition.ToVector3(), (constantPosition + SVector4::Forward).ToVector3(), SVector::Up);
		//view1.ShadowProjectionMatrix = constantProjectionMatrix;

		//// Right
		//SShadowmapViewData& view2 = pointLightComp->ShadowmapViews[1];
		//view2.ShadowPosition = constantPosition;
		//view2.ShadowmapViewportIndex = 2;
		//view2.ShadowViewMatrix = SMatrix::LookAtLH(constantPosition.ToVector3(), (constantPosition + SVector4::Right).ToVector3(), SVector::Up);
		//view2.ShadowProjectionMatrix = constantProjectionMatrix;

		//// Backward
		//SShadowmapViewData& view3 = pointLightComp->ShadowmapViews[2];
		//view3.ShadowPosition = constantPosition;
		//view3.ShadowmapViewportIndex = 3;
		//view3.ShadowViewMatrix = SMatrix::LookAtLH(constantPosition.ToVector3(), (constantPosition + SVector4::Backward).ToVector3(), SVector::Up);
		//view3.ShadowProjectionMatrix = constantProjectionMatrix;

		//// Left
		//SShadowmapViewData& view4 = pointLightComp->ShadowmapViews[3];
		//view4.ShadowPosition = constantPosition;
		//view4.ShadowmapViewportIndex = 4;
		//view4.ShadowViewMatrix = SMatrix::LookAtLH(constantPosition.ToVector3(), (constantPosition + SVector4::Left).ToVector3(), SVector::Up);
		//view4.ShadowProjectionMatrix = constantProjectionMatrix;

		//// Up
		//SShadowmapViewData& view5 = pointLightComp->ShadowmapViews[4];
		//view5.ShadowPosition = constantPosition;
		//view5.ShadowmapViewportIndex = 5;
		//view5.ShadowViewMatrix = SMatrix::LookAtLH(constantPosition.ToVector3(), (constantPosition + SVector4::Up).ToVector3(), SVector::Backward);
		//view5.ShadowProjectionMatrix = constantProjectionMatrix;

		//// Down
		//SShadowmapViewData& view6 = pointLightComp->ShadowmapViews[5];
		//view6.ShadowPosition = constantPosition;
		//view6.ShadowmapViewportIndex = 6;
		//view6.ShadowViewMatrix = SMatrix::LookAtLH(constantPosition.ToVector3(), (constantPosition + SVector4::Down).ToVector3(), SVector::Forward);
		//view6.ShadowProjectionMatrix = constantProjectionMatrix;
		//// === !Point light ===

		//// === Spotlight ===
		//auto spotlight = CreateEntity("SpotLight");

		//auto& spotlightTransform = AddTransformComponentToEntity(spotlight)->Transform;
		//spotlightTransform.Translate({ 1.5f, 0.5f, -1.0f });

		//auto spotlightComp = AddSpotLightComponentToEntity(spotlight);
		//spotlightComp->Direction = SVector4::Forward;
		//spotlightComp->DirectionNormal1 = SVector4::Right;
		//spotlightComp->DirectionNormal2 = SVector4::Up;
		//spotlightComp->ColorAndIntensity = { 0.0f, 1.0f, 0.0f, 5.0f };
		//spotlightComp->OuterAngle = 25.0f;
		//spotlightComp->InnerAngle = 5.0f;
		//spotlightComp->Range = 3.0f;

		//auto volumetricSpotLight = AddVolumetricLightComponentToEntity(spotlight);
		////volumetricSpotLight->IsActive = true;

		//const SMatrix spotlightProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(90.0f), 1.0f, 0.001f, spotlightComp->Range);
		//const SVector4 spotlightPosition = TransformComponents.back()->Transform.GetMatrix().GetTranslation4();

		//spotlightComp->ShadowmapView.ShadowPosition = spotlightPosition;
		//spotlightComp->ShadowmapView.ShadowmapViewportIndex = 7;
		//spotlightComp->ShadowmapView.ShadowViewMatrix = SMatrix::LookAtLH(spotlightPosition.ToVector3(), (spotlightPosition + spotlightComp->Direction).ToVector3(), spotlightComp->DirectionNormal2.ToVector3());
		//spotlightComp->ShadowmapView.ShadowProjectionMatrix = spotlightProjection;
		//// === !Spotlight ===

		//// === Decal ===
		//auto decal = CreateEntity("Decal");

		//auto& decalTransform = AddTransformComponentToEntity(decal)->Transform;
		//decalTransform.GetMatrix().SetTranslation({ 0.45f, 1.60f, 0.85f });

		//auto decalComp = AddDecalComponentToEntity(decal);

		//renderManager->LoadDecalComponent({"Assets/Textures/T_noscare_AL_c.hva", "Assets/Textures/T_noscare_AL_m.hva", "Assets/Textures/T_noscare_AL_n.hva"}, decalComp.get());
		//decalComp->ShouldRenderAlbedo = true;
		//decalComp->ShouldRenderMaterial = true;
		//decalComp->ShouldRenderNormal = true;
		//// === !Decal ===

		const std::string modelPath1 = "Assets/Tests/En_P_PendulumClock.hva";
		const std::vector<std::string> materialNames1 = { "Assets/Materials/M_PendulumClock.hva", "Assets/Materials/M_Checkboard_128x128.hva" };
		const std::string modelPath2 = "Assets/Tests/En_P_Bed.hva";
		const std::vector<std::string> materialNames2 = { "Assets/Materials/M_Bed.hva", "Assets/Materials/M_Bedsheet.hva" };
		const std::string modelPath3 = "Assets/Tests/Quad.hva";
		const std::vector<std::string> materialNames3 = { "Assets/Materials/M_Quad.hva" };
		const std::string modelPath4 = "Assets/Tests/En_P_WallLamp.hva";
		const std::vector<std::string> materialNames4 = { "Assets/Materials/M_Quad.hva", "Assets/Materials/M_Emissive.hva", "Assets/Materials/M_Headlamp.hva" };

		// === Pendulum ===
		auto pendulum = CreateEntity("Clock");

		//auto& transform1 = AddTransformComponentToEntity(pendulum)->Transform;
		auto& transform1 = TransformComponents[pendulum->ID]->Transform;
		transform1.GetMatrix().SetTranslation({1.75f, 0.0f, 0.25f});

		//renderManager->LoadStaticMeshComponent(modelPath1, AddStaticMeshComponentToEntity(pendulum).get());
		//renderManager->LoadMaterialComponent(materialNames1, AddMaterialComponentToEntity(pendulum).get());
		renderManager->LoadStaticMeshComponent(modelPath1, StaticMeshComponents[pendulum->ID]);
		renderManager->LoadMaterialComponent(materialNames1, MaterialComponents[pendulum->ID]);

		GEngine::GetWorld()->GetAssetRegistry()->Register(modelPath1, SAssetReferenceCounter(EComponentType::StaticMeshComponent, static_cast<U16>(StaticMeshComponents.size()) - 1, 0, 0));
		GEngine::GetWorld()->GetAssetRegistry()->Register(materialNames1, SAssetReferenceCounter(EComponentType::MaterialComponent, static_cast<U16>(MaterialComponents.size()) - 1, 0, 0));

		// === !Pendulum ===

		// === Bed ===
		//auto bed = CreateEntity("Bed");

		//auto& transform2 = AddTransformComponentToEntity(bed)->Transform;
		//transform2.GetMatrix().SetTranslation({ 0.25f, 0.0f, 0.25f });

		//renderManager->LoadStaticMeshComponent(modelPath2, AddStaticMeshComponentToEntity(bed).get());
		//renderManager->LoadMaterialComponent(materialNames2, AddMaterialComponentToEntity(bed).get());

		//GEngine::GetWorld()->GetAssetRegistry()->Register(modelPath2, SAssetReferenceCounter(EComponentType::StaticMeshComponent, static_cast<U16>(StaticMeshComponents.size()) - 1, 0, 0));
		//GEngine::GetWorld()->GetAssetRegistry()->Register(materialNames2, SAssetReferenceCounter(EComponentType::MaterialComponent, static_cast<U16>(MaterialComponents.size()) - 1, 0, 0));
		// === !Bed ===

		// === Lamp ===
		//auto lamp = CreateEntity("Lamp");

		//auto lampTransform = AddTransformComponentToEntity(lamp);
		//auto& transform4 = lampTransform->Transform;
		//transform4.GetMatrix().SetTranslation({ -1.0f, 1.4f, -0.75f });
		//transform4.Rotate({ 0.0f, UMath::DegToRad(90.0f), 0.0f });

		//renderManager->LoadStaticMeshComponent(modelPath4, AddStaticMeshComponentToEntity(lamp).get());
		//renderManager->LoadMaterialComponent(materialNames4, AddMaterialComponentToEntity(lamp).get());

		//GEngine::GetWorld()->GetAssetRegistry()->Register(modelPath4, SAssetReferenceCounter(EComponentType::StaticMeshComponent, static_cast<U16>(StaticMeshComponents.size()) - 1, 0, 0));
		//GEngine::GetWorld()->GetAssetRegistry()->Register(materialNames4, SAssetReferenceCounter(EComponentType::MaterialComponent, static_cast<U16>(MaterialComponents.size()) - 1, 0, 0));
		// === !Lamp ===

		// === TestSaveEntity ===
		//auto testSaveEntity = CreateEntity("TestSaveEntity");

		//auto testSaveEntityTransform = AddTransformComponentToEntity(testSaveEntity);
		//auto& testSaveEntityTransform4 = testSaveEntityTransform->Transform;
		//testSaveEntityTransform4.GetMatrix().SetTranslation({ -1.0f, 1.4f, -0.75f });
		//testSaveEntityTransform4.Rotate({ 0.0f, UMath::DegToRad(90.0f), 0.0f });
		//testSaveEntityTransform4.Scale({ 1.1f, 2.0f, 0.9f });
		// === !TestSaveEntity ===

		//// === Floor ===
		//std::vector<SVector> translations;
		//translations.emplace_back(-1.0f, 0.0f, -2.0f);
		//translations.emplace_back(0.0f, 0.0f, -2.0f);
		//translations.emplace_back(1.0f, 0.0f, -2.0f);
		//translations.emplace_back(2.0f, 0.0f, -2.0f);
		//translations.emplace_back(-1.0f, 0.0f, -1.0f);
		//translations.emplace_back(0.0f, 0.0f, -1.0f);
		//translations.emplace_back(1.0f, 0.0f, -1.0f);
		//translations.emplace_back(2.0f, 0.0f, -1.0f);
		//translations.emplace_back(-1.0f, 0.0f, 0.0f);
		//translations.emplace_back(0.0f, 0.0f, 0.0f);
		//translations.emplace_back(1.0f, 0.0f, 0.0f);
		//translations.emplace_back(2.0f, 0.0f, 0.0f);

		//for (U8 i = 0; i < 12; ++i)
		//{
		//	auto floor = CreateEntity("Floor");

		//	auto& transform3 = AddTransformComponentToEntity(floor)->Transform;
		//	transform3.GetMatrix().SetTranslation(translations[i]);
		//	transform3.GetMatrix().SetRotation(SMatrix::CreateRotationAroundZ(UMath::DegToRad(-90.0f)));

		//	renderManager->LoadStaticMeshComponent(modelPath3, AddStaticMeshComponentToEntity(floor).get());
		//	renderManager->LoadMaterialComponent(materialNames3, AddMaterialComponentToEntity(floor).get());
		//}
		//// === !Floor ===

		//// === Wall ===
		//translations.clear();
		//translations.emplace_back(-1.0f, 0.5f, 0.5f);
		//translations.emplace_back(0.0f, 0.5f, 0.5f);
		//translations.emplace_back(1.0f, 0.5f, 0.5f);
		//translations.emplace_back(2.0f, 0.5f, 0.5f);
		//translations.emplace_back(-1.0f, 1.5f, 0.5f);
		//translations.emplace_back(0.0f, 1.5f, 0.5f);
		//translations.emplace_back(1.0f, 1.5f, 0.5f);
		//translations.emplace_back(2.0f, 1.5f, 0.5f);
		//translations.emplace_back(-1.0f, 2.5f, 0.5f);
		//translations.emplace_back(0.0f, 2.5f, 0.5f);
		//translations.emplace_back(1.0f, 2.5f, 0.5f);
		//translations.emplace_back(2.0f, 2.5f, 0.5f);

		//for (U8 i = 0; i < 12; ++i)
		//{
		//	auto floor = CreateEntity("Wall");

		//	auto& transform3 = AddTransformComponentToEntity(floor)->Transform;
		//	transform3.GetMatrix().SetTranslation(translations[i]);
		//	transform3.GetMatrix().SetRotation(SMatrix::CreateRotationAroundZ(UMath::DegToRad(-90.0f)) * SMatrix::CreateRotationAroundX(UMath::DegToRad(-90.0f)));

		//	renderManager->LoadStaticMeshComponent(modelPath3, AddStaticMeshComponentToEntity(floor).get());
		//	renderManager->LoadMaterialComponent(materialNames3, AddMaterialComponentToEntity(floor).get());
		//}
		//// === !Wall ===

		//// === Other Wall ===
		//translations.clear();
		//translations.emplace_back(-1.0f, 0.5f, -2.5f);
		//translations.emplace_back(-1.0f, 0.5f, -1.5f);
		//translations.emplace_back(-1.0f, 0.5f, -0.5f);
		//translations.emplace_back(-1.0f, 1.5f, -2.5f);
		//translations.emplace_back(-1.0f, 1.5f, -1.5f);
		//translations.emplace_back(-1.0f, 1.5f, -0.5f);
		//translations.emplace_back(-1.0f, 2.5f, -2.5f);
		//translations.emplace_back(-1.0f, 2.5f, -1.5f);
		//translations.emplace_back(-1.0f, 2.5f, -0.5f);

		//for (U8 i = 0; i < 9; ++i)
		//{
		//	auto floor = CreateEntity("Wall");

		//	auto& transform3 = AddTransformComponentToEntity(floor)->Transform;
		//	transform3.GetMatrix().SetTranslation(translations[i]);
		//	transform3.GetMatrix().SetRotation(SMatrix::CreateRotationAroundZ(UMath::DegToRad(-90.0f)) * SMatrix::CreateRotationAroundX(UMath::DegToRad(-90.0f)) * SMatrix::CreateRotationAroundY(UMath::DegToRad(-90.0f)));
		//	
		//	renderManager->LoadStaticMeshComponent(modelPath3, AddStaticMeshComponentToEntity(floor).get());
		//	renderManager->LoadMaterialComponent(materialNames3, AddMaterialComponentToEntity(floor).get());
		//}
		//// === !Other Wall ===

		//bool useLifeTime = true;
		//bool ignoreDepth = true;
		//F32 durationSeconds = 60.0f;
		//F32 thickness = Debug::UDebugShapeSystem::ThicknessMaximum;
		//Debug::UDebugShapeSystem::AddLine(SVector(0.0f, 0.0f, 0.0f), SVector(1.0f, 0.0f, 0.0f), Color::Red, durationSeconds, useLifeTime, thickness, ignoreDepth);
		//Debug::UDebugShapeSystem::AddLine(SVector(0.0f, 0.0f, 0.0f), SVector(0.0f, 1.0f, 0.0f), Color::Green, durationSeconds, useLifeTime, thickness, ignoreDepth);
		//Debug::UDebugShapeSystem::AddLine(SVector(0.0f, 0.0f, 0.0f), SVector(0.0f, 0.0f, 1.0f), Color::Blue, durationSeconds, useLifeTime, thickness,  ignoreDepth);

		//ignoreDepth = false;
		//thickness = Debug::UDebugShapeSystem::ThicknessMinimum;
		//Debug::UDebugShapeSystem::AddLine(SVector(0.0f, 0.0f, 0.0f), SVector(-1.0f, 0.0f, 0.0f), Color::Yellow, durationSeconds, useLifeTime, thickness, ignoreDepth);
		//Debug::UDebugShapeSystem::AddLine(SVector(0.0f, 0.0f, 0.0f), SVector(0.0f, -1.0f, 0.0f), Color::Teal, durationSeconds, useLifeTime, thickness, ignoreDepth);
		//Debug::UDebugShapeSystem::AddLine(SVector(0.0f, 0.0f, 0.0f), SVector(0.0f, 0.0f, -1.0f), Color::Purple, durationSeconds, useLifeTime, thickness,  ignoreDepth);

		return true;
	}

	void CScene::SaveScene(const std::string& destinationPath)
	{
		SSceneFileHeader fileHeader;
		fileHeader.SceneName = "TestScene";
		fileHeader.SceneNameLength = static_cast<U32>(fileHeader.SceneName.size());
		fileHeader.NumberOfEntities = static_cast<U32>(Entities.size());
		fileHeader.Scene = this;
		
		U32 size = fileHeader.GetSize();
		char* data = new char[size];
		
		U32 pointerPosition = 0;
		fileHeader.Serialize(data, pointerPosition);
		size = fileHeader.GetSize();
		GEngine::GetFileSystem()->Serialize(destinationPath, &data[0], size);
		delete[] data;
	}

	void CScene::LoadScene(const std::string& /*destinationPath*/)
	{
	}

	U32 CScene::GetSize() const
	{
		//U32 size = 0;

		//for (auto& entity : Entities)
		//{
		//	size += sizeof(U32);
		//	size += sizeof(char) * entity->Name.Length();

		//	size += sizeof(I64) * static_cast<size_t>(EComponentType::Count);

		//	if (entity->HasComponent(EComponentType::TransformComponent))
		//		size += sizeof(STransformComponent);

		//	if (entity->HasComponent(EComponentType::StaticMeshComponent))
		//		size += StaticMeshComponents[entity->GetComponentIndex(EComponentType::StaticMeshComponent)]->GetSize();

		//	if (entity->HasComponent(EComponentType::CameraComponent))
		//		size += sizeof(SCameraComponent);

		//	if (entity->HasComponent(EComponentType::CameraControllerComponent))
		//		size += sizeof(SCameraControllerComponent);

		//	if (entity->HasComponent(EComponentType::MaterialComponent))
		//		size += MaterialComponents[entity->GetComponentIndex(EComponentType::MaterialComponent)]->GetSize();

		//	if (entity->HasComponent(EComponentType::EnvironmentLightComponent))
		//		size += sizeof(SEnvironmentLightComponent);

		//	if (entity->HasComponent(EComponentType::DirectionalLightComponent))
		//		size += sizeof(SDirectionalLightComponent);

		//	if (entity->HasComponent(EComponentType::PointLightComponent))
		//		size += sizeof(SPointLightComponent);

		//	if (entity->HasComponent(EComponentType::SpotLightComponent))
		//		size += sizeof(SSpotLightComponent);

		//	if (entity->HasComponent(EComponentType::VolumetricLightComponent))
		//		size += sizeof(SVolumetricLightComponent);

		//	if (entity->HasComponent(EComponentType::DecalComponent))
		//		size += DecalComponents[entity->GetComponentIndex(EComponentType::DecalComponent)]->GetSize();
		//}

		//return size;
		return 0;
	}

	void CScene::Serialize(char* /*toData*/, U32& /*pointerPosition*/) const
	{
		//for (auto& entity : Entities)
		//{
		//	pointerPosition += SerializeSimple(entity->Name.Length(), toData, pointerPosition);
		//	pointerPosition += SerializeString(entity->Name.ConstChar(), toData, pointerPosition);

		//	pointerPosition += SerializeSimple(entity->GetComponentIndices(), toData, pointerPosition);

		//	if (entity->HasComponent(EComponentType::TransformComponent))
		//	{
		//		pointerPosition += TransformComponents[entity->GetComponentIndex(EComponentType::TransformComponent)]->Serialize<STransformComponent>(toData, pointerPosition);
		//	}

		//	if (entity->HasComponent(EComponentType::StaticMeshComponent))
		//	{
		//		StaticMeshComponents[entity->GetComponentIndex(EComponentType::StaticMeshComponent)]->Serialize(toData, pointerPosition);
		//	}

		//	if (entity->HasComponent(EComponentType::CameraComponent))
		//	{
		//		pointerPosition += CameraComponents[entity->GetComponentIndex(EComponentType::CameraComponent)]->Serialize<SCameraComponent>(toData, pointerPosition);
		//	}

		//	if (entity->HasComponent(EComponentType::CameraControllerComponent))
		//	{
		//		pointerPosition += CameraControllerComponents[entity->GetComponentIndex(EComponentType::CameraControllerComponent)]->Serialize<SCameraControllerComponent>(toData, pointerPosition);
		//	}
		//	
		//	if (entity->HasComponent(EComponentType::MaterialComponent))
		//	{
		//		MaterialComponents[entity->GetComponentIndex(EComponentType::MaterialComponent)]->Serialize(toData, pointerPosition);
		//	}

		//	if (entity->HasComponent(EComponentType::EnvironmentLightComponent))
		//	{
		//		pointerPosition += EnvironmentLightComponents[entity->GetComponentIndex(EComponentType::EnvironmentLightComponent)]->Serialize<SEnvironmentLightComponent>(toData, pointerPosition);
		//	}

		//	if (entity->HasComponent(EComponentType::DirectionalLightComponent))
		//	{
		//		pointerPosition += DirectionalLightComponents[entity->GetComponentIndex(EComponentType::DirectionalLightComponent)]->Serialize<SDirectionalLightComponent>(toData, pointerPosition);
		//	}

		//	if (entity->HasComponent(EComponentType::PointLightComponent))
		//	{
		//		pointerPosition += PointLightComponents[entity->GetComponentIndex(EComponentType::PointLightComponent)]->Serialize<SPointLightComponent>(toData, pointerPosition);
		//	}

		//	if (entity->HasComponent(EComponentType::SpotLightComponent))
		//	{
		//		pointerPosition += SpotLightComponents[entity->GetComponentIndex(EComponentType::SpotLightComponent)]->Serialize<SSpotLightComponent>(toData, pointerPosition);
		//	}

		//	if (entity->HasComponent(EComponentType::VolumetricLightComponent))
		//	{
		//		pointerPosition += VolumetricLightComponents[entity->GetComponentIndex(EComponentType::VolumetricLightComponent)]->Serialize<SVolumetricLightComponent>(toData, pointerPosition);
		//	}
		//		
		//	if (entity->HasComponent(EComponentType::DecalComponent))
		//	{
		//		DecalComponents[entity->GetComponentIndex(EComponentType::DecalComponent)]->Serialize(toData, pointerPosition);
		//	}
		//}
	}

	void CScene::Deserialize(const char* /*fromData*/, U32& /*pointerPosition*/)
	{
		//U32 numberOfEntities = 0;
		//pointerPosition += DeserializeSimple(numberOfEntities, fromData, pointerPosition);

		//for (U32 i = 0; i < numberOfEntities; i++)
		//{
		//	U32 nameLength = 0;
		//	pointerPosition += DeserializeSimple(nameLength, fromData, pointerPosition);
		//	std::string name = "";
		//	pointerPosition += DeserializeString(name, fromData, nameLength, pointerPosition);

		//	auto entity = CreateEntity(name);

		//	std::array<I64, static_cast<size_t>(EComponentType::Count)> indices;
		//	pointerPosition += DeserializeSimple(indices, fromData, pointerPosition);

		//	//TODO.NR: Component Entity Data is overwritten when Deserialized, figure it out again

		//	if (indices[static_cast<U32>(EComponentType::TransformComponent)] >= 0)
		//	{
		//		AddTransformComponentToEntity(entity);
		//		pointerPosition += TransformComponents.back()->Deserialize<STransformComponent>(fromData, pointerPosition);
		//	}

		//	if (indices[static_cast<U32>(EComponentType::StaticMeshComponent)] >= 0)
		//	{
		//		AddStaticMeshComponentToEntity(entity);
		//		StaticMeshComponents.back()->Deserialize(fromData, pointerPosition);
		//	}

		//	if (indices[static_cast<U32>(EComponentType::CameraComponent)] >= 0)
		//	{
		//		AddCameraComponentToEntity(entity);
		//		pointerPosition += CameraComponents.back()->Deserialize<SCameraComponent>(fromData, pointerPosition);
		//	}

		//	if (indices[static_cast<U32>(EComponentType::CameraControllerComponent)] >= 0)
		//	{
		//		AddCameraControllerComponentToEntity(entity);
		//		pointerPosition += CameraControllerComponents.back()->Deserialize<SCameraControllerComponent>(fromData, pointerPosition);
		//	}

		//	if (indices[static_cast<U32>(EComponentType::MaterialComponent)] >= 0)
		//	{
		//		AddMaterialComponentToEntity(entity);
		//		MaterialComponents.back()->Deserialize(fromData, pointerPosition);
		//	}

		//	if (indices[static_cast<U32>(EComponentType::EnvironmentLightComponent)] >= 0)
		//	{
		//		AddEnvironmentLightComponentToEntity(entity);
		//		pointerPosition += EnvironmentLightComponents.back()->Deserialize<SEnvironmentLightComponent>(fromData, pointerPosition);
		//	}

		//	if (indices[static_cast<U32>(EComponentType::DirectionalLightComponent)] >= 0)
		//	{
		//		AddDirectionalLightComponentToEntity(entity);
		//		pointerPosition += DirectionalLightComponents.back()->Deserialize<SDirectionalLightComponent>(fromData, pointerPosition);
		//	}

		//	if (indices[static_cast<U32>(EComponentType::PointLightComponent)] >= 0)
		//	{
		//		AddPointLightComponentToEntity(entity);
		//		pointerPosition += PointLightComponents.back()->Deserialize<SPointLightComponent>(fromData, pointerPosition);
		//	}

		//	if (indices[static_cast<U32>(EComponentType::SpotLightComponent)] >= 0)
		//	{
		//		AddSpotLightComponentToEntity(entity);
		//		pointerPosition += SpotLightComponents.back()->Deserialize<SSpotLightComponent>(fromData, pointerPosition);
		//	}

		//	if (indices[static_cast<U32>(EComponentType::VolumetricLightComponent)] >= 0)
		//	{
		//		AddVolumetricLightComponentToEntity(entity);
		//		pointerPosition += VolumetricLightComponents.back()->Deserialize<SVolumetricLightComponent>(fromData, pointerPosition);
		//	}

		//	if (indices[static_cast<U32>(EComponentType::DecalComponent)] >= 0)
		//	{
		//		AddDecalComponentToEntity(entity);
		//		DecalComponents.back()->Deserialize(fromData, pointerPosition);
		//	}
		//}
	}

	const std::vector<SEntity>& CScene::GetEntities() 
	{
		return Entities; 
	}

	bool CScene::TryGetNewEntity(SEntity& outEntity)
	{
		// TODO: Safeguard
		if (FirstUnusedEntityIndex >= ENTITY_LIMIT)
		{
			HV_LOG_ERROR("Reached ENTITY_LIMIT.");
			return false;
		}

		outEntity = Entities[FirstUnusedEntityIndex];
		outEntity.GUID = gEntityGUID++;

		EntityVectorIndices.emplace(outEntity.GUID, FirstUnusedEntityIndex);
		FirstUnusedEntityIndex++;

		return true;
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

		I64 entityIndex = EntityVectorIndices[entity.GUID];

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
		UpdateComponentVector(DebugShapeComponents, entityIndex);
		UpdateComponentVector(MetaDataComponents, entityIndex);

		Entities[FirstUnusedEntityIndex - 1].GUID = 0;
		FirstUnusedEntityIndex--;

		return true;
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
	COMPONENT_REMOVER_DEFINITION(DebugShapeComponent)
	COMPONENT_REMOVER_DEFINITION(MetaDataComponent)
}
