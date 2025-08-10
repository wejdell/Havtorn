// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "LightSystem.h"
#include "Scene/Scene.h"
#include "ECS/Components/DirectionalLightComponent.h"
#include "ECS/Components/PointLightComponent.h"
#include "ECS/Components/SpotLightComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/CameraComponent.h"
#include "Graphics/RenderManager.h"

namespace Havtorn
{
	CLightSystem::CLightSystem(CRenderManager* renderManager)
		: ISystem()
		, RenderManager(renderManager)
	{}

	void CLightSystem::Update(CScene* scene)
	{
		for (SDirectionalLightComponent* directionalLightComp : scene->GetComponents<SDirectionalLightComponent>())
		{
			if (!SComponent::IsValid(directionalLightComp))
				continue;

			//TODO.NW: Think about whether it makes more sense to have many directional lights vs one that follows the main camera, probably many?
			STransformComponent& directionalLightTransform = *scene->GetComponent<STransformComponent>(directionalLightComp);
			//STransformComponent& cameraTransform = *scene->GetComponent<STransformComponent>(scene->MainCameraEntity);
			//directionalLightTransform.Transform = cameraTransform.Transform;

			directionalLightComp->ShadowmapView.ShadowPosition = directionalLightTransform.Transform.GetMatrix().GetTranslation4();
			directionalLightComp->ShadowmapView.ShadowPosition.Y = 4.0f;

			// Round to pixel positions
			SVector position = { directionalLightComp->ShadowmapView.ShadowPosition.X, directionalLightComp->ShadowmapView.ShadowPosition.Y, directionalLightComp->ShadowmapView.ShadowPosition.Z };
			const SVector2<F32> unitsPerPixel = directionalLightComp->ShadowViewSize / RenderManager->GetShadowAtlasResolution();

			auto shadowTransform = SMatrix();
			const F32 radiansY = atan2(-directionalLightComp->Direction.X, -directionalLightComp->Direction.Z);
			const F32 l = sqrt(directionalLightComp->Direction.Z * directionalLightComp->Direction.Z + directionalLightComp->Direction.X * directionalLightComp->Direction.X);
			const F32 radiansX = atan2(directionalLightComp->Direction.Y, l);
			shadowTransform *= SMatrix::CreateRotationAroundY(radiansY);
			shadowTransform *= SMatrix::CreateRotationFromAxisAngle(shadowTransform.GetRight(), radiansX);

			F32 rightStep = position.Dot(shadowTransform.GetRight());
			position -= rightStep * shadowTransform.GetRight();
			rightStep = floor(rightStep / unitsPerPixel.X) * unitsPerPixel.X;
			position += rightStep * shadowTransform.GetRight();

			F32 upStep = position.Dot(shadowTransform.GetUp());
			position -= upStep * shadowTransform.GetUp();
			upStep = floor(upStep / unitsPerPixel.Y) * unitsPerPixel.Y;
			position += upStep * shadowTransform.GetUp();

			directionalLightComp->ShadowmapView.ShadowPosition = SVector4(position.X, position.Y, position.Z, 1.0f);

			const SVector shadowDirection = { directionalLightComp->Direction.X, directionalLightComp->Direction.Y, directionalLightComp->Direction.Z };
			directionalLightComp->ShadowmapView.ShadowViewMatrix = SMatrix::LookAtLH(position, position - shadowDirection, SVector::Up);
		}

		for (SPointLightComponent* pointLightComp : scene->GetComponents<SPointLightComponent>())
		{
			if (!SComponent::IsValid(pointLightComp))
				continue;

			SVector4 constantPosition = scene->GetComponent<STransformComponent>(pointLightComp->Owner)->Transform.GetMatrix().GetTranslation4();
			const SMatrix constantProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(90.0f), 1.0f, 0.01f, pointLightComp->Range);

			auto updateViewData = [constantPosition, constantProjectionMatrix](const U16 index, const SVector4& lookAtDirection, const SVector& lookAtUpVector, SShadowmapViewData& outViewData)
				{
					outViewData.ShadowPosition = constantPosition;
					outViewData.ShadowmapViewportIndex = index;
					outViewData.ShadowViewMatrix = SMatrix::LookAtLH(constantPosition.ToVector3(), (constantPosition + lookAtDirection).ToVector3(), lookAtUpVector);
					outViewData.ShadowProjectionMatrix = constantProjectionMatrix;
				};

			updateViewData(1, SVector4::Forward, SVector::Up, pointLightComp->ShadowmapViews[0]);
			updateViewData(2, SVector4::Right, SVector::Up, pointLightComp->ShadowmapViews[1]);
			updateViewData(3, SVector4::Backward, SVector::Up, pointLightComp->ShadowmapViews[2]);
			updateViewData(4, SVector4::Left, SVector::Up, pointLightComp->ShadowmapViews[3]);
			updateViewData(5, SVector4::Up, SVector::Backward, pointLightComp->ShadowmapViews[4]);
			updateViewData(6, SVector4::Down, SVector::Forward, pointLightComp->ShadowmapViews[5]);
		}

		for (SSpotLightComponent* spotLightComp : scene->GetComponents<SSpotLightComponent>())
		{
			if (!SComponent::IsValid(spotLightComp))
				continue;

			const SMatrix spotlightProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(90.0f), 1.0f, 0.001f, spotLightComp->Range);
			const SVector4 spotlightPosition = scene->GetComponent<STransformComponent>(spotLightComp->Owner)->Transform.GetMatrix().GetTranslation4();

			spotLightComp->ShadowmapView.ShadowPosition = spotlightPosition;
			spotLightComp->ShadowmapView.ShadowmapViewportIndex = 7;
			spotLightComp->ShadowmapView.ShadowViewMatrix = SMatrix::LookAtLH(spotlightPosition.ToVector3(), (spotlightPosition + spotLightComp->Direction).ToVector3(), spotLightComp->DirectionNormal2.ToVector3());
			spotLightComp->ShadowmapView.ShadowProjectionMatrix = spotlightProjection;
		}
	}
}
