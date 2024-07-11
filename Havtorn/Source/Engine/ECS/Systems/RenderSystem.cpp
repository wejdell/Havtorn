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
			command.Matrices.push_back(transformComponent.Transform.GetMatrix());
			command.Matrices.push_back(cameraComponent.ProjectionMatrix);
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
						command.ShadowmapViews.push_back(directionalLightComp.ShadowmapView);
						command.Matrices.push_back(transformComp.Transform.GetMatrix());
						command.Strings.push_back(staticMeshComponent.Name.AsString());
						command.U8s.push_back(staticMeshComponent.TopologyIndex);
						command.DrawCallData = staticMeshComponent.DrawCallData;
						RenderManager->PushRenderCommand(command);
					}

					if (pointLightComponents[j].IsInUse)
					{
						const SPointLightComponent& pointLightComp = pointLightComponents[j];

						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassPoint;
						command.Matrices.push_back(transformComp.Transform.GetMatrix());
						command.Strings.push_back(staticMeshComponent.Name.AsString());
						command.U8s.push_back(staticMeshComponent.TopologyIndex);
						command.DrawCallData = staticMeshComponent.DrawCallData;
						command.SetShadowMapViews(pointLightComp.ShadowmapViews);
						RenderManager->PushRenderCommand(command);
					}

					if (spotLightComponents[j].IsInUse)
					{
						const SSpotLightComponent& spotLightComp = spotLightComponents[j];

						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassSpot;
						command.Matrices.push_back(transformComp.Transform.GetMatrix());
						command.Strings.push_back(staticMeshComponent.Name.AsString());
						command.U8s.push_back(staticMeshComponent.TopologyIndex);
						command.DrawCallData = staticMeshComponent.DrawCallData;
						command.ShadowmapViews.push_back(spotLightComp.ShadowmapView);
						RenderManager->PushRenderCommand(command);
					}
				}

				SRenderCommand command;
				command.Type = ERenderCommandType::GBufferDataInstanced;
				command.Matrices.push_back(transformComp.Transform.GetMatrix());
				command.Strings.push_back(staticMeshComponent.Name.AsString());
				command.U8s.push_back(staticMeshComponent.TopologyIndex);
				command.U8s.push_back(staticMeshComponent.PixelShaderIndex);
				command.U8s.push_back(staticMeshComponent.SamplerIndex);
				command.DrawCallData = staticMeshComponent.DrawCallData;
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
			command.Matrices.push_back(transformComp.Transform.GetMatrix());
			command.Flags.push_back(decalComponent.ShouldRenderAlbedo);
			command.Flags.push_back(decalComponent.ShouldRenderMaterial);
			command.Flags.push_back(decalComponent.ShouldRenderNormal);
			command.U16s = decalComponent.TextureReferences;
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
			command.U16s.push_back(environmentLightComp.AmbientCubemapReference);
			command.Vectors.push_back(directionalLightComp.Direction);
			command.Colors.push_back(directionalLightComp.Color);
			command.ShadowmapViews.push_back(directionalLightComp.ShadowmapView);
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
			command.Matrices.push_back(transformComp.Transform.GetMatrix());
			command.Colors.push_back(SColor(pointLightComp.ColorAndIntensity.X, pointLightComp.ColorAndIntensity.Y, pointLightComp.ColorAndIntensity.Z, 1.0f));
			command.F32s.push_back(pointLightComp.ColorAndIntensity.W);
			command.F32s.push_back(pointLightComp.Range);
			command.SetShadowMapViews(pointLightComp.ShadowmapViews);
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
			command.Matrices.push_back(transformComp.Transform.GetMatrix());
			command.Colors.push_back(SColor(spotLightComp.ColorAndIntensity.X, spotLightComp.ColorAndIntensity.Y, spotLightComp.ColorAndIntensity.Z, 1.0f));
			command.F32s.push_back(spotLightComp.ColorAndIntensity.W);
			command.F32s.push_back(spotLightComp.Range);
			command.F32s.push_back(spotLightComp.OuterAngle);
			command.F32s.push_back(spotLightComp.InnerAngle);
			command.Vectors.push_back(spotLightComp.Direction);
			command.Vectors.push_back(spotLightComp.DirectionNormal1);
			command.Vectors.push_back(spotLightComp.DirectionNormal2);
			command.ShadowmapViews.push_back(spotLightComp.ShadowmapView);
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
					command.U32s.push_back(spriteComp.TextureIndex);
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
					command.U32s.push_back(spriteComp.TextureIndex);
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
