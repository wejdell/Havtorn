// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "Ray.h"

namespace Havtorn
{
	namespace UMathUtilities
	{
		static SRay RaycastWorld(const SVector2<F32>& mouseWindowPos, const SVector2<F32>& rectPixelSize, const SVector2<F32>& rectPixelPos, const SMatrix& viewMatrix, const SMatrix& projectionMatrix)
		{
			SVector2<F32> rectRelativeMousePos = SVector2((mouseWindowPos.X - rectPixelPos.X) / rectPixelSize.X, (mouseWindowPos.Y - rectPixelPos.Y) / rectPixelSize.Y);
			F32 xV = (2.0f * rectRelativeMousePos.X - 1.0f) / projectionMatrix(0, 0);
			F32 yV = (-2.0f * rectRelativeMousePos.Y + 1.0f) / projectionMatrix(1, 1);

			SVector4 origin = { 0.0f, 0.0f, 0.0f, 1.0f };
			SVector4 dir = { xV, yV, 1.0f, 0.0f };

			origin = viewMatrix * origin;
			dir = (viewMatrix * dir).GetNormalized();

			// Alternative method https://antongerdelan.net/opengl/raycasting.html
			//F32 x = (2.0f * rectRelativeMousePos.X - 1.0f);
			//F32 y = 1.0f - (2.0f * rectRelativeMousePos.Y);
			//SVector4 rayClip = { x, y, 1.0f, 1.0f };
			//SVector4 rayEye = projectionMatrix.Inverse() * rayClip;
			//rayEye = { rayEye.X, rayEye.Y, 1.0f, 0.0f };

			//SVector4 origin = { 0.0f, 0.0f, 0.0f, 1.0f };
			//origin = viewMatrix * origin;
			//SVector4 dir = (viewMatrix * rayEye).GetNormalized();

			HV_LOG_WARN("Mouse pos: %s", mouseWindowPos.ToString().c_str());
			HV_LOG_WARN("Rect relative pos: %s", rectRelativeMousePos.ToString().c_str());

			// NR: No way to distinguish between SRay(origin, pointOnRay) and SRay(origin, direction).
			// The former seems nice to have still so doing this for now
			SRay worldRay;
			worldRay.Origin = origin.ToVector3();
			worldRay.Direction = dir.ToVector3();
			return worldRay;
		}

		static F32 GetFocusDistanceForBounds(const SVector& boundsCenter, const SVector& bounds, SVector2<F32> fov, F32 marginPercentage = 1.0f)
		{
			auto calculateDistance = [](const SVector& center, const SVector& extents, const SVector& closestFaceDirection, const SVector& largestExtentDirection, F32 fov, F32 marginPercentage)
				{
					const SVector centerFacePosition = SVector::MaskCombine(center, extents, closestFaceDirection);
					const SVector edgePosition = SVector::MaskCombine(centerFacePosition, extents, largestExtentDirection);
					const F32 faceExtent = (centerFacePosition - edgePosition).Length();
					const F32 fittingDistance = (faceExtent * marginPercentage) / UMath::Tan(UMath::DegToRad(fov * 0.5f));
					return fittingDistance + centerFacePosition.Length();
				};

			const F32 xDistance = calculateDistance(boundsCenter, bounds, SVector::Backward, SVector::Left, fov.Y, marginPercentage);
			const F32 yDistance = calculateDistance(boundsCenter, bounds, SVector::Backward, SVector::Up, fov.X, marginPercentage);
			const F32 zDistance = calculateDistance(boundsCenter, bounds, SVector::Left, SVector::Forward, fov.X, marginPercentage);

			return UMath::Max(xDistance, UMath::Max(yDistance, zDistance));
		};
	}
}
