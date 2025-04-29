// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Matrix.h"
#include "Vector.h"
#include "Quaternion.h"

namespace Havtorn
{
	const SMatrix SMatrix::Identity = SMatrix(1.0f);
	const SMatrix SMatrix::Zero = SMatrix(0.0f);
	
	SVector SMatrix::GetEuler() const
	{
		SMatrix rotationMatrix = *this;
		rotationMatrix.OrthoNormalize();

		SVector euler;
		euler.X = UMath::RadToDeg(atan2f(rotationMatrix.M[1][2], rotationMatrix.M[2][2]));
		euler.Y = UMath::RadToDeg(atan2f(-rotationMatrix.M[0][2], sqrtf(rotationMatrix.M[1][2] * rotationMatrix.M[1][2] + rotationMatrix.M[2][2] * rotationMatrix.M[2][2])));
		euler.Z = UMath::RadToDeg(atan2f(rotationMatrix.M[0][1], rotationMatrix.M[0][0]));

		//SQuaternion quat = SQuaternion(rotationMatrix);
		//return quat.ToEuler();

		// TODO.NR: Still need to figure out what is going wrong here. Using LookAtLH to look at 0,2,1 seems to give us 0,1,2, and vice versa.

		// https://learnopencv.com/rotation-matrix-to-euler-angles/
		//SVector euler;
		//F32 sy = sqrtf(rotationMatrix(0, 0) * rotationMatrix(0, 0) + rotationMatrix(0, 1) * rotationMatrix(0, 1));
		//bool isSingular = sy < KINDA_SMALL_NUMBER;
		//if (!isSingular)
		//{
		//	euler.X = atan2f(rotationMatrix(1, 2), rotationMatrix(2, 2));
		//	euler.Y = atan2f(-rotationMatrix(0, 2), sy);
		//	euler.Z = atan2f(rotationMatrix(0, 1), rotationMatrix(0, 0));
		//}
		//else
		//{
		//	euler.X = atan2f(-rotationMatrix(2, 1), rotationMatrix(1, 1));
		//	euler.Y = atan2f(-rotationMatrix(0, 2), sy);
		//	euler.Z = 0;
		//}

		// https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToEuler/index.htm
		//SVector euler;
		//if (rotationMatrix(1, 0) > 0.998) // singularity at north pole
		//{
		//	euler.Y = atan2(rotationMatrix(0, 2), rotationMatrix(2, 2));
		//	euler.X = UMath::Pi / 2;
		//	euler.Z = 0;
		//}
		//else if (rotationMatrix(1, 0) < -0.998) // singularity at south pole
		//{
		//	euler.Y = atan2f(rotationMatrix(0, 2), rotationMatrix(2, 2));
		//	euler.X = -UMath::Pi / 2;
		//	euler.Z = 0;
		//}
		//else
		//{
		//	euler.Y = atan2f(-rotationMatrix(2, 0), rotationMatrix(0, 0));
		//	euler.Z = atan2f(-rotationMatrix(1, 2), rotationMatrix(1, 1));
		//	euler.X = UMath::ASin(rotationMatrix(1, 0));
		//}

		return euler;
	}

	void SMatrix::Decompose(const SMatrix& matrix, SVector& translation, SQuaternion& rotation, SVector& scale)
	{
		scale = matrix.GetScale();
		rotation = SQuaternion(matrix);
		translation = matrix.GetTranslation();
	}

	void SMatrix::Recompose(const SVector& translation, const SQuaternion& rotation, const SVector& scale, SMatrix& outMatrix)
	{
		SMatrix scaleMatrix;
		scaleMatrix.SetScale(scale);
		outMatrix = scaleMatrix;

		outMatrix *= SMatrix::CreateRotationFromQuaternion(rotation);

		outMatrix.SetTranslation(translation);
	}

	SMatrix SMatrix::Interpolate(const SMatrix& a, const SMatrix& b, F32 t)
	{
		SVector startTranslation, startScale, targetTranslation, targetScale = SVector::Zero;
		SQuaternion startRotation, targetRotation = SQuaternion::Identity;

		SMatrix::Decompose(a, startTranslation, startRotation, startScale);
		SMatrix::Decompose(b, targetTranslation, targetRotation, targetScale);

		SVector intermediateTranslation = SVector::Lerp(startTranslation, targetTranslation, t);
		SQuaternion intermediateRotation = SQuaternion::Slerp(startRotation, targetRotation, t).GetNormalized();
		SVector intermediateScale = SVector::Lerp(startScale, targetScale, t);

		SMatrix intermediateMatrix = SMatrix::Identity;
		SMatrix::Recompose(intermediateTranslation, intermediateRotation, intermediateScale, intermediateMatrix);

		return intermediateMatrix;
	}
}
