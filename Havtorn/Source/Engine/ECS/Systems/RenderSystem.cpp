// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "RenderSystem.h"
#include "Scene/Scene.h"
#include "ECS/ECSInclude.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderCommand.h"
#include "Input/Input.h"

namespace Havtorn
{
	CRenderSystem::CRenderSystem(CRenderManager* renderManager)
		: ISystem()
		, RenderManager(renderManager)
	{
	}

	void CRenderSystem::Update(CScene* scene)
	{
		std::vector<SStaticMeshComponent>& staticMeshComponents = scene->GetStaticMeshComponents();
		std::vector<STransformComponent>& transformComponents = scene->GetTransformComponents();
		std::vector<SCameraComponent>& cameraComponents = scene->GetCameraComponents();
		std::vector<SMaterialComponent>& materialComponents = scene->GetMaterialComponents();
		std::vector<SEnvironmentLightComponent>& environmentLightComponents = scene->GetEnvironmentLightComponents();
		std::vector<SDirectionalLightComponent>& directionalLightComponents = scene->GetDirectionalLightComponents();
		std::vector<SPointLightComponent>& pointLightComponents = scene->GetPointLightComponents();
		std::vector<SSpotLightComponent>& spotLightComponents = scene->GetSpotLightComponents();
		std::vector<SVolumetricLightComponent>& volumetricLightComponents = scene->GetVolumetricLightComponents();
		std::vector<SSpriteComponent>& spriteComponents = scene->GetSpriteComponents();
		std::vector<STransform2DComponent>& transform2DComponents = scene->GetTransform2DComponents();

		const auto& decalComponents = scene->GetDecalComponents();

		RenderManager->ClearSystemStaticMeshInstanceTransforms();
		RenderManager->ClearSpriteInstanceWorldSpaceTransforms();
		RenderManager->ClearSpriteInstanceScreenSpaceTransforms();
		RenderManager->ClearSpriteInstanceUVRects();
		RenderManager->ClearSpriteInstanceColors();

		bool sceneHasActiveCamera = false;

		// TODO.NR: Could probably merge all of these loops into one
		// NR: Not worth doing right now

		for (U64 i = 0; i < cameraComponents.size(); i++)
		{
			const SCameraComponent& cameraComponent = cameraComponents[i];
			if (!cameraComponent.IsInUse)
				continue;

			sceneHasActiveCamera = true;

			const STransformComponent& transformComponent = transformComponents[i];

			SRenderCommand command;
			command.Type = ERenderCommandType::CameraDataStorage;
			command.ObjectMatrix = transformComponent.Transform.GetMatrix();
			command.ProjectionMatrix = cameraComponent.ProjectionMatrix;
			RenderManager->PushRenderCommand(command);
		}

		if (!sceneHasActiveCamera)
			return;

		for (U64 i = 0; i < staticMeshComponents.size(); i++)
		{
			const SStaticMeshComponent& staticMeshComponent = staticMeshComponents[i];
			if (!staticMeshComponent.IsInUse)
				continue;
		
			const I64 transformCompIndex = i;
			const STransformComponent& transformComp = transformComponents[transformCompIndex];

			const I64 materialCompIndex = i;
			auto& materialComp = materialComponents[materialCompIndex];

			if (!RenderManager->IsStaticMeshInInstancedRenderList(staticMeshComponent.Name.AsString())) // if static, if instanced
			{		
				for (U64 j = 0; j < staticMeshComponents.size(); j++)
				{
					if (directionalLightComponents[j].IsInUse)
					{
						const SDirectionalLightComponent& directionalLightComp = directionalLightComponents[j];

						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassDirectional;			
						command.ShadowViewData = directionalLightComp.ShadowmapView;
						command.ObjectMatrix = transformComp.Transform.GetMatrix();
						command.StaticMeshName = staticMeshComponent.Name.AsString();
						command.TopologyIndex = staticMeshComponent.TopologyIndex;
						command.DrawCallData = staticMeshComponent.DrawCallData;
						RenderManager->PushRenderCommand(command);
					}

					if (pointLightComponents[j].IsInUse)
					{
						const SPointLightComponent& pointLightComp = pointLightComponents[j];

						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassPoint;
						command.ObjectMatrix = transformComp.Transform.GetMatrix();
						command.StaticMeshName = staticMeshComponent.Name.AsString();
						command.TopologyIndex = staticMeshComponent.TopologyIndex;
						command.DrawCallData = staticMeshComponent.DrawCallData;
						command.ShadowmapViews = pointLightComp.ShadowmapViews;
						RenderManager->PushRenderCommand(command);
					}

					if (spotLightComponents[j].IsInUse)
					{
						const SSpotLightComponent& spotLightComp = spotLightComponents[j];

						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassSpot;
						command.ShadowViewData = spotLightComp.ShadowmapView;
						command.ObjectMatrix = transformComp.Transform.GetMatrix();
						command.StaticMeshName = staticMeshComponent.Name.AsString();
						command.TopologyIndex = staticMeshComponent.TopologyIndex;
						command.DrawCallData = staticMeshComponent.DrawCallData;
						RenderManager->PushRenderCommand(command);
					}
				}

				SRenderCommand command;
				command.Type = ERenderCommandType::GBufferDataInstanced;
				command.ObjectMatrix = transformComp.Transform.GetMatrix();
				command.StaticMeshName = staticMeshComponent.Name.AsString();
				command.TopologyIndex = staticMeshComponent.TopologyIndex;
				command.DrawCallData = staticMeshComponent.DrawCallData;
				command.SamplerIndex = staticMeshComponent.SamplerIndex;
				command.PixelShaderIndex = staticMeshComponent.PixelShaderIndex;
				command.Materials = materialComp.Materials;
				RenderManager->PushRenderCommand(command);
			}

			RenderManager->AddStaticMeshToInstancedRenderList(staticMeshComponent.Name.AsString(), transformComp.Transform.GetMatrix());
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::DecalDepthCopy;
			RenderManager->PushRenderCommand(command);
		}
		
		for (U64 i = 0; i < decalComponents.size(); i++)
		{
			const SDecalComponent& decalComponent = decalComponents[i];
			if (!decalComponent.IsInUse)
				continue;

			const STransformComponent& transformComp = transformComponents[i];

			SRenderCommand command;
			command.Type = ERenderCommandType::DeferredDecal;
			command.ObjectMatrix = transformComp.Transform.GetMatrix();
			command.ShouldRenderAlbedo = decalComponent.ShouldRenderAlbedo;
			command.ShouldRenderMaterial = decalComponent.ShouldRenderMaterial;
			command.ShouldRenderNormal = decalComponent.ShouldRenderNormal;
			command.TextureReferences = decalComponent.TextureReferences;
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::PreLightingPass;
			RenderManager->PushRenderCommand(command);
		}

		for (U64 i = 0; i < directionalLightComponents.size(); i++)
		{
			const SDirectionalLightComponent& directionalLightComp = directionalLightComponents[i];
			if (!directionalLightComp.IsInUse)
				continue;

			// TODO: Fix hard coded index, maybe store index of environmentlight entity that the light source should use/closest?
			const SEnvironmentLightComponent& environmentLightComp = environmentLightComponents[1];
			if (!environmentLightComp.IsInUse)
				continue;

			SRenderCommand command;
			command.Type = ERenderCommandType::DeferredLightingDirectional;
			command.AmbientCubemapReference = environmentLightComp.AmbientCubemapReference;
			command.Direction = directionalLightComp.Direction;
			command.Color = directionalLightComp.Color;
			command.ShadowViewData = directionalLightComp.ShadowmapView;
			RenderManager->PushRenderCommand(command);

			if (volumetricLightComponents[i].IsInUse)
			{
				SVolumetricLightComponent& volumetricLightComp = volumetricLightComponents[i];

				if (volumetricLightComp.IsActive)
				{
					command.Type = ERenderCommandType::VolumetricLightingDirectional;
					command.SetVolumetricDataFromComponent(volumetricLightComp);
					RenderManager->PushRenderCommand(command);
				}
			}
		}

		// TODO: Fix ECS View functionality, going to be a mess to keep hard coded indices
		
		for (U64 i = 0; i < pointLightComponents.size(); i++)
		{
			const SPointLightComponent& pointLightComp = pointLightComponents[i];
			if (!pointLightComp.IsInUse)
				continue;

			const STransformComponent& transformComp = transformComponents[i];

			SRenderCommand command;
			command.Type = ERenderCommandType::DeferredLightingPoint;
			command.ObjectMatrix = transformComp.Transform.GetMatrix();
			command.Color = SColor(pointLightComp.ColorAndIntensity.X, pointLightComp.ColorAndIntensity.Y, pointLightComp.ColorAndIntensity.Z, 1.0f);
			command.Intensity = pointLightComp.ColorAndIntensity.W;
			command.Range = pointLightComp.Range;
			command.ShadowmapViews = pointLightComp.ShadowmapViews;
			RenderManager->PushRenderCommand(command);

			if (volumetricLightComponents[i].IsInUse)
			{
				SVolumetricLightComponent& volumetricLightComp = volumetricLightComponents[i];

				if (volumetricLightComp.IsActive)
				{
					command.Type = ERenderCommandType::VolumetricLightingPoint;
					command.SetVolumetricDataFromComponent(volumetricLightComp);
					RenderManager->PushRenderCommand(command);
				}
			}
		}

		for (U64 i = 0; i < spotLightComponents.size(); i++)
		{
			const SSpotLightComponent& spotLightComp = spotLightComponents[i];
			if (!spotLightComp.IsInUse)
				continue;

			const STransformComponent& transformComp = transformComponents[i];

			SRenderCommand command;
			command.Type = ERenderCommandType::DeferredLightingSpot;
			command.ObjectMatrix = transformComp.Transform.GetMatrix();
			command.Color = SColor(spotLightComp.ColorAndIntensity.X, spotLightComp.ColorAndIntensity.Y, spotLightComp.ColorAndIntensity.Z, 1.0f);
			command.Intensity = spotLightComp.ColorAndIntensity.W;
			command.Range = spotLightComp.Range;
			command.Direction = spotLightComp.Direction;
			command.DirectionNormal1 = spotLightComp.DirectionNormal1;
			command.DirectionNormal2 = spotLightComp.DirectionNormal2;
			command.OuterAngle = spotLightComp.OuterAngle;
			command.InnerAngle = spotLightComp.InnerAngle;
			command.ShadowViewData = spotLightComp.ShadowmapView;
			RenderManager->PushRenderCommand(command);

			if (volumetricLightComponents[i].IsInUse)
			{
				SVolumetricLightComponent& volumetricLightComp = volumetricLightComponents[i];

				if (volumetricLightComp.IsActive)
				{
					command.Type = ERenderCommandType::VolumetricLightingSpot;
					command.SetVolumetricDataFromComponent(volumetricLightComp);
					RenderManager->PushRenderCommand(command);
				}
			}
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::PostBaseLightingPass;
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::VolumetricBufferBlurPass;
			RenderManager->PushRenderCommand(command);
		}

		for (U64 i = 0; i < spriteComponents.size(); i++)
		{
			const SSpriteComponent& spriteComp = spriteComponents[i];
			if (!spriteComp.IsInUse)
				continue;

			RenderManager->AddSpriteToInstancedUVRectRenderList(spriteComp.TextureIndex, spriteComp.UVRect);
			RenderManager->AddSpriteToInstancedColorRenderList(spriteComp.TextureIndex, spriteComp.Color.AsVector4());

			const STransformComponent& transformComp = transformComponents[i];
			const STransform2DComponent& transform2DComp = transform2DComponents[i];

			if (transformComp.IsInUse)
			{
				if (!RenderManager->IsSpriteInInstancedWorldSpaceTransformRenderList(spriteComp.TextureIndex)) 
				{
					// NR: Don't push a command every time
					SRenderCommand command;
					command.Type = ERenderCommandType::GBufferSpriteInstanced;
					command.TextureIndex = spriteComp.TextureIndex;
					RenderManager->PushRenderCommand(command);
				}

				RenderManager->AddSpriteToInstancedWorldSpaceTransformRenderList(spriteComp.TextureIndex, transformComp.Transform.GetMatrix());
			}
			else if (transform2DComp.IsInUse)
			{
				if (!RenderManager->IsSpriteInInstancedScreenSpaceTransformRenderList(spriteComp.TextureIndex))
				{
					SRenderCommand command;
					command.Type = ERenderCommandType::ScreenSpaceSprite;
					command.TextureIndex = spriteComp.TextureIndex;
					RenderManager->PushRenderCommand(command);
				}

				SMatrix transformFrom2DComponent;
				transformFrom2DComponent.SetScale(transform2DComp.Scale.X, transform2DComp.Scale.Y, 1.0f);
				transformFrom2DComponent *= SMatrix::CreateRotationAroundZ(UMath::DegToRad(transform2DComp.DegreesRoll));
				transformFrom2DComponent.SetTranslation({ transform2DComp.Position.X, transform2DComp.Position.Y, 0.0f });
				RenderManager->AddSpriteToInstancedScreenSpaceTransformRenderList(spriteComp.TextureIndex, transformFrom2DComponent);
			}
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::Bloom;
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::Tonemapping;
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::AntiAliasing;
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::GammaCorrection;
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::RendererDebug;
			RenderManager->PushRenderCommand(command);
		}
	}
}
