// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "RenderSystem.h"
#include "Engine.h"
#include "Scene/World.h"
#include "Scene/Scene.h"
#include "ECS/ECSInclude.h"
#include "Graphics/DefaultRenderPassData.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderGraph.h"
#include "Graphics/RenderResourceRegistry.h"
#include "ECS/ComponentAlgo.h"
#include "Input/Input.h"
#include "Assets/AssetRegistry.h"

#include <MathTypes/Sphere.h>
#include <MathTypes/Frustum.h>

namespace Havtorn
{
	CRenderSystem::CRenderSystem(CRenderManager* renderManager, CWorld* world)
		: ISystem()
		, RenderManager(renderManager)
		, World(world)
	{
		StateManager = &RenderManager->GetRenderStateManager();
		//RenderResourceRegistry = &RenderManager->GetRenderResourceRegistry();
		AssetRegistry = GEngine::GetAssetRegistry();
	}

	void CRenderSystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		const bool isInPlayingPlayState = World->GetWorldPlayState() == EWorldPlayState::Playing;
		
		// Render View Pre-Pass
		// TODO.NW: Unify?
		std::vector<SCameraData> activeCameras = {};
		for (Ptr<CScene>& scene : scenes)
		{
			std::vector<SCameraComponent*> cameraComponents = scene->GetComponents<SCameraComponent>();
			
			for (SCameraComponent* cameraComponent : cameraComponents)
			{
				if (cameraComponent->IsActive)
				{
					RenderManager->RequestRenderView(cameraComponent->Owner.GUID);
					
					SCameraData data;
					data.TransformComponent = scene->GetComponent<STransformComponent>(cameraComponent->Owner);
					data.CameraComponent = cameraComponent;
					activeCameras.push_back(data);
				}
				else
				{
					RenderManager->UnrequestRenderView(cameraComponent->Owner.GUID);
				}
			}
		}

