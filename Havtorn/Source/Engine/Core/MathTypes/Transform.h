// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	struct STransform
	{
	private:
		SMatrix LocalMatrix = SMatrix::Identity;
		SMatrix WorldMatrix = SMatrix::Identity;

	public:
		[[nodiscard]] const SMatrix& GetMatrix() const;
		void SetMatrix(const SMatrix& matrix);

		// TODO.NR: Make all use of EulerAngles expect degrees, convert to radians internally, never force radians

		inline void Rotate(const SMatrix& rotationMatrix);
		inline void Rotate(const SVector& eulerAngles);
		inline void Translate(const SVector& v);
		inline void Translate(const SVector4& v);
		inline void Move(const SVector& v);
		inline void Move(const SVector4& v);
		// N.B: Multiplicative scaling
		inline void Scale(const SVector& v);
		// N.B: Multiplicative scaling
		inline void Scale(F32 xScale, F32 yScale, F32 zScale);
		// N.B: Multiplicative scaling
		inline void Scale(F32 scale);
		inline void Orbit(const STransform& transform, const SMatrix& rotation);
		inline void Orbit(const SVector& point, const SMatrix& rotation);
		inline void Orbit(const SVector4& point, const SMatrix& rotation);
	};

	inline const SMatrix& STransform::GetMatrix() const
	{
		//if (Parent)
		//{
		//	WorldMatrix = LocalMatrix * Parent->GetMatrix();
		//	return WorldMatrix;
		//}

		return LocalMatrix;
	}

	inline void STransform::SetMatrix(const SMatrix& matrix)
	{
		LocalMatrix = matrix;
	}

	inline void STransform::Rotate(const SMatrix& rotationMatrix)
	{
		LocalMatrix *= rotationMatrix;
	}

	inline void STransform::Rotate(const SVector& eulerAngles)
	{
		if (eulerAngles.IsEqual(SVector::Zero))
			return;

		const SMatrix rightRotation = SMatrix::CreateRotationAroundAxis(eulerAngles.X, LocalMatrix.GetRight());
		const SMatrix upRotation = SMatrix::CreateRotationAroundAxis(eulerAngles.Y, LocalMatrix.GetUp());
		const SMatrix forwardRotation = SMatrix::CreateRotationAroundAxis(eulerAngles.Z, LocalMatrix.GetForward());

		SMatrix finalRotation = LocalMatrix.GetRotationMatrix();
		finalRotation *= rightRotation;
		finalRotation *= upRotation;
		finalRotation *= forwardRotation;
		LocalMatrix.SetRotation(finalRotation);
	}

	inline void STransform::Translate(const SVector& v)
	{
		SVector localMove = LocalMatrix.GetRight() * v.X;
		LocalMatrix.M[3][0] += localMove.X;
		LocalMatrix.M[3][1] += localMove.Y;
		LocalMatrix.M[3][2] += localMove.Z;
		localMove = LocalMatrix.GetUp() * v.Y;
		LocalMatrix.M[3][0] += localMove.X;
		LocalMatrix.M[3][1] += localMove.Y;
		LocalMatrix.M[3][2] += localMove.Z;
		localMove = LocalMatrix.GetForward() * v.Z;
		LocalMatrix.M[3][0] += localMove.X;
		LocalMatrix.M[3][1] += localMove.Y;
		LocalMatrix.M[3][2] += localMove.Z;
	}

	inline void STransform::Translate(const SVector4& v)
	{
		Translate({ v.X, v.Y, v.Z });
	}

	inline void STransform::Move(const SVector& v)
	{
		Translate(v);
	}

	inline void STransform::Move(const SVector4& v)
	{
		Translate(v);
	}

	inline void STransform::Scale(const SVector& scale)
	{
		F32 validScale[3];
		validScale[0] = scale.X < FLT_EPSILON ? 0.001f : scale.X;
		validScale[1] = scale.Y < FLT_EPSILON ? 0.001f : scale.Y;
		validScale[2] = scale.Z < FLT_EPSILON ? 0.001f : scale.Z;

		LocalMatrix(0, 0) *= validScale[0];
		LocalMatrix(1, 1) *= validScale[1];
		LocalMatrix(2, 2) *= validScale[2];
	}

	inline void STransform::Scale(F32 xScale, F32 yScale, F32 zScale)
	{
		Scale({ xScale, yScale, zScale });
	}

	inline void STransform::Scale(F32 scale)
	{
		Scale({ scale, scale, scale });
	}

	// TODO.NR: Make transform struct which can store local transform data and make parent from point argument
	inline void STransform::Orbit(const STransform& /*transform*/, const SMatrix& rotation)
	{
		SMatrix finalRotation = rotation;
		//finalRotation.Translation(point);
		//SMatrix parentTransform = SMatrix();
		//parentTransform.Translation(point);
		//Translate(-point);
		LocalMatrix *= finalRotation;
		//(*this) *= parentTransform;
		//Translate(point);
	}

	// TODO.NR: Make transform struct which can store local transform data and make parent from point argument
	inline void STransform::Orbit(const SVector& /*point*/, const SMatrix& rotation)
	{
		SMatrix finalRotation = rotation;
		//finalRotation.Translation(point);
		//SMatrix parentTransform = SMatrix();
		//parentTransform.Translation(point);
		//Translate(-point);
		LocalMatrix *= finalRotation;
		//(*this) *= parentTransform;
		//Translate(point);
	}

	inline void STransform::Orbit(const SVector4& /*point*/, const SMatrix& rotation)
	{
		SMatrix finalRotation = rotation;
		//finalRotation.Translation(point);
		LocalMatrix *= finalRotation;
	}

}
