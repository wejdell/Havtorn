// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Matrix.h"
#include "Vector.h"
#include "Quaternion.h"

namespace Havtorn
{
	const SMatrix SMatrix::Identity = SMatrix(1.0f);
	const SMatrix SMatrix::Zero = SMatrix(0.0f);

	void SMatrix::Decompose(const SMatrix& matrix, SVector& translation, SQuaternion& rotation, SVector& scale)
	{
		scale = matrix.GetScale();
		rotation = SQuaternion(matrix);
		translation = matrix.GetTranslation();
	}

	void SMatrix::Recompose(const SVector& translation, const SQuaternion& rotation, const SVector& scale, SMatrix& outMatrix)
	{
		// TODO.NR: Make this base recomposition, as quaternion rotation should work better?
		Recompose(translation, rotation.ToEuler(), scale, outMatrix);
	}

	SMatrix SMatrix::Interpolate(const SMatrix& a, const SMatrix& b, F32 t)
	{
		SVector startTranslation, startScale, targetTranslation, targetScale = SVector::Zero;
		SQuaternion startRotation, targetRotation = SQuaternion::Identity;

		SMatrix::Decompose(a, startTranslation, startRotation, startScale);
		SMatrix::Decompose(b, targetTranslation, targetRotation, targetScale);

		SVector intermediateTranslation = SVector::Lerp(startTranslation, targetTranslation, t);
		SQuaternion intermediateRotation = SQuaternion::Slerp(startRotation, targetRotation, t);
		SVector intermediateScale = SVector::Lerp(startScale, targetScale, t);

		SMatrix intermediateMatrix = SMatrix::Identity;
		SMatrix::Recompose(intermediateTranslation, intermediateRotation, intermediateScale, intermediateMatrix);

		return intermediateMatrix;
	}
}