		for (const SCameraData& cameraData : activeCameras)
		{
			SEntity& cameraEntity = cameraData.CameraComponent->Owner;	
			SRenderView* renderView = RenderManager->GetRenderView(cameraEntity.GUID);

			for (Ptr<CScene>& scene : scenes)
			{
				ProcessRenderView(renderView, scene.get(), { cameraData.TransformComponent->Transform.GetMatrix(), cameraData.CameraComponent->ProjectionMatrix }, true);
			}

			// TODO.NW: Skip editor data if Game view mode is active as well
			AddDefaultPasses(renderView, !isInPlayingPlayState && cameraEntity == World->GetMainCamera());
		}
	}

	SSceneLightCollection CRenderSystem::GatherActiveLights(CScene* scene, const bool doCulling, const SFrustum& viewFrustum) const
	{
		// TODO.NW: Consider implementing cascaded shadow maps
		// https://learnopengl.com/Guest-Articles/2021/CSM
		// https://developer.download.nvidia.com/SDK/10.5/opengl/src/cascaded_shadow_maps/doc/cascaded_shadow_maps.pdf

		std::vector<SDirectionalLightComponent*> directionalLights;
		std::vector<SFrustum> directionalLightCullGeometry;
		for (SDirectionalLightComponent* directionalLightComp : scene->GetComponents<SDirectionalLightComponent>())
		{
			if (!SComponent::IsValid(directionalLightComp) || !directionalLightComp->IsActive)
				continue;

			// Additional Validation
			{
				const SEntity& closestEnvironmentLightEntity = UComponentAlgo::GetClosestEntity3D(directionalLightComp->Owner, scene->GetComponents<SEnvironmentLightComponent>(), scene);
				const SEnvironmentLightComponent* environmentLightComp = scene->GetComponent<SEnvironmentLightComponent>(closestEnvironmentLightEntity);
				if (!SComponent::IsValid(environmentLightComp))
					continue;

				// Will be unrequested when the environment light component unloads
				const STextureCubeAsset* cubemapTextureAsset = AssetRegistry->RequestAssetData<STextureCubeAsset>(environmentLightComp->AssetReference, environmentLightComp->Owner.GUID);
				if (cubemapTextureAsset == nullptr)
					continue;
			}

			if (!doCulling)
			{
				directionalLights.push_back(directionalLightComp);
				continue;
			}

			// TODO.NW: We can do a cheaper test of orthographic frustum (light frustum) vs perspective frustum (camera frustum)
			// by taking the camera frustum vertices into the light frustum NDC space (through view-perspective matrix multiplication)
			// and thereby doing way fewer axis tests, which might be more performant. Implement a function for this.
			const SFrustum lightFrustum = SFrustum(directionalLightComp->ShadowmapView.ShadowViewMatrix, directionalLightComp->ShadowmapView.ShadowProjectionMatrix);
			if (viewFrustum.Intersects(lightFrustum))
			{
				directionalLights.push_back(directionalLightComp);
				directionalLightCullGeometry.push_back(lightFrustum);
			}
		}

		std::vector<SPointLightComponent*> pointLights;
		std::vector<SSphere> pointLightCullGeometry;
		for (SPointLightComponent* pointLightComp : scene->GetComponents<SPointLightComponent>())
		{
			if (!SComponent::IsValid(pointLightComp) || !pointLightComp->IsActive)
				continue;
			
			// Additional Validation
			{
				const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(pointLightComp);
				if (!SComponent::IsValid(transformComp))
					continue;
			}

			if (!doCulling)
			{
				pointLights.push_back(pointLightComp);
				continue;
			}

			const STransformComponent* pointLightTransform = scene->GetComponent<STransformComponent>(pointLightComp);
			if (!SComponent::IsValid(pointLightTransform))
				continue;

			const SSphere lightBoundingSphere = SSphere(pointLightTransform->Transform.GetMatrix().GetTranslation(), pointLightComp->Range);
			if (viewFrustum.Intersects(lightBoundingSphere))
			{
				pointLights.push_back(pointLightComp);
				pointLightCullGeometry.push_back(lightBoundingSphere);
			}
		}

		std::vector<SSpotLightComponent*> spotlights;
		std::vector<SFrustum> spotlightCullGeometry;
		for (SSpotLightComponent* spotLightComp : scene->GetComponents<SSpotLightComponent>())
		{
			if (!SComponent::IsValid(spotLightComp) || !spotLightComp->IsActive)
				continue;

			// Additional Validation
			{
				const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(spotLightComp);
				if (!SComponent::IsValid(transformComp))
					continue;
			}

			if (!doCulling)
			{
				spotlights.push_back(spotLightComp);
				continue;
			}

			// TODO.NW: Consider a bounding sphere instead
			const SFrustum lightFrustum = SFrustum(spotLightComp->ShadowmapView.ShadowViewMatrix, spotLightComp->ShadowmapView.ShadowProjectionMatrix);
			if (viewFrustum.Intersects(lightFrustum))
			{
				spotlights.push_back(spotLightComp);
				spotlightCullGeometry.push_back(lightFrustum);
			}
		}

		return SSceneLightCollection
		{
			.DirectionalLights = directionalLights,
			.PointLights = pointLights,
			.Spotlights = spotlights,

			.DirectionalLightCullGeometry = directionalLightCullGeometry,
			.PointLightCullGeometry = pointLightCullGeometry,
			.SpotlightCullGeometry = spotlightCullGeometry
		};
	}

	void CRenderSystem::ProcessRenderView(SRenderView* renderView, CScene* scene, const SViewData& viewData, const bool doCulling) const
	{
		if (renderView == nullptr || scene == nullptr)
			return;

		renderView->ViewMatrix = viewData.ViewMatrix;
		renderView->ProjectionMatrix = viewData.ProjectionMatrix;

		const SVector2<F32> shadowAtlasResolution = RenderManager->GetShadowAtlasResolution();
		const F32 shadowTestTolerance = RenderManager->GetShadowTestTolerance();

		const SFrustum cameraFrustum = doCulling ? SFrustum(viewData.ViewMatrix, viewData.ProjectionMatrix) : SFrustum(SMatrix::Identity, SMatrix::Identity);
		const SSceneLightCollection lights = GatherActiveLights(scene, doCulling, cameraFrustum); 
		const SEntity& editorRenderExemptEntity = GEngine::GetWorld()->GetEditorRenderExemptEntity();

		for (const SDirectionalLightComponent* directionalLightComp : lights.DirectionalLights)
		{
			ProcessDirectionalLight(renderView, scene, directionalLightComp, shadowAtlasResolution, shadowTestTolerance);
		}

		for (const SPointLightComponent* pointLightComp : lights.PointLights)
		{
			ProcessPointLight(renderView, scene, pointLightComp, shadowAtlasResolution, shadowTestTolerance);
		}

		for (const SSpotLightComponent* spotlightComp : lights.Spotlights)
		{
			ProcessSpotlight(renderView, scene, spotlightComp, shadowAtlasResolution, shadowTestTolerance);
		}

		for (const SStaticMeshComponent* staticMeshComponent : scene->GetComponents<SStaticMeshComponent>())
		{
			ProcessStaticMesh(renderView, scene, staticMeshComponent, doCulling, cameraFrustum, lights, editorRenderExemptEntity);
		}

		for (const SSkeletalMeshComponent* skeletalMeshComponent : scene->GetComponents<SSkeletalMeshComponent>())
		{
			ProcessSkeletalMesh(renderView, scene, skeletalMeshComponent, doCulling, cameraFrustum, lights, editorRenderExemptEntity);
		}

		for (const SDecalComponent* decalComponent : scene->GetComponents<SDecalComponent>())
		{
			ProcessDecal(renderView, scene, decalComponent, doCulling, cameraFrustum);
		}

		for (const SSpriteComponent* spriteComp : scene->GetComponents<SSpriteComponent>())
		{
			ProcessSprite(renderView, scene, spriteComp, doCulling, cameraFrustum);
		}

		for (const SUICanvasComponent* uiCanvasComp : scene->GetComponents<SUICanvasComponent>())
		{
			ProcessUICanvas(renderView, scene, uiCanvasComp);
		}

		// TODO.NW: Do we need to find just one closest environmentlight from all of the scenes?
		{
			const SEntity& closestEnvironmentLightEntity = UComponentAlgo::GetClosestEntity3D(viewData.ViewMatrix, scene->GetComponents<SEnvironmentLightComponent>(), scene);
			if (closestEnvironmentLightEntity.IsValid())
			{
				const SEnvironmentLightComponent* environmentLightComp = scene->GetComponent<SEnvironmentLightComponent>(closestEnvironmentLightEntity);
				if (SComponent::IsValid(environmentLightComp))
				{
					STextureCubeAsset* asset = AssetRegistry->RequestAssetData<STextureCubeAsset>(environmentLightComp->AssetReference, environmentLightComp->Owner.GUID);
					if (asset != nullptr)
					{
						renderView->SkyboxCubemapTextureHandle = renderView->RenderResourceRegistry->DeclareRenderTexture("Skybox Cubemap", &asset->RenderTexture);
					}
				}
			}
		}
	}

	void CRenderSystem::ProcessDirectionalLight(SRenderView* renderView, CScene* scene, const SDirectionalLightComponent* lightComponent, const SVector2<F32>& shadowAtlasResolution, const F32 shadowTestTolerance) const
	{
		// Validation happened in GatherActiveLights
		const SEntity& closestEnvironmentLightEntity = UComponentAlgo::GetClosestEntity3D(lightComponent->Owner, scene->GetComponents<SEnvironmentLightComponent>(), scene);
		const SEnvironmentLightComponent* environmentLightComp = scene->GetComponent<SEnvironmentLightComponent>(closestEnvironmentLightEntity);
		STextureCubeAsset* cubemapTextureAsset = AssetRegistry->RequestAssetData<STextureCubeAsset>(environmentLightComp->AssetReference, environmentLightComp->Owner.GUID);

		const SVolumetricLightComponent* volumetricLightComp = scene->GetComponent<SVolumetricLightComponent>(lightComponent);
		const bool hasActiveVolumetricLightComp = SComponent::IsValid(volumetricLightComp) && volumetricLightComp->IsActive;

		const auto& [bufferData, viewportIndices] = ProcessShadowmapBufferData({ lightComponent->ShadowmapView }, shadowAtlasResolution, shadowTestTolerance);

		renderView->DrawData.DirectionalLightDrawData.emplace_back(SDirectionalLightDrawData
			{
				// Filled by mesh processing
				.ShadowCasterData = {},
				// Negate the direction going into the shaders
				.LightBufferData = { .ToDirectionalLight = -lightComponent->Direction, .DirectionalLightColor = lightComponent->Color },
				.ShadowmapBufferData = bufferData[0],
				.VolumetricBufferData = ProcessVolumetricLightData(volumetricLightComp),
				.CubemapTextureHandle = renderView->RenderResourceRegistry->DeclareRenderTexture(cubemapTextureAsset->Name.c_str(), &cubemapTextureAsset->RenderTexture),
				.ShadowmapViewportIndex = viewportIndices[0],
				.RenderVolumetrics = hasActiveVolumetricLightComp
			});
	}

	void CRenderSystem::ProcessPointLight(SRenderView* renderView, CScene* scene, const SPointLightComponent* lightComponent, const SVector2<F32>& shadowAtlasResolution, const F32 shadowTestTolerance) const
	{
		const SVolumetricLightComponent* volumetricLightComp = scene->GetComponent<SVolumetricLightComponent>(lightComponent);
		const bool hasActiveVolumetricLightComp = SComponent::IsValid(volumetricLightComp) && volumetricLightComp->IsActive;

		const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(lightComponent);
		const SMatrix transformMatrix = transformComp->Transform.GetMatrix();
		const SVector translation = transformMatrix.GetTranslation();

		const auto& [bufferData, viewportIndices] = ProcessShadowmapBufferData({ lightComponent->ShadowmapViews.begin(), lightComponent->ShadowmapViews.end() }, shadowAtlasResolution, shadowTestTolerance);
		std::array<SShadowmapBufferData, 6> bufferDataArray;
		std::copy(bufferData.begin(), bufferData.end(), bufferDataArray.begin());
		
		renderView->DrawData.PointLightDrawData.emplace_back(SPointLightDrawData
			{
				// Filled by mesh processing
				.ShadowCasterData = {},
				.LightBufferData = {.ToWorldFromObject = transformComp->Transform.GetMatrix(), .ColorAndIntensity = lightComponent->ColorAndIntensity, .PositionAndRange = SVector4(translation, lightComponent->Range)},
				.ShadowmapBufferData = bufferDataArray,
				.VolumetricBufferData = ProcessVolumetricLightData(volumetricLightComp),
				.ShadowmapViewportIndex = viewportIndices[0],
				.RenderVolumetrics = hasActiveVolumetricLightComp
			});
	}

	void CRenderSystem::ProcessSpotlight(SRenderView* renderView, CScene* scene, const SSpotLightComponent* lightComponent, const SVector2<F32>& shadowAtlasResolution, const F32 shadowTestTolerance) const
	{
		const SVolumetricLightComponent* volumetricLightComp = scene->GetComponent<SVolumetricLightComponent>(lightComponent);
		const bool hasActiveVolumetricLightComp = SComponent::IsValid(volumetricLightComp) && volumetricLightComp->IsActive;

		const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(lightComponent);
		const SMatrix transformMatrix = transformComp->Transform.GetMatrix();
		const SVector translation = transformMatrix.GetTranslation();

		const auto& [bufferData, viewportIndices] = ProcessShadowmapBufferData({ lightComponent->ShadowmapView }, shadowAtlasResolution, shadowTestTolerance);

		renderView->DrawData.SpotlightDrawData.emplace_back(SSpotlightDrawData
			{
				// Filled by mesh processing
				.ShadowCasterData = {},
				.PointLightBufferData = {.ToWorldFromObject = transformComp->Transform.GetMatrix(), .ColorAndIntensity = lightComponent->ColorAndIntensity, .PositionAndRange = SVector4(translation, lightComponent->Range)},
				.SpotlightBufferData = SSpotLightBufferData
				{
					.ColorAndIntensity = lightComponent->ColorAndIntensity,
					.PositionAndRange = SVector4(translation, lightComponent->Range),
					.Direction = lightComponent->Direction,
					.DirectionNormal1 = lightComponent->DirectionNormal1,
					.DirectionNormal2 = lightComponent->DirectionNormal2,
					.OuterAngle = lightComponent->OuterAngle,
					.InnerAngle = lightComponent->InnerAngle
				},
				.ShadowmapBufferData = bufferData[0],
				.VolumetricBufferData = ProcessVolumetricLightData(volumetricLightComp),
				.ShadowmapViewportIndex = viewportIndices[0],
				.RenderVolumetrics = hasActiveVolumetricLightComp
			});
	}

	void CRenderSystem::ProcessStaticMesh(SRenderView* renderView, CScene* scene, const SStaticMeshComponent* meshComponent, const bool doCulling, const SFrustum& viewFrustum, const SSceneLightCollection& sceneLights, const SEntity& editorRenderExemptEntity) const
	{
		const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(meshComponent);
		SMaterialComponent* materialComp = scene->GetComponent<SMaterialComponent>(meshComponent);

		if (!SComponent::IsValid(meshComponent) || !SComponent::IsValid(transformComp) || !SComponent::IsValid(materialComp))
			return;

		SStaticMeshAsset* asset = AssetRegistry->RequestAssetData<SStaticMeshAsset>(meshComponent->AssetReference, meshComponent->Owner.GUID);
		if (asset == nullptr)
			return;

		const bool deferCommand = meshComponent->Owner == editorRenderExemptEntity;
		U32 drawID = meshComponent->AssetReference.UID + UComponentAlgo::CalculateAggregateMaterialID(materialComp);
		if (deferCommand)
			drawID += 1000;

		// TODO.NW: Consider making a utility function along the lines of GetBoundingSphere(asset, transformMatrix);
		const SMatrix& transformMatrix = transformComp->Transform.GetMatrix();
		// NW: This seems to give the right results, notice the Hadamard multiplication between bounds vector and scale
		const F32 radius = ((asset->BoundsMax - asset->BoundsMin) * transformMatrix.GetScale()).Size();
		const SSphere meshBoundingSphere = SSphere((SVector4(asset->BoundsCenter, 1.0f) * transformMatrix).ToVector3(), radius);

		// TODO.NW: Figure out where to put this
		if (materialComp->AssetReferences.size() != asset->NumberOfMaterials)
			materialComp->AssetReferences.resize(asset->NumberOfMaterials, SAssetReference("Resources/M_MeshPreview.hva"));

		std::vector<SGraphicsMaterialAsset*> materialAssets = AssetRegistry->RequestAssetData<SGraphicsMaterialAsset>(materialComp->AssetReferences, materialComp->Owner.GUID);

		if (SStaticMeshDrawData* drawData = ProcessStaticGeometry(renderView, asset->DrawCallData, materialAssets,
			sceneLights, doCulling, meshBoundingSphere, viewFrustum, transformMatrix, drawID, meshComponent->AssetReference.UID, materialComp->Owner.GUID))
		{
			drawData->Transforms.push_back(transformMatrix);
			drawData->Entities.push_back(meshComponent->Owner);
		}
	}

	void CRenderSystem::ProcessSkeletalMesh(SRenderView* renderView, CScene* scene, const SSkeletalMeshComponent* meshComponent, const bool doCulling, const SFrustum& viewFrustum, const SSceneLightCollection& /*sceneLights*/, const SEntity& editorRenderExemptEntity) const
	{
		const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(meshComponent);
		SMaterialComponent* materialComp = scene->GetComponent<SMaterialComponent>(meshComponent);

		if (!SComponent::IsValid(meshComponent) || !SComponent::IsValid(transformComp) || !SComponent::IsValid(materialComp))
			return;

		SSkeletalMeshAsset* asset = AssetRegistry->RequestAssetData<SSkeletalMeshAsset>(meshComponent->AssetReference, meshComponent->Owner.GUID);
		if (asset == nullptr)
			return;

		const bool deferCommand = meshComponent->Owner == editorRenderExemptEntity;
		U32 drawID = meshComponent->AssetReference.UID + UComponentAlgo::CalculateAggregateMaterialID(materialComp);
		if (deferCommand)
			drawID += 1000;

		// TODO.NW: Consider making a utility function along the lines of GetBoundingSphere(asset, transformMatrix);
		const SMatrix& transformMatrix = transformComp->Transform.GetMatrix();
		// NW: This seems to give the right results, notice the Hadamard multiplication between bounds vector and scale
		const F32 radius = ((asset->BoundsMax - asset->BoundsMin) * transformMatrix.GetScale()).Size();
		const SSphere meshBoundingSphere = SSphere((SVector4(asset->BoundsCenter, 1.0f) * transformMatrix).ToVector3(), radius);

		// TODO.NW: Figure out where to put this
		if (materialComp->AssetReferences.size() != asset->NumberOfMaterials)
			materialComp->AssetReferences.resize(asset->NumberOfMaterials, SAssetReference("Resources/M_MeshPreview.hva"));

		std::vector<SGraphicsMaterialAsset*> materialAssets = AssetRegistry->RequestAssetData<SGraphicsMaterialAsset>(materialComp->AssetReferences, materialComp->Owner.GUID);

		// TODO.NW: It's probably a great idea to save this in the runtime asset for use here, like the draw call data is now,
		// if we can handle the transition in the render resource registry nicely. Might be a thing to keep buffers loaded
		// in the registry itself while the asset is loaded? Maybe.
		// When do we fill the material data?
		std::vector<SPrimitiveData> primitiveDataOnly;
		std::vector<SMeshDrawData> fullDrawData;

		for (const SDrawCallData& drawCallData : asset->DrawCallData)
		{
			SPrimitiveData primitiveData;
			// TODO.NW: Need unique names for all these resources, including materials
			primitiveData.VertexBufferHandle = renderView->RenderResourceRegistry->DeclareVertexBuffer<SSkeletalMeshVertex>("VertexBuffer", &RenderManager->GetRenderStateManager().VertexBuffers[drawCallData.VertexBufferIndex]);
			primitiveData.IndexBufferHandle = renderView->RenderResourceRegistry->DeclareIndexBuffer("IndexBuffer", &RenderManager->GetRenderStateManager().IndexBuffers[drawCallData.IndexBufferIndex]);
			primitiveData.IndexCount = drawCallData.IndexCount;
			primitiveDataOnly.push_back(primitiveData);

			const SEngineGraphicsMaterial& material = materialAssets[drawCallData.MaterialIndex]->Material;

			SMeshDrawData meshDrawData;
			meshDrawData.PrimitiveData = primitiveData;
			std::tie(meshDrawData.MaterialTextureHandles, meshDrawData.MaterialBufferData) = ProcessMaterialData(renderView, material, materialComp->Owner.GUID);
			fullDrawData.push_back(meshDrawData);
		}

		// TODO.NW: Implement skel mesh shadow casting
		/*for (U64 i = 0; i < sceneLights.DirectionalLights.size(); i++)
		{
			if (!doCulling || sceneLights.DirectionalLightCullGeometry[i].Intersects(meshBoundingSphere))
			{
				std::unordered_map<U32, SSkeletalShadowCasterData>& shadowCasterData = renderView->DirectionalLightDrawData[i].ShadowCasterData;
				if (!shadowCasterData.contains(meshComponent->AssetReference.UID))
					shadowCasterData.emplace(meshComponent->AssetReference.UID, SSkeletalShadowCasterData{ .PrimitiveData = primitiveDataOnly, .Transforms = {} });

				shadowCasterData.at(meshComponent->AssetReference.UID).Transforms.push_back(transformMatrix);
			}
		}

		for (U64 i = 0; i < sceneLights.PointLights.size(); i++)
		{
			if (!doCulling || sceneLights.PointLightCullGeometry[i].Intersects(meshBoundingSphere))
			{
				std::unordered_map<U32, SSkeletalShadowCasterData>& shadowCasterData = renderView->PointLightDrawData[i].ShadowCasterData;
				if (!shadowCasterData.contains(meshComponent->AssetReference.UID))
					shadowCasterData.emplace(meshComponent->AssetReference.UID, SSkeletalShadowCasterData{ .PrimitiveData = primitiveDataOnly, .Transforms = {} });

				shadowCasterData.at(meshComponent->AssetReference.UID).Transforms.push_back(transformMatrix);
			}
		}

		for (U64 i = 0; i < sceneLights.Spotlights.size(); i++)
		{
			if (!doCulling || sceneLights.SpotlightCullGeometry[i].Intersects(meshBoundingSphere))
			{
				std::unordered_map<U32, SSkeletalShadowCasterData>& shadowCasterData = renderView->SpotlightDrawData[i].ShadowCasterData;
				if (!shadowCasterData.contains(meshComponent->AssetReference.UID))
					shadowCasterData.emplace(meshComponent->AssetReference.UID, SSkeletalShadowCasterData{ .PrimitiveData = primitiveDataOnly, .Transforms = {} });

				shadowCasterData.at(meshComponent->AssetReference.UID).Transforms.push_back(transformMatrix);
			}
		}*/

		if (doCulling && !viewFrustum.Intersects(meshBoundingSphere))
			return;

		if (!renderView->DrawData.SkeletalMeshDrawData.contains(drawID))
			renderView->DrawData.SkeletalMeshDrawData.emplace(drawID, SSkeletalMeshDrawData{ .DrawData = fullDrawData, .Transforms = {}, .Entities = {}, .Bones = std::vector<SMatrix>(asset->BindPoseBones.size(), SMatrix::Identity) });

		SSkeletalMeshDrawData& drawData = renderView->DrawData.SkeletalMeshDrawData.at(drawID);
		drawData.Transforms.push_back(transformMatrix);
		drawData.Entities.push_back(meshComponent->Owner);
		
		// TODO.NW: Still need to instance skel meshes somehow
		const SSkeletalAnimationComponent* animationComponent = scene->GetComponent<SSkeletalAnimationComponent>(meshComponent);
		if (SComponent::IsValid(animationComponent))
			drawData.Bones = animationComponent->Bones;
	}

	void CRenderSystem::ProcessDecal(SRenderView* renderView, CScene* scene, const SDecalComponent* component, const bool doCulling, const SFrustum& viewFrustum) const
	{
		const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(component);

		if (!SComponent::IsValid(component) || !SComponent::IsValid(transformComp))
			return;

		SGraphicsMaterialAsset* asset = AssetRegistry->RequestAssetData<SGraphicsMaterialAsset>(component->AssetReference, component->Owner.GUID);
		if (asset == nullptr)
			return;

		const U32 drawID = component->AssetReference.UID;
		
		// NW: These are assumed correct from the form of GeometryPrimitives::DecalProjector
		const SVector projectorObjectMin = SVector(-0.5f);
		const SVector projectorObjectMax = SVector(0.5f);
		const SVector projectorObjectCenter = SVector::Zero;

		const SMatrix& transformMatrix = transformComp->Transform.GetMatrix();
		// NW: This seems to give the right results, notice the Hadamard multiplication between bounds vector and scale
		const F32 radius = ((projectorObjectMax - projectorObjectMin) * transformMatrix.GetScale()).Size();
		const SSphere meshBoundingSphere = SSphere((SVector4(projectorObjectCenter, 1.0f) * transformMatrix).ToVector3(), radius);

		if (doCulling && !viewFrustum.Intersects(meshBoundingSphere))
			return;

		if (!renderView->DrawData.DecalDrawData.contains(drawID))
		{
			SDecalDrawData decalDrawData;
			std::tie(decalDrawData.MaterialTextureHandles, decalDrawData.MaterialBufferData) = ProcessMaterialData(renderView, asset->Material, component->Owner.GUID);
			renderView->DrawData.DecalDrawData.emplace(drawID, decalDrawData);
		}

		SDecalDrawData& drawData = renderView->DrawData.DecalDrawData.at(drawID);
		drawData.Transforms.push_back(transformMatrix);
	}

	void CRenderSystem::ProcessSprite(SRenderView* renderView, CScene* scene, const SSpriteComponent* component, const bool doCulling, const SFrustum& viewFrustum) const
	{
		if (!SComponent::IsValid(component))
			return;

		STextureAsset* asset = AssetRegistry->RequestAssetData<STextureAsset>(component->AssetReference, component->Owner.GUID);
		if (asset == nullptr)
			return;

		const U32 drawID = component->AssetReference.UID;

		const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(component);
		const STransform2DComponent* transform2DComp = scene->GetComponent<STransform2DComponent>(component);
		if (SComponent::IsValid(transformComp))
		{
			if (doCulling)
			{
				// NW: These are assumed correct from the form of GeometryPrimitives::SpriteQuad
				const SVector quadObjectMin = SVector(-0.5f, -0.5f, 0.0f);
				const SVector quadObjectMax = SVector(0.5f, 0.5f, 0.0f);
				const SVector quadObjectCenter = SVector::Zero;

				const SMatrix& transformMatrix = transformComp->Transform.GetMatrix();
				// NW: This seems to give the right results, notice the Hadamard multiplication between bounds vector and scale
				const F32 radius = ((quadObjectMax - quadObjectMin) * transformMatrix.GetScale()).Size();
				const SSphere quadBoundingSphere = SSphere((SVector4(quadObjectCenter, 1.0f) * transformMatrix).ToVector3(), radius);

				if (!viewFrustum.Intersects(quadBoundingSphere))
					return;
			}

			if (!renderView->DrawData.WorldSpaceSpriteDrawData.contains(drawID))
			{
				// TODO.NW: Need unique names for all external assets that are declared, maybe if they are external they
				// can just use the asset ref ID as resource ID?
				SRenderResourceHandle spriteTextureHandle = renderView->RenderResourceRegistry->DeclareRenderTexture(asset->Name.c_str(), &asset->RenderTexture);
				renderView->DrawData.WorldSpaceSpriteDrawData.emplace(drawID, SSpriteDrawData{ .SpriteTextureHandle = spriteTextureHandle });
			}

			SSpriteDrawData& spriteDrawData = renderView->DrawData.WorldSpaceSpriteDrawData.at(drawID);
			spriteDrawData.Transforms.push_back(transformComp->Transform.GetMatrix());
			spriteDrawData.UVRects.push_back(component->UVRect);
			spriteDrawData.Colors.emplace_back(component->Color.AsVector4());
			spriteDrawData.Entities.push_back(component->Owner);
		}
		else if (SComponent::IsValid(transform2DComp))
		{
			if (doCulling)
			{
				// TODO.NW: Figure out bounding quad for rotated screen space quad, check intersection with screen quad
				// (or just directly check transformed (including aspect ratio) vertices against -0.5 -> 0.5 space)
			}

			if (!renderView->DrawData.ScreenSpaceSpriteDrawData.contains(drawID))
			{
				// TODO.NW: Need unique names for all external assets that are declared, maybe if they are external they
				// can just use the asset ref ID as resource ID?
				SRenderResourceHandle spriteTextureHandle = renderView->RenderResourceRegistry->DeclareRenderTexture(asset->Name.c_str(), &asset->RenderTexture);
				renderView->DrawData.ScreenSpaceSpriteDrawData.emplace(drawID, SSpriteDrawData{ .SpriteTextureHandle = spriteTextureHandle });
			}

			SMatrix screenSpaceMatrix;
			screenSpaceMatrix.SetScale(transform2DComp->Scale.X, transform2DComp->Scale.Y, 1.0f);
			screenSpaceMatrix *= SMatrix::CreateRotationAroundZ(UMath::DegToRad(transform2DComp->DegreesRoll));
			screenSpaceMatrix.SetTranslation({ transform2DComp->Position.X, transform2DComp->Position.Y, 0.0f });

			SSpriteDrawData& spriteDrawData = renderView->DrawData.ScreenSpaceSpriteDrawData.at(drawID);
			spriteDrawData.Transforms.push_back(screenSpaceMatrix);
			spriteDrawData.UVRects.push_back(component->UVRect);
			spriteDrawData.Colors.emplace_back(component->Color.AsVector4());
			spriteDrawData.Entities.push_back(component->Owner);
		}
	}

	void CRenderSystem::ProcessUICanvas(SRenderView* renderView, CScene* scene, const SUICanvasComponent* component) const
	{
		if (!SComponent::IsValid(component) || !component->IsActive)
			return;

		const STransform2DComponent* transform2DComp = scene->GetComponent<STransform2DComponent>(component);
		if (!SComponent::IsValid(transform2DComp))
			return;
		
		for (const SUIElement& element : component->Elements)
		{
			if (element.StateAssetReferences.size() != STATIC_U64(EUIElementState::Count))
				continue;

			if (element.UVRects.size() != STATIC_U64(EUIElementState::Count))
				continue;

			const SAssetReference& assetReference = element.StateAssetReferences[STATIC_U8(element.State)];
			if (!assetReference.IsValid())
				continue;

			STextureAsset* asset = AssetRegistry->RequestAssetData<STextureAsset>(assetReference, component->Owner.GUID);
			if (asset == nullptr)
				continue;

			const U32 drawID = assetReference.UID;

			if (!renderView->DrawData.ScreenSpaceSpriteDrawData.contains(drawID))
			{
				// TODO.NW: Need unique names for all external assets that are declared, maybe if they are external they
				// can just use the asset ref ID as resource ID?
				SRenderResourceHandle spriteTextureHandle = renderView->RenderResourceRegistry->DeclareRenderTexture("Sprite", &asset->RenderTexture);
				renderView->DrawData.ScreenSpaceSpriteDrawData.emplace(drawID, SSpriteDrawData{ .SpriteTextureHandle = spriteTextureHandle });
			}

			SMatrix screenSpaceMatrix;
			screenSpaceMatrix.SetScale(transform2DComp->Scale.X * element.LocalScale.X, transform2DComp->Scale.Y * element.LocalScale.Y, 1.0f);
			screenSpaceMatrix *= SMatrix::CreateRotationAroundZ(UMath::DegToRad(transform2DComp->DegreesRoll + element.LocalDegreesRoll));
			screenSpaceMatrix.SetTranslation({ transform2DComp->Position.X + element.LocalPosition.X, transform2DComp->Position.Y + element.LocalPosition.Y, 0.0f });

			SSpriteDrawData& spriteDrawData = renderView->DrawData.ScreenSpaceSpriteDrawData.at(drawID);
			spriteDrawData.Transforms.push_back(screenSpaceMatrix);
			spriteDrawData.UVRects.push_back(element.UVRects[STATIC_U8(element.State)]);
			spriteDrawData.Colors.emplace_back(element.Color.AsVector4());
			spriteDrawData.Entities.push_back(component->Owner);			
		}	
	}

	SVolumetricLightBufferData CRenderSystem::ProcessVolumetricLightData(const SVolumetricLightComponent* component) const
	{
		const bool hasActiveVolumetricLightComp = SComponent::IsValid(component) && component->IsActive;

		return !hasActiveVolumetricLightComp ? SVolumetricLightBufferData{} :
			SVolumetricLightBufferData
			{
			.NumberOfSamplesReciprocal = 1.0f / component->NumberOfSamples,
			.LightPower = component->LightPower,
			.ScatteringProbability = component->ScatteringProbability,
			.HenyeyGreensteinGValue = component->HenyeyGreensteinGValue
			};
	}

	SShadowmapProcessingResult CRenderSystem::ProcessShadowmapBufferData(const std::vector<SShadowmapViewData>& viewData, const SVector2<F32>& shadowAtlasResolution, const F32 shadowTestTolerance) const
	{
		std::vector<SShadowmapBufferData> bufferData;
		std::vector<U16> viewportIndices;

		for (const SShadowmapViewData& data : viewData)
		{
			const CRenderViewport shadowmapViewport = StateManager->Viewports[data.ShadowmapViewportIndex];
			const SVector2<F32> shadowmapStartingUV = { shadowmapViewport.TopLeftX / shadowAtlasResolution.X, shadowmapViewport.TopLeftY / shadowAtlasResolution.Y };

			bufferData.emplace_back(SShadowmapBufferData
				{
				.ToShadowmapView = data.ShadowViewMatrix,
				.ToShadowmapProjection = data.ShadowProjectionMatrix,
				.ShadowmapPosition = data.ShadowPosition,
				.ShadowmapResolution = shadowmapViewport.GetResolution(),
				.ShadowAtlasResolution = shadowAtlasResolution,
				.ShadowmapStartingUV = shadowmapStartingUV,
				.ShadowTestTolerance = shadowTestTolerance
				});

			viewportIndices.push_back(data.ShadowmapViewportIndex);
		}

		return { bufferData, viewportIndices };
	}

	SStaticMeshDrawData* CRenderSystem::ProcessStaticGeometry(SRenderView* renderView, const std::vector<SDrawCallData>& drawCallData, const std::vector<SGraphicsMaterialAsset*>& materialAssets, const SSceneLightCollection& sceneLights, const bool doCulling, const SSphere& boundingSphere, const SFrustum& viewFrustum, const SMatrix& meshTransformMatrix, const U32 drawID, const U32 meshAssetID, const U64 assetRequester) const
	{
		// TODO.NW: It's probably a great idea to save this in the runtime asset for use here, like the draw call data is now,
		// if we can handle the transition in the render resource registry nicely. Might be a thing to keep buffers loaded
		// in the registry itself while the asset is loaded? Maybe.
		// When do we fill the material data?
		std::vector<SPrimitiveData> primitiveDataOnly;
		std::vector<SMeshDrawData> fullDrawData;

		for (const SDrawCallData& data : drawCallData)
		{
			SPrimitiveData primitiveData;
			// TODO.NW: Need unique names for all these resources, including materials
			primitiveData.VertexBufferHandle = renderView->RenderResourceRegistry->DeclareVertexBuffer<SStaticMeshVertex>("VertexBuffer", &RenderManager->GetRenderStateManager().VertexBuffers[data.VertexBufferIndex]);
			primitiveData.IndexBufferHandle = renderView->RenderResourceRegistry->DeclareIndexBuffer("IndexBuffer", &RenderManager->GetRenderStateManager().IndexBuffers[data.IndexBufferIndex]);
			primitiveData.IndexCount = data.IndexCount;
			primitiveDataOnly.push_back(primitiveData);

			const SEngineGraphicsMaterial& material = materialAssets[data.MaterialIndex]->Material;

			SMeshDrawData meshDrawData;
			meshDrawData.PrimitiveData = primitiveData;
			std::tie(meshDrawData.MaterialTextureHandles, meshDrawData.MaterialBufferData) = ProcessMaterialData(renderView, material, assetRequester);
			fullDrawData.push_back(meshDrawData);
		}

		for (U64 i = 0; i < sceneLights.DirectionalLights.size(); i++)
		{
			if (!doCulling || sceneLights.DirectionalLightCullGeometry[i].Intersects(boundingSphere))
			{
				std::unordered_map<U32, SStaticShadowCasterData>& shadowCasterData = renderView->DrawData.DirectionalLightDrawData[i].ShadowCasterData;
				if (!shadowCasterData.contains(meshAssetID))
					shadowCasterData.emplace(meshAssetID, SStaticShadowCasterData{ .PrimitiveData = primitiveDataOnly, .Transforms = {} });

				shadowCasterData.at(meshAssetID).Transforms.push_back(meshTransformMatrix);
			}
		}

		for (U64 i = 0; i < sceneLights.PointLights.size(); i++)
		{
			if (!doCulling || sceneLights.PointLightCullGeometry[i].Intersects(boundingSphere))
			{
				std::unordered_map<U32, SStaticShadowCasterData>& shadowCasterData = renderView->DrawData.PointLightDrawData[i].ShadowCasterData;
				if (!shadowCasterData.contains(meshAssetID))
					shadowCasterData.emplace(meshAssetID, SStaticShadowCasterData{ .PrimitiveData = primitiveDataOnly, .Transforms = {} });

				shadowCasterData.at(meshAssetID).Transforms.push_back(meshTransformMatrix);
			}
		}

		for (U64 i = 0; i < sceneLights.Spotlights.size(); i++)
		{
			if (!doCulling || sceneLights.SpotlightCullGeometry[i].Intersects(boundingSphere))
			{
				std::unordered_map<U32, SStaticShadowCasterData>& shadowCasterData = renderView->DrawData.SpotlightDrawData[i].ShadowCasterData;
				if (!shadowCasterData.contains(meshAssetID))
					shadowCasterData.emplace(meshAssetID, SStaticShadowCasterData{ .PrimitiveData = primitiveDataOnly, .Transforms = {} });

				shadowCasterData.at(meshAssetID).Transforms.push_back(meshTransformMatrix);
			}
		}

		if (doCulling && !viewFrustum.Intersects(boundingSphere))
			return nullptr;

		if (!renderView->DrawData.StaticMeshDrawData.contains(drawID))
			renderView->DrawData.StaticMeshDrawData.emplace(drawID, SStaticMeshDrawData{ .DrawData = fullDrawData, .Transforms = {}, .Entities = {} });

		return &renderView->DrawData.StaticMeshDrawData.at(drawID);
	}

	SMaterialProcessingResult CRenderSystem::ProcessMaterialData(SRenderView* renderView, const SEngineGraphicsMaterial& material, const U64 assetRequester) const
	{
		// TODO.NW: Potentially finish building these (assigning material resources) after light binding and camera frustum culling?
		std::map<U32, CStaticRenderTexture> uniqueMaterialTextures = material.GetRenderTextures(assetRequester);
		
		std::vector<SRenderResourceHandle> textureHandles;
		for (auto& [textureUID, texture] : uniqueMaterialTextures)
		{
			// TODO.NW: Need unique names for all these resources, including buffers
			textureHandles.push_back(renderView->RenderResourceRegistry->DeclareRenderTexture("MaterialTexture", &texture));
		}

		// Post Processing material buffer: Assigning runtime index from unique texture set
		SMaterialBufferData bufferData = SMaterialBufferData(material);
		for (SRuntimeGraphicsMaterialProperty& property : bufferData.Properties)
		{
			auto it = uniqueMaterialTextures.find(property.TextureUID);
			if (it == uniqueMaterialTextures.end())
				continue;

			property.TextureIndex = STATIC_F32(std::distance(uniqueMaterialTextures.begin(), it));
		}

		return std::tuple(textureHandles, bufferData);
	}

	void CRenderSystem::AddDefaultPasses(SRenderView* renderView, const bool runEditorDataPasses) const
	{
		// TODO.NW: Put passes for thumbnail rendering in Editor project?

		SVector2<F32> shadowAtlasResolution = RenderManager->GetShadowAtlasResolution();

		renderView->RenderGraph.AddPass<SShadowmapPrePassData>("ShadowmapPrePass",
			[&](CRenderResourceRegistry& registry)
			{
				SShadowmapPrePassData passData;

				passData.TransformBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
				passData.ViewDataBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SFrameBufferData>("Shadowmap Frame Buffer"));

				// TODO.NW: Figure out if it's reasonable to bind any other existing render target here, do they even need to be the same dimensions?
				passData.EmptyRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Empty Render Target", ERenderResourceFormat::R8_UnsignedInt, SVector2<U16>(STATIC_U16(shadowAtlasResolution.X), STATIC_U16(shadowAtlasResolution.Y))));
				passData.ShadowmapRenderTargetHandle = passData.AddOutput(registry.DeclareRenderDepth("Shadow Atlas Depth", ERenderResourceFormat::R32_Typeless, SVector2<U16>(STATIC_U16(shadowAtlasResolution.X), STATIC_U16(shadowAtlasResolution.Y))));

				return passData;
			},
			[](const SShadowmapPrePassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SMainTargetRenderData targetData;
				targetData.MainRenderTarget = registry.GetResource(data.EmptyRenderTargetHandle);
				targetData.DepthTexture = registry.GetResource(data.ShadowmapRenderTargetHandle);
				renderManager->LightRenderer.SetupShadowPrepassRender(targetData);

				{
					SStaticMeshShadowRenderData rendererData;

					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.ViewDataBuffer = registry.GetResource(data.ViewDataBufferHandle);

					for (const SDirectionalLightDrawData& directionalLightData : renderView.DrawData.DirectionalLightDrawData)
					{
						rendererData.ShadowmapViewportIndex = directionalLightData.ShadowmapViewportIndex;
						const SShadowmapBufferData& bufferData = directionalLightData.ShadowmapBufferData;
						SFrameBufferData frameBufferData;
						frameBufferData.ToCameraFromWorld = bufferData.ToShadowmapView;
						frameBufferData.ToWorldFromCamera = bufferData.ToShadowmapView.FastInverse();
						frameBufferData.ToProjectionFromCamera = bufferData.ToShadowmapProjection;
						frameBufferData.ToCameraFromProjection = bufferData.ToShadowmapProjection.Inverse();
						frameBufferData.CameraPosition = bufferData.ShadowmapPosition;
						rendererData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

						for (const auto& [drawID, shadowCasterData] : directionalLightData.ShadowCasterData)
						{
							rendererData.TransformBuffer->DataBuffer->BindBuffer(shadowCasterData.Transforms);
							rendererData.InstanceCount = STATIC_U32(shadowCasterData.Transforms.size());

							for (const SPrimitiveData& primitiveData : shadowCasterData.PrimitiveData)
							{
								rendererData.VertexBuffer = registry.GetResource(primitiveData.VertexBufferHandle);
								rendererData.IndexBuffer = registry.GetResource(primitiveData.IndexBufferHandle);
								rendererData.IndexCount = primitiveData.IndexCount;

								renderManager->LightRenderer.RenderStaticMeshDepthPrePass(rendererData);
							}
						}
					}

					for (const SPointLightDrawData& pointLightData : renderView.DrawData.PointLightDrawData)
					{
						rendererData.ShadowmapViewportIndex = pointLightData.ShadowmapViewportIndex;
						for (const SShadowmapBufferData& bufferData : pointLightData.ShadowmapBufferData)
						{
							SFrameBufferData frameBufferData;
							frameBufferData.ToCameraFromWorld = bufferData.ToShadowmapView;
							frameBufferData.ToWorldFromCamera = bufferData.ToShadowmapView.FastInverse();
							frameBufferData.ToProjectionFromCamera = bufferData.ToShadowmapProjection;
							frameBufferData.ToCameraFromProjection = bufferData.ToShadowmapProjection.Inverse();
							frameBufferData.CameraPosition = bufferData.ShadowmapPosition;
							rendererData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

							for (const auto& [drawID, shadowCasterData] : pointLightData.ShadowCasterData)
							{
								rendererData.TransformBuffer->DataBuffer->BindBuffer(shadowCasterData.Transforms);
								rendererData.InstanceCount = STATIC_U32(shadowCasterData.Transforms.size());

								for (const SPrimitiveData& primitiveData : shadowCasterData.PrimitiveData)
								{
									rendererData.VertexBuffer = registry.GetResource(primitiveData.VertexBufferHandle);
									rendererData.IndexBuffer = registry.GetResource(primitiveData.IndexBufferHandle);
									rendererData.IndexCount = primitiveData.IndexCount;

									renderManager->LightRenderer.RenderStaticMeshDepthPrePass(rendererData);
								}
							}
						}
					}

					for (const SSpotlightDrawData& spotlightData : renderView.DrawData.SpotlightDrawData)
					{
						rendererData.ShadowmapViewportIndex = spotlightData.ShadowmapViewportIndex;
						const SShadowmapBufferData& bufferData = spotlightData.ShadowmapBufferData;
						SFrameBufferData frameBufferData;
						frameBufferData.ToCameraFromWorld = bufferData.ToShadowmapView;
						frameBufferData.ToWorldFromCamera = bufferData.ToShadowmapView.FastInverse();
						frameBufferData.ToProjectionFromCamera = bufferData.ToShadowmapProjection;
						frameBufferData.ToCameraFromProjection = bufferData.ToShadowmapProjection.Inverse();
						frameBufferData.CameraPosition = bufferData.ShadowmapPosition;
						rendererData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

						for (const auto& [drawID, shadowCasterData] : spotlightData.ShadowCasterData)
						{
							rendererData.TransformBuffer->DataBuffer->BindBuffer(shadowCasterData.Transforms);
							rendererData.InstanceCount = STATIC_U32(shadowCasterData.Transforms.size());

							for (const SPrimitiveData& primitiveData : shadowCasterData.PrimitiveData)
							{
								rendererData.VertexBuffer = registry.GetResource(primitiveData.VertexBufferHandle);
								rendererData.IndexBuffer = registry.GetResource(primitiveData.IndexBufferHandle);
								rendererData.IndexCount = primitiveData.IndexCount;

								renderManager->LightRenderer.RenderStaticMeshDepthPrePass(rendererData);
							}
						}
					}
				}

				{
					// TODO.NW: Skeletal mesh shadows
				}
			}
		);

		renderView->RenderGraph.AddPass<SGBufferBasePassData>("GBufferBasePass",
			[&](CRenderResourceRegistry& registry)
			{
				SGBufferBasePassData passData;

				renderView->RenderGraph.AddDependency<SShadowmapPrePassData, SGBufferBasePassData>();

				passData.ViewDataBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SFrameBufferData>("View Data Buffer"));
				passData.TransformBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
				passData.UVRectBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SVector4>("Instanced UV Rect Buffer", InstancedDrawInstanceLimit));
				passData.ColorBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SVector4>("Instanced Color Buffer", InstancedDrawInstanceLimit));
				passData.EntityBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SEntity>("Instanced Entity Buffer", InstancedDrawInstanceLimit));
				passData.MaterialBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SMaterialBufferData>("Material Buffer"));
				passData.BoneBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SBoneBufferData>("Bone Buffer"));

				// NOTE.NW: It's interesting to note here that whenever we're rendering geometry, it's assumed we'd like to do it to a full resolution target. 
				// That may not be the case, and we might want to make use of the render view parameter in this case or have some option to decide for each pass 
				// how it should be handled.

				passData.GBufferAlbedoTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt, renderView));
				passData.GBufferWNormalTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt, renderView));
				passData.GBufferVNormalTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt, renderView));
				passData.GBufferMaterialTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt, renderView));

				if (runEditorDataPasses)
				{
					passData.GBufferWorldPositionTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer World Position", ERenderResourceFormat::R32G32B32A32_Float, renderView));
					passData.GBufferEditorDataTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Editor Data", ERenderResourceFormat::R32G32_UnsignedInt, renderView));
				}

				passData.IntermediateDepthTargetHandle = passData.AddOutput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless, renderView));

				return passData;
			},
			[=](const SGBufferBasePassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFrameBufferData frameBufferData;
				frameBufferData.ToWorldFromCamera = renderView.ViewMatrix;
				frameBufferData.ToCameraFromWorld = renderView.ViewMatrix.FastInverse();
				frameBufferData.ToProjectionFromCamera = renderView.ProjectionMatrix;
				frameBufferData.ToCameraFromProjection = renderView.ProjectionMatrix.Inverse();
				frameBufferData.CameraPosition = renderView.ViewMatrix.GetTranslation4();
				SGeometryConstantRenderData constantRenderData;
				constantRenderData.ViewDataBuffer = registry.GetResource(data.ViewDataBufferHandle);
				constantRenderData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

				SGeometryTargetRenderData targetRenderData;
				targetRenderData.AlbedoRenderTarget = registry.GetResource(data.GBufferAlbedoTargetHandle);
				targetRenderData.WorldNormalRenderTarget = registry.GetResource(data.GBufferWNormalTargetHandle);
				targetRenderData.VertexNormalRenderTarget = registry.GetResource(data.GBufferVNormalTargetHandle);
				targetRenderData.MaterialRenderTarget = registry.GetResource(data.GBufferMaterialTargetHandle);
				targetRenderData.DepthTexture = registry.GetResource(data.IntermediateDepthTargetHandle);

				SEditorRenderData editorData;
				if (runEditorDataPasses)
				{
					editorData.WorldPositionRenderTarget = registry.GetResource(data.GBufferWorldPositionTargetHandle);
					editorData.EditorDataRenderTarget = registry.GetResource(data.GBufferEditorDataTargetHandle);
					editorData.EntityBuffer = registry.GetResource(data.EntityBufferHandle);

					renderManager->GeometryRenderer.SetupRender(constantRenderData, targetRenderData, editorData);
				}
				else
				{
					renderManager->GeometryRenderer.SetupRender(constantRenderData, targetRenderData);
				}

				{
					SStaticMeshRenderData rendererData;
					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.MaterialBuffer = registry.GetResource(data.MaterialBufferHandle);

					for (auto& [drawID, meshData] : renderView.DrawData.StaticMeshDrawData) // Static Mesh Level
					{
						rendererData.TransformBuffer->DataBuffer->BindBuffer(meshData.Transforms);

						if (runEditorDataPasses)
							editorData.EntityBuffer->DataBuffer->BindBuffer(meshData.Entities);

						for (const SMeshDrawData& drawData : meshData.DrawData) // Sub Mesh Level
						{
							for (const SRenderResourceHandle& handle : drawData.MaterialTextureHandles) // Material Texture Level
								rendererData.MaterialTextures.emplace_back(registry.GetResource(handle));

							rendererData.MaterialBuffer->DataBuffer->BindBuffer(drawData.MaterialBufferData);

							rendererData.VertexBuffer = registry.GetResource(drawData.PrimitiveData.VertexBufferHandle);
							rendererData.IndexBuffer = registry.GetResource(drawData.PrimitiveData.IndexBufferHandle);

							rendererData.IndexCount = drawData.PrimitiveData.IndexCount;
							rendererData.InstanceCount = STATIC_U32(meshData.Transforms.size());

							if (runEditorDataPasses)
								renderManager->GeometryRenderer.RenderStaticMesh(rendererData, editorData);
							else
								renderManager->GeometryRenderer.RenderStaticMesh(rendererData);
						}
					}
				}

				{
					SSkeletalMeshRenderData rendererData;
					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.MaterialBuffer = registry.GetResource(data.MaterialBufferHandle);
					rendererData.BoneBuffer = registry.GetResource(data.BoneBufferHandle);

					for (auto& [drawID, meshData] : renderView.DrawData.SkeletalMeshDrawData)
					{
						rendererData.TransformBuffer->DataBuffer->BindBuffer(meshData.Transforms);
						rendererData.BoneBuffer->DataBuffer->BindBuffer(meshData.Bones);

						if (runEditorDataPasses)
							editorData.EntityBuffer->DataBuffer->BindBuffer(meshData.Entities);

						for (const SMeshDrawData& drawData : meshData.DrawData)
						{
							for (const SRenderResourceHandle& handle : drawData.MaterialTextureHandles)
								rendererData.MaterialTextures.emplace_back(registry.GetResource(handle));

							rendererData.MaterialBuffer->DataBuffer->BindBuffer(drawData.MaterialBufferData);

							rendererData.VertexBuffer = registry.GetResource(drawData.PrimitiveData.VertexBufferHandle);
							rendererData.IndexBuffer = registry.GetResource(drawData.PrimitiveData.IndexBufferHandle);

							rendererData.IndexCount = drawData.PrimitiveData.IndexCount;
							rendererData.InstanceCount = STATIC_U32(meshData.Transforms.size());

							if (runEditorDataPasses)
								renderManager->GeometryRenderer.RenderSkeletalMesh(rendererData, editorData);
							else
								renderManager->GeometryRenderer.RenderSkeletalMesh(rendererData);
						}
					}
				}

				{
					SSpriteRenderData rendererData;
					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.UVRectBuffer = registry.GetResource(data.UVRectBufferHandle);
					rendererData.ColorBuffer = registry.GetResource(data.ColorBufferHandle);

					for (auto& [spriteID, drawData] : renderView.DrawData.WorldSpaceSpriteDrawData)
					{
						rendererData.TransformBuffer->DataBuffer->BindBuffer(drawData.Transforms);
						rendererData.UVRectBuffer->DataBuffer->BindBuffer(drawData.UVRects);
						rendererData.ColorBuffer->DataBuffer->BindBuffer(drawData.Colors);

						if (runEditorDataPasses)
							editorData.EntityBuffer->DataBuffer->BindBuffer(drawData.Entities);

						rendererData.SpriteTexture = registry.GetResource(drawData.SpriteTextureHandle);
						rendererData.InstanceCount = STATIC_U32(drawData.Transforms.size());

						if (runEditorDataPasses)
							renderManager->SpriteRenderer.RenderEditorWorldSpaceSprite(rendererData, editorData);
						else
							renderManager->SpriteRenderer.RenderWorldSpaceSprite(rendererData);
					}
				}
			}
		);

		renderView->RenderGraph.AddPass<SGeometryDepthCopyPassData>("GeometryDepthCopy",
			[&](CRenderResourceRegistry& registry)
			{
				SGeometryDepthCopyPassData passData;

				renderView->RenderGraph.AddDependency<SGBufferBasePassData, SGeometryDepthCopyPassData>();

				passData.DepthTextureHandle = passData.AddInput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless, renderView));
				passData.DepthRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Depth Copy Target", ERenderResourceFormat::R32_Float, renderView));

				return passData;
			},
			[](const SGeometryDepthCopyPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& /*renderView*/)
			{
				SFullscreenRenderData rendererData;
				rendererData.TextureResource1 = registry.GetResource(data.DepthTextureHandle);
				rendererData.RenderTarget = registry.GetResource(data.DepthRenderTargetHandle);
				rendererData.PixelShader = EPixelShaders::FullscreenCopyDepth;
				rendererData.BlendState = EBlendStates::Disable;
				renderManager->FullscreenRenderer.Render(rendererData);
			}
		);

		renderView->RenderGraph.AddPass<SGeometryDecalPassData>("GBufferDecalPass",
			[&](CRenderResourceRegistry& registry)
			{
				SGeometryDecalPassData passData;

				renderView->RenderGraph.AddDependency<SGBufferBasePassData, SGeometryDecalPassData>();
				renderView->RenderGraph.AddDependency<SGeometryDepthCopyPassData, SGeometryDecalPassData>();

				passData.ViewDataBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SFrameBufferData>("View Data Buffer"));
				passData.DepthCopyTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Depth Copy", ERenderResourceFormat::R32_Float, renderView));
				passData.TransformBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
				passData.MaterialBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SMaterialBufferData>("Material Buffer"));

				passData.GBufferAlbedoTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt, renderView));
				passData.GBufferWNormalTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt, renderView));
				passData.GBufferVNormalTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt, renderView));
				passData.GBufferMaterialTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt, renderView));
				passData.IntermediateDepthTargetHandle = passData.AddOutput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless, renderView));

				return passData;
			},
			[](const SGeometryDecalPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFrameBufferData frameBufferData;
				frameBufferData.ToWorldFromCamera = renderView.ViewMatrix;
				frameBufferData.ToCameraFromWorld = renderView.ViewMatrix.FastInverse();
				frameBufferData.ToProjectionFromCamera = renderView.ProjectionMatrix;
				frameBufferData.ToCameraFromProjection = renderView.ProjectionMatrix.Inverse();
				frameBufferData.CameraPosition = renderView.ViewMatrix.GetTranslation4();
				SGeometryConstantRenderData constantRenderData;
				constantRenderData.ViewDataBuffer = registry.GetResource(data.ViewDataBufferHandle);
				constantRenderData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

				SGeometryTargetRenderData targetRenderData;
				targetRenderData.AlbedoRenderTarget = registry.GetResource(data.GBufferAlbedoTargetHandle);
				targetRenderData.WorldNormalRenderTarget = registry.GetResource(data.GBufferWNormalTargetHandle);
				targetRenderData.VertexNormalRenderTarget = registry.GetResource(data.GBufferVNormalTargetHandle);
				targetRenderData.MaterialRenderTarget = registry.GetResource(data.GBufferMaterialTargetHandle);
				targetRenderData.DepthTexture = registry.GetResource(data.IntermediateDepthTargetHandle);

				renderManager->GeometryRenderer.SetupRender(constantRenderData, targetRenderData);

				SDecalRenderData rendererData;
				rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
				rendererData.MaterialBuffer = registry.GetResource(data.MaterialBufferHandle);
				rendererData.DepthCopyTexture = registry.GetResource(data.DepthCopyTextureHandle);

				for (auto& [materialID, drawData] : renderView.DrawData.DecalDrawData)
				{
					rendererData.TransformBuffer->DataBuffer->BindBuffer(drawData.Transforms);
					rendererData.MaterialBuffer->DataBuffer->BindBuffer(drawData.MaterialBufferData);

					for (const SRenderResourceHandle& handle : drawData.MaterialTextureHandles)
						rendererData.MaterialTextures.emplace_back(registry.GetResource(handle));

					rendererData.InstanceCount = STATIC_U32(drawData.Transforms.size());

					renderManager->GeometryRenderer.RenderDecal(rendererData);
				}
			}
		);

		renderView->RenderGraph.AddPass<SSSAOPassData>("SSAO",
			[&](CRenderResourceRegistry& registry)
			{
				SSSAOPassData passData;

				renderView->RenderGraph.AddDependency<SGeometryDecalPassData, SSSAOPassData>();

				// TODO.NW: Figure out why we can't do a half resolution SSAO pass here, do we need to assign the viewports or something?
				// Also match it with the light pass below
				passData.SSAOBufferTextureHandle = passData.AddOutput(registry.DeclareRenderTexture("SSAO Blur Texture", ERenderResourceFormat::R16G16B16A16_Float, renderView));
				passData.DepthTextureHandle = passData.AddInput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless, renderView));
				passData.GBufferWNormalTextureHandle = passData.AddInput(registry.DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt, renderView));
				passData.SSAOBlurTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("SSAO Blur Target", ERenderResourceFormat::R16G16B16A16_Float, renderView));

				return passData;
			},
			[](const SSSAOPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& /*renderView*/)
			{
				SFullscreenRenderData rendererData;
				rendererData.TextureResource1 = registry.GetResource(data.DepthTextureHandle);
				rendererData.TextureResource2 = registry.GetResource(data.GBufferWNormalTextureHandle);
				rendererData.RenderTarget = registry.GetResource(data.SSAOBufferTextureHandle);
				rendererData.PixelShader = EPixelShaders::FullscreenSSAO;
				rendererData.BlendState = EBlendStates::Disable;
				renderManager->FullscreenRenderer.Render(rendererData);

				rendererData.TextureResource1 = registry.GetResource(data.SSAOBufferTextureHandle);
				rendererData.RenderTarget = registry.GetResource(data.SSAOBlurTargetHandle);
				rendererData.PixelShader = EPixelShaders::FullscreenSSAOBlur;
				rendererData.BlendState = EBlendStates::Disable;
				renderManager->FullscreenRenderer.Render(rendererData);
			}
		);

		renderView->RenderGraph.AddPass<SLightPassData>("LightingPass",
			[&](CRenderResourceRegistry& registry)
			{
				SLightPassData passData;

				renderView->RenderGraph.AddDependency<SSSAOPassData, SLightPassData>();

				passData.ViewDataBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SFrameBufferData>("View Data Buffer"));
				passData.GBufferAlbedoTextureHandle = passData.AddInput(registry.DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt, renderView)); // slot 1
				passData.GBufferWNormalTextureHandle = passData.AddInput(registry.DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt, renderView)); // slot 2
				passData.GBufferVNormalTextureHandle = passData.AddInput(registry.DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt, renderView)); // slot 3
				passData.GBufferMaterialTextureHandle = passData.AddInput(registry.DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt, renderView)); // slot 4
				passData.IntermediateDepthTextureHandle = passData.AddInput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless, renderView)); // slot 21
				passData.ShadowAtlasDepthTextureHandle = passData.AddInput(registry.DeclareRenderDepth("Shadow Atlas Depth", ERenderResourceFormat::R32_Typeless, SVector2<U16>(STATIC_U16(shadowAtlasResolution.X), STATIC_U16(shadowAtlasResolution.Y)))); // slot 22
				passData.SSAOBlurTextureHandle = passData.AddInput(registry.DeclareRenderTexture("SSAO Blur Texture", ERenderResourceFormat::R16G16B16A16_Float, renderView)); // slot 23

				passData.DirectionalLightBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SDirectionalLightBufferData>("Directional Light Buffer"));
				passData.PointLightBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SDirectionalLightBufferData>("Point Light Buffer"));
				passData.SpotlightBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SDirectionalLightBufferData>("Spotlight Buffer"));
				passData.ShadowmapBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SShadowmapBufferData>("Shadowmap Buffer", 6));
				passData.EmissiveBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SEmissiveBufferData>("Emissive Buffer"));

				//passData.MainRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Main Render Target", &renderView->RenderTarget));
				passData.MainRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Lit Scene", ERenderResourceFormat::R16G16B16A16_Float, renderView));

				return passData;
			},
			[](const SLightPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFrameBufferData frameBufferData;
				frameBufferData.ToWorldFromCamera = renderView.ViewMatrix;
				frameBufferData.ToCameraFromWorld = renderView.ViewMatrix.FastInverse();
				frameBufferData.ToProjectionFromCamera = renderView.ProjectionMatrix;
				frameBufferData.ToCameraFromProjection = renderView.ProjectionMatrix.Inverse();
				frameBufferData.CameraPosition = renderView.ViewMatrix.GetTranslation4();
				SGeometryConstantRenderData viewData;
				viewData.ViewDataBuffer = registry.GetResource(data.ViewDataBufferHandle);
				viewData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

				SLightConstantRenderData constantData;
				constantData.GBufferAlbedoTexture = registry.GetResource(data.GBufferAlbedoTextureHandle);
				constantData.GBufferWNormalTexture = registry.GetResource(data.GBufferWNormalTextureHandle);
				constantData.GBufferVNormalTexture = registry.GetResource(data.GBufferVNormalTextureHandle);
				constantData.GBufferMaterialTexture = registry.GetResource(data.GBufferMaterialTextureHandle);
				constantData.IntermediateDepthTexture = registry.GetResource(data.IntermediateDepthTextureHandle);
				constantData.ShadowAtlasDepthTexture = registry.GetResource(data.ShadowAtlasDepthTextureHandle);
				constantData.SSAOBlurTexture = registry.GetResource(data.SSAOBlurTextureHandle);

				SMainTargetRenderData targetData;
				targetData.MainRenderTarget = registry.GetResource(data.MainRenderTargetHandle);

				renderManager->LightRenderer.SetupLightRender(viewData, constantData, targetData);

				{
					SDirectionalLightRenderData rendererData;

					for (const SDirectionalLightDrawData& drawData : renderView.DrawData.DirectionalLightDrawData)
					{
						rendererData.LightBuffer = registry.GetResource(data.DirectionalLightBufferHandle);
						rendererData.LightBuffer->DataBuffer->BindBuffer(drawData.LightBufferData);

						rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
						rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(drawData.ShadowmapBufferData);

						rendererData.EmissiveBuffer = registry.GetResource(data.EmissiveBufferHandle);
						rendererData.EmissiveBuffer->DataBuffer->BindBuffer(SEmissiveBufferData{ .EmissiveStrength = 2.1f }); // TODO.NW: Extract this from fullscreenrenderer post processing buffer data

						rendererData.CubemapTexture = registry.GetResource(drawData.CubemapTextureHandle);

						renderManager->LightRenderer.RenderDirectionalLight(rendererData);
					}
				}

				{
					SPointLightRenderData rendererData;

					for (const SPointLightDrawData& drawData : renderView.DrawData.PointLightDrawData)
					{
						rendererData.LightBuffer = registry.GetResource(data.PointLightBufferHandle);
						rendererData.LightBuffer->DataBuffer->BindBuffer(drawData.LightBufferData);

						rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
						rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(drawData.ShadowmapBufferData.data());

						renderManager->LightRenderer.RenderPointLight(rendererData);
					}
				}

				{
					SSpotlightRenderData rendererData;

					for (const SSpotlightDrawData& drawData : renderView.DrawData.SpotlightDrawData)
					{
						rendererData.PointLightBuffer = registry.GetResource(data.PointLightBufferHandle);
						rendererData.PointLightBuffer->DataBuffer->BindBuffer(drawData.PointLightBufferData);

						rendererData.SpotlightBuffer = registry.GetResource(data.SpotlightBufferHandle);
						rendererData.SpotlightBuffer->DataBuffer->BindBuffer(drawData.SpotlightBufferData);

						rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
						rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(drawData.ShadowmapBufferData);

						renderManager->LightRenderer.RenderSpotlight(rendererData);
					}
				}

				if (renderView.SkyboxCubemapTextureHandle.IsValid())
				{
					targetData.DepthTexture = constantData.IntermediateDepthTexture;
					renderManager->LightRenderer.SetupSkyboxRender(viewData, targetData);

					SSkyboxRenderData rendererData;
					rendererData.CubemapTexture = registry.GetResource(renderView.SkyboxCubemapTextureHandle);
					renderManager->LightRenderer.RenderSkybox(rendererData);
				}
				// ändrat ssao till fullres, ändrade den här lit scene -> main render target från additiv blend till disable, 
				// renderade direkt till main render target innan. Behöver cleara alla texturerna vi använder innan vi använder dem nästa frame
				// (alltså resurser som fortfarande lever och inte plockats bort av resource registryn, men som fortfarande ägs av grafen)
				renderView.RenderTarget.SetAsActiveTarget();
				targetData.MainRenderTarget->RenderTexture->SetAsPSResourceOnSlot(0);
				renderManager->FullscreenRenderer.Render(EPixelShaders::FullscreenCopy, EBlendStates::Disable);

				// TODO.NW: See if there's any issue with binding depth for skybox and rendering to 
				// main target instead of keeping a LitScene copy
			}
		);

		renderView->RenderGraph.AddPass<SVolumetricLightPassData>("VolumetricAccumulationPass",
			[&](CRenderResourceRegistry& registry)
			{
				SVolumetricLightPassData passData;

				renderView->RenderGraph.AddDependency<SLightPassData, SVolumetricLightPassData>();
				renderView->RenderGraph.AddDependency<SShadowmapPrePassData, SVolumetricLightPassData>();

				passData.ViewDataBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SFrameBufferData>("View Data Buffer"));
				passData.IntermediateDepthTextureHandle = passData.AddInput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless, renderView)); // slot 21
				passData.ShadowAtlasDepthTextureHandle = passData.AddInput(registry.DeclareRenderDepth("Shadow Atlas Depth", ERenderResourceFormat::R32_Typeless, SVector2<U16>(STATIC_U16(shadowAtlasResolution.X), STATIC_U16(shadowAtlasResolution.Y)))); // slot 22	
				passData.DirectionalLightBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SDirectionalLightBufferData>("Directional Light Buffer"));
				passData.PointLightBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SDirectionalLightBufferData>("Point Light Buffer"));
				passData.SpotlightBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SDirectionalLightBufferData>("Spotlight Buffer"));
				passData.VolumetricLightBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SVolumetricLightBufferData>("Volumetric Light Buffer"));
				passData.ShadowmapBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SShadowmapBufferData>("Shadowmap Buffer", 6));

				passData.AccumulationRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Volumetric Accumulation", ERenderResourceFormat::R16G16B16A16_Float, renderView, ERenderTextureSizeHint::HalfResolution));

				return passData;
			},
			[](const SVolumetricLightPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFrameBufferData frameBufferData;
				frameBufferData.ToWorldFromCamera = renderView.ViewMatrix;
				frameBufferData.ToCameraFromWorld = renderView.ViewMatrix.FastInverse();
				frameBufferData.ToProjectionFromCamera = renderView.ProjectionMatrix;
				frameBufferData.ToCameraFromProjection = renderView.ProjectionMatrix.Inverse();
				frameBufferData.CameraPosition = renderView.ViewMatrix.GetTranslation4();
				SGeometryConstantRenderData viewData;
				viewData.ViewDataBuffer = registry.GetResource(data.ViewDataBufferHandle);
				viewData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

				SVolumetricConstantRenderData constantData;
				constantData.IntermediateDepthTexture = registry.GetResource(data.IntermediateDepthTextureHandle);
				constantData.ShadowAtlasDepthTexture = registry.GetResource(data.ShadowAtlasDepthTextureHandle);

				SMainTargetRenderData targetData;
				targetData.MainRenderTarget = registry.GetResource(data.AccumulationRenderTargetHandle);

				renderManager->LightRenderer.SetupVolumetricRender(viewData, constantData, targetData);

				// TODO.NW: Find neat way of skipping these calls if the volumetrics aren't enabled
				{
					SVolumetricDirectionalLightRenderData rendererData;

					for (const SDirectionalLightDrawData& drawData : renderView.DrawData.DirectionalLightDrawData)
					{
						if (!drawData.RenderVolumetrics)
							continue;

						rendererData.LightBuffer = registry.GetResource(data.DirectionalLightBufferHandle);
						rendererData.LightBuffer->DataBuffer->BindBuffer(drawData.LightBufferData);

						rendererData.VolumetricLightBuffer = registry.GetResource(data.VolumetricLightBufferHandle);
						rendererData.VolumetricLightBuffer->DataBuffer->BindBuffer(drawData.VolumetricBufferData);

						rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
						rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(drawData.ShadowmapBufferData);

						renderManager->LightRenderer.RenderVolumetricDirectionalLight(rendererData);
					}
				}

				{
					SVolumetricPointLightRenderData rendererData;

					for (const SPointLightDrawData& drawData : renderView.DrawData.PointLightDrawData)
					{
						if (!drawData.RenderVolumetrics)
							continue;

						rendererData.LightBuffer = registry.GetResource(data.PointLightBufferHandle);
						rendererData.LightBuffer->DataBuffer->BindBuffer(drawData.LightBufferData);

						rendererData.VolumetricLightBuffer = registry.GetResource(data.VolumetricLightBufferHandle);
						rendererData.VolumetricLightBuffer->DataBuffer->BindBuffer(drawData.VolumetricBufferData);

						rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
						rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(drawData.ShadowmapBufferData);

						renderManager->LightRenderer.RenderVolumetricPointLight(rendererData);
					}
				}

				{
					SVolumetricSpotlightRenderData rendererData;

					for (const SSpotlightDrawData& drawData : renderView.DrawData.SpotlightDrawData)
					{
						if (!drawData.RenderVolumetrics)
							continue;

						rendererData.PointLightBuffer = registry.GetResource(data.PointLightBufferHandle);
						rendererData.PointLightBuffer->DataBuffer->BindBuffer(drawData.PointLightBufferData);

						rendererData.SpotlightBuffer = registry.GetResource(data.SpotlightBufferHandle);
						rendererData.SpotlightBuffer->DataBuffer->BindBuffer(drawData.SpotlightBufferData);

						rendererData.VolumetricLightBuffer = registry.GetResource(data.VolumetricLightBufferHandle);
						rendererData.VolumetricLightBuffer->DataBuffer->BindBuffer(drawData.VolumetricBufferData);

						rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
						rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(drawData.ShadowmapBufferData);

						renderManager->LightRenderer.RenderVolumetricSpotlight(rendererData);
					}
				}
			}
		);

		renderView->RenderGraph.AddPass<SVolumetricBlurPassData>("VolumetricBlurPass",
			[&](CRenderResourceRegistry& registry)
			{
				SVolumetricBlurPassData passData;

				renderView->RenderGraph.AddDependency<SVolumetricLightPassData, SVolumetricBlurPassData>();
				renderView->RenderGraph.AddDependency<SVolumetricBlurPassData, SBloomPassData>();
				
				passData.DepthTextureHandle = passData.AddInput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless, renderView));
				passData.DownsamplingDepthTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Downsampling Depth", ERenderResourceFormat::R32_Float, renderView, ERenderTextureSizeHint::HalfResolution));
				passData.PrimaryBlurTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Volumetric Accumulation", ERenderResourceFormat::R16G16B16A16_Float, renderView, ERenderTextureSizeHint::HalfResolution));
				passData.SecondaryBlurTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Volumetric Blur", ERenderResourceFormat::R16G16B16A16_Float, renderView, ERenderTextureSizeHint::HalfResolution));

				passData.RenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Main Render Target", &renderView->RenderTarget));

				return passData;
			},
			[](const SVolumetricBlurPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& /*renderView*/)
			{
				SBilateralBlurRenderData rendererData;
				rendererData.RenderTarget = registry.GetResource(data.RenderTargetHandle);
				rendererData.DepthTexture = registry.GetResource(data.DepthTextureHandle);
				rendererData.DownsamplingDepthTexture = registry.GetResource(data.DownsamplingDepthTextureHandle);
				rendererData.PrimaryBlurTexture = registry.GetResource(data.PrimaryBlurTextureHandle);
				rendererData.SecondaryBlurTexture = registry.GetResource(data.SecondaryBlurTextureHandle);
				renderManager->FullscreenRenderer.RenderBilateralBlur(rendererData);
			}
		);

		renderView->RenderGraph.AddPass<SBloomPassData>("Bloom",
			[&](CRenderResourceRegistry& registry)
			{
				SBloomPassData passData;

				renderView->RenderGraph.AddDependency<SLightPassData, SBloomPassData>();
				renderView->RenderGraph.AddDependency<SBloomPassData, STonemappingPassData>();

				passData.BloomBufferTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Bloom Buffer", ERenderResourceFormat::R16G16B16A16_Float, renderView));
				passData.TargetCopyTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Render Target Copy", ERenderResourceFormat::R16G16B16A16_Float, renderView));
				passData.HalfResolutionTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Half Resolution Target", ERenderResourceFormat::R16G16B16A16_Float, renderView, ERenderTextureSizeHint::HalfResolution));
				passData.PrimaryBlurTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Primary Blur Texture", ERenderResourceFormat::R16G16B16A16_Float, renderView, ERenderTextureSizeHint::QuarterResolution));
				passData.SecondaryBlurTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Secondary Blur Texture", ERenderResourceFormat::R16G16B16A16_Float, renderView, ERenderTextureSizeHint::QuarterResolution));
				
				passData.MainRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Main Render Target", &renderView->RenderTarget));

				return passData;
			},
			[](const SBloomPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& /*renderView*/)
			{
				SFullscreenRenderData fullscreenRenderData;
				fullscreenRenderData.PixelShader = EPixelShaders::FullscreenCopy;
				fullscreenRenderData.BlendState = EBlendStates::Disable;
				fullscreenRenderData.RenderTarget = registry.GetResource(data.TargetCopyTextureHandle);
				fullscreenRenderData.TextureResource1 = registry.GetResource(data.MainRenderTargetHandle);
				renderManager->FullscreenRenderer.Render(fullscreenRenderData);

				fullscreenRenderData.PixelShader = EPixelShaders::FullscreenLuminance;
				fullscreenRenderData.BlendState = EBlendStates::Disable;
				fullscreenRenderData.RenderTarget = registry.GetResource(data.BloomBufferTextureHandle);
				fullscreenRenderData.TextureResource1 = registry.GetResource(data.MainRenderTargetHandle);
				renderManager->FullscreenRenderer.Render(fullscreenRenderData);
				
				SGaussianBlurRenderData blurRenderData;
				blurRenderData.RenderTarget = registry.GetResource(data.BloomBufferTextureHandle);
				blurRenderData.HalfResolutionTexture = registry.GetResource(data.HalfResolutionTextureHandle);
				blurRenderData.PrimaryBlurTexture = registry.GetResource(data.PrimaryBlurTextureHandle);
				blurRenderData.SecondaryBlurTexture = registry.GetResource(data.SecondaryBlurTextureHandle);
				renderManager->FullscreenRenderer.RenderGaussianBlur(blurRenderData);

				// TODO.NW: Could potentially be a thing to additively blend on the bloom buffer directly 
				// to the render target, using only one resource (and additive blend state)
				// Alternatively, use a UAV?
				fullscreenRenderData.PixelShader = EPixelShaders::FullscreenBloom;
				fullscreenRenderData.BlendState = EBlendStates::Disable;
				fullscreenRenderData.RenderTarget = registry.GetResource(data.MainRenderTargetHandle);
				fullscreenRenderData.TextureResource1 = registry.GetResource(data.TargetCopyTextureHandle);
				fullscreenRenderData.TextureResource2 = registry.GetResource(data.BloomBufferTextureHandle);
				renderManager->FullscreenRenderer.Render(fullscreenRenderData);
			}
		);

		renderView->RenderGraph.AddPass<STonemappingPassData>("Tonemapping",
			[&](CRenderResourceRegistry& registry)
			{
				STonemappingPassData passData;

				passData.RenderTargetTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Main Render Target", &renderView->RenderTarget));
				passData.TonemappingTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Tonemapped Texture", ERenderResourceFormat::R16G16B16A16_Float, renderView));
				
				return passData;
			},
			[](const STonemappingPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& /*renderView*/)
			{
				SFullscreenRenderData rendererData;
				rendererData.TextureResource1 = registry.GetResource(data.RenderTargetTextureHandle);
				rendererData.RenderTarget = registry.GetResource(data.TonemappingTargetHandle);
				rendererData.PixelShader = EPixelShaders::FullscreenTonemap;
				rendererData.BlendState = EBlendStates::Disable;
				renderManager->FullscreenRenderer.Render(rendererData);
			}
		);

		renderView->RenderGraph.AddPass<SUIPassData>("UIPass",
			[&](CRenderResourceRegistry& registry)
			{
				SUIPassData passData;

				renderView->RenderGraph.AddDependency<STonemappingPassData, SUIPassData>();
				renderView->RenderGraph.AddDependency<SUIPassData, SAntialiasingPassData>();

				passData.TransformBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
				passData.UVRectBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SVector4>("Instanced UV Rect Buffer", InstancedDrawInstanceLimit));
				passData.ColorBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SVector4>("Instanced Color Buffer", InstancedDrawInstanceLimit));

				passData.MainRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Tonemapped Texture", ERenderResourceFormat::R16G16B16A16_Float, renderView));

				// NW: Should this pass have depth?
				passData.DepthTargetHandle = passData.AddOutput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless, renderView));

				return passData;
			},
			[](const SUIPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SMainTargetRenderData targetData;
				targetData.MainRenderTarget = registry.GetResource(data.MainRenderTargetHandle);
				targetData.DepthTexture = registry.GetResource(data.DepthTargetHandle);
				renderManager->SpriteRenderer.SetupScreenSpaceRender(targetData);

				{
					SSpriteRenderData rendererData;
					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.UVRectBuffer = registry.GetResource(data.UVRectBufferHandle);
					rendererData.ColorBuffer = registry.GetResource(data.ColorBufferHandle);

					for (auto& [spriteID, drawData] : renderView.DrawData.UISpriteDrawData)
					{
						rendererData.TransformBuffer->DataBuffer->BindBuffer(drawData.Transforms);
						rendererData.UVRectBuffer->DataBuffer->BindBuffer(drawData.UVRects);
						rendererData.ColorBuffer->DataBuffer->BindBuffer(drawData.Colors);

						rendererData.SpriteTexture = registry.GetResource(drawData.SpriteTextureHandle);

						rendererData.InstanceCount = STATIC_U32(drawData.Transforms.size());

						renderManager->SpriteRenderer.RenderScreenSpaceSprite(rendererData);
					}
				}
			}
		);

		renderView->RenderGraph.AddPass<SDebugDrawPassData>("DebugLines",
			[&](CRenderResourceRegistry& registry)
			{
				SDebugDrawPassData passData;

				renderView->RenderGraph.AddDependency<STonemappingPassData, SDebugDrawPassData>();
				renderView->RenderGraph.AddDependency<SDebugDrawPassData, SAntialiasingPassData>();

				passData.ViewDataBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SFrameBufferData>("View Data Buffer"));
				passData.DebugObjectBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SDebugShapeObjectBufferData>("Debug Shape Buffer"));

				passData.MainRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Tonemapped Texture", ERenderResourceFormat::R16G16B16A16_Float, renderView));
				passData.DepthTargetHandle = passData.AddOutput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless, renderView));

				return passData;
			},
			[](const SDebugDrawPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SDebugRenderData rendererData;
				rendererData.ObjectBuffer = registry.GetResource(data.DebugObjectBufferHandle);
				SDebugShapeObjectBufferData bufferData;

				SFrameBufferData frameBufferData;
				frameBufferData.ToWorldFromCamera = renderView.ViewMatrix;
				frameBufferData.ToCameraFromWorld = renderView.ViewMatrix.FastInverse();
				frameBufferData.ToProjectionFromCamera = renderView.ProjectionMatrix;
				frameBufferData.ToCameraFromProjection = renderView.ProjectionMatrix.Inverse();
				frameBufferData.CameraPosition = renderView.ViewMatrix.GetTranslation4();
				SGeometryConstantRenderData viewData;
				viewData.ViewDataBuffer = registry.GetResource(data.ViewDataBufferHandle);
				viewData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

				SMainTargetRenderData targetData;
				targetData.MainRenderTarget = registry.GetResource(data.MainRenderTargetHandle);
				targetData.DepthTexture = registry.GetResource(data.DepthTargetHandle);

				auto fillRendererData = [&](const SDebugObjectDrawData& drawData)
					{
						bufferData.ToWorldFromObject = drawData.ToWorldFromObject;
						bufferData.Color = drawData.Color;
						bufferData.HalfThickness = drawData.HalfThickness;

						rendererData.ObjectBuffer->DataBuffer->BindBuffer(bufferData);
						rendererData.VertexBuffer = registry.GetResource(drawData.PrimitiveData.VertexBufferHandle);
						rendererData.IndexBuffer = registry.GetResource(drawData.PrimitiveData.IndexBufferHandle);
						rendererData.IndexCount = drawData.PrimitiveData.IndexCount;

						return rendererData;
					};

				renderManager->DebugRenderer.SetupDepthRender(viewData, targetData);
				{
					for (const SDebugObjectDrawData& drawData : renderView.DrawData.DebugObjectsDepth3D)
						renderManager->DebugRenderer.RenderDebugShape3D(fillRendererData(drawData));

					for (const SDebugObjectDrawData& drawData : renderView.DrawData.DebugObjectsDepth2D)
						renderManager->DebugRenderer.RenderDebugShape2D(fillRendererData(drawData));
				}

				renderManager->DebugRenderer.SetupNoDepthRender(viewData, targetData);
				{
					for (const SDebugObjectDrawData& drawData : renderView.DrawData.DebugObjectsNoDepth3D)
						renderManager->DebugRenderer.RenderDebugShape3D(fillRendererData(drawData));
					
					for (const SDebugObjectDrawData& drawData : renderView.DrawData.DebugObjectsNoDepth2D)
						renderManager->DebugRenderer.RenderDebugShape2D(fillRendererData(drawData));
				}
			}
		);

		renderView->RenderGraph.AddPass<SAntialiasingPassData>("Antialiasing",
			[&](CRenderResourceRegistry& registry)
			{
				SAntialiasingPassData passData;

				renderView->RenderGraph.AddDependency<STonemappingPassData, SAntialiasingPassData>();
				
				passData.TonemappedTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Tonemapped Texture", ERenderResourceFormat::R16G16B16A16_Float, renderView));
				passData.AntialiasingTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Antialiased Target", ERenderResourceFormat::R16G16B16A16_Float, renderView));

				return passData;
			},
			[](const SAntialiasingPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& /*renderView*/)
			{
				SFullscreenRenderData rendererData;
				rendererData.TextureResource1 = registry.GetResource(data.TonemappedTextureHandle);
				rendererData.RenderTarget = registry.GetResource(data.AntialiasingTargetHandle);
				rendererData.PixelShader = EPixelShaders::FullscreenFXAA;
				rendererData.BlendState = EBlendStates::Disable;
				renderManager->FullscreenRenderer.Render(rendererData);
			}
		);

		renderView->RenderGraph.AddPass<SGammaCorrectionPassData>("GammaCorrection",
			[&](CRenderResourceRegistry& registry)
			{
				SGammaCorrectionPassData passData;

				renderView->RenderGraph.AddDependency<SAntialiasingPassData, SGammaCorrectionPassData>();
				
				passData.AntialiasedTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Antialiased Target", ERenderResourceFormat::R16G16B16A16_Float, renderView));
				passData.MainRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Main Render Target", &renderView->RenderTarget));

				return passData;
			},
			[](const SGammaCorrectionPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& /*renderView*/)
			{
				SFullscreenRenderData rendererData;
				rendererData.TextureResource1 = registry.GetResource(data.AntialiasedTextureHandle);
				rendererData.RenderTarget = registry.GetResource(data.MainRenderTargetHandle);
				rendererData.PixelShader = EPixelShaders::FullscreenGammaCorrection;
				rendererData.BlendState = EBlendStates::Disable;
				renderManager->FullscreenRenderer.Render(rendererData);
			}
		);

		renderView->RenderGraph.AddPass<SFinalizePassData>("Finalize",
			[&](CRenderResourceRegistry& /*registry*/)
			{
				SFinalizePassData passData;
				passData.SetIsRootPass();

				//// TODO.NW: Figure out how to do this, potentially in the Finalize pass
				//CRenderViewport viewport = CRenderViewport(RHI, 0.0f, 0.0f, 256.0f, 256.0f, 0.0f, 1.0f);
				//viewport.SetViewport();
				//ShadowAtlasDepth.SetAsPSResourceOnSlot(0);
				////RenderFullscreenPass(EPixelShaders::FullscreenCopyDepth, EBlendStates::Disable);

				// TODO.NW: Figure out what we can do debug wise. By setting dependencies we should
				// be able to automatically disable passes that aren't used. What can we do to override
				// Rasterizer states globally (for geometry) for wireframe debugging?
				// if (!RenderManager->ParseDebugBuffer)
				renderView->RenderGraph.AddDependency<SGammaCorrectionPassData, SFinalizePassData>();
				// 
				//else
				//{
				//	passData.AddProducer<SGeometryBasePassData>();
				//	passData.PrimaryOptionalTextureHandle = passData.AddInput(registry.DeclareRenderTexture("GBuffer Albedo Texture", ERenderResourceFormat::R16G16B16A16_Float, renderView));
				//	passData.MainRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Main Render Target", &renderView->RenderTarget));
				//}

				return passData;
			},
			[](const SFinalizePassData& /*data*/, CRenderResourceRegistry& /*registry*/, CRenderManager* /*renderManager*/, SRenderView& /*renderView*/)
			{
				//SFullscreenRenderData rendererData;
				//rendererData.TextureResource1 = registry.GetResource(data.PrimaryOptionalTextureHandle);
				////rendererData.TextureResource2 = registry.GetResource(data.SecondaryOptionalTextureHandle);
				//rendererData.RenderTarget = registry.GetResource(data.MainRenderTargetHandle);
				//rendererData.PixelShader = EPixelShaders::FullscreenCopy;
				//rendererData.BlendState = EBlendStates::Disable;
				//renderManager->FullscreenRenderer.Render(rendererData);
			}
		);
	}
}
