// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "Transform.h"

#include <ranges>

namespace Havtorn
{
	const SMatrix& STransform::GetMatrix() const
	{
		if (Parent)
			return WorldMatrix;

		return LocalMatrix;
	}

	const SMatrix& STransform::GetLocalMatrix() const
	{
		return LocalMatrix;
	}

	void STransform::SetMatrix(const SMatrix& matrix)
	{
		if (Parent)
		{
			WorldMatrix = matrix;
			LocalMatrix = WorldMatrix * Parent->GetMatrix().FastInverse();
		}
		else
			LocalMatrix = matrix;

		for (STransform* transform : AttachedTransforms)
			transform->SetMatrix(transform->GetLocalMatrix() * GetMatrix());
	}

	void STransform::Rotate(const SMatrix& rotationMatrix)
	{
		LocalMatrix *= rotationMatrix;
	}

	void STransform::Rotate(const SVector& eulerAnglesRadians)
	{
		if (eulerAnglesRadians.IsEqual(SVector::Zero))
			return;

		const SMatrix rightRotation = SMatrix::CreateRotationAroundAxis(eulerAnglesRadians.X, LocalMatrix.GetRight());
		const SMatrix upRotation = SMatrix::CreateRotationAroundAxis(eulerAnglesRadians.Y, LocalMatrix.GetUp());
		const SMatrix forwardRotation = SMatrix::CreateRotationAroundAxis(eulerAnglesRadians.Z, LocalMatrix.GetForward());

		SMatrix finalRotation = LocalMatrix.GetRotationMatrix();
		finalRotation *= rightRotation;
		finalRotation *= upRotation;
		finalRotation *= forwardRotation;
		LocalMatrix.SetRotation(finalRotation);
	}

	void STransform::Translate(const SVector& v)
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

	void STransform::Translate(const SVector4& v)
	{
		Translate({ v.X, v.Y, v.Z });
	}

	void STransform::Move(const SVector& v)
	{
		Translate(v);
	}

	void STransform::Move(const SVector4& v)
	{
		Translate(v);
	}

	void STransform::Scale(const SVector& scale)
	{
		F32 validScale[3];
		validScale[0] = scale.X < FLT_EPSILON ? 0.001f : scale.X;
		validScale[1] = scale.Y < FLT_EPSILON ? 0.001f : scale.Y;
		validScale[2] = scale.Z < FLT_EPSILON ? 0.001f : scale.Z;

		LocalMatrix(0, 0) *= validScale[0];
		LocalMatrix(1, 1) *= validScale[1];
		LocalMatrix(2, 2) *= validScale[2];
	}

	void STransform::Scale(F32 xScale, F32 yScale, F32 zScale)
	{
		Scale({ xScale, yScale, zScale });
	}

	void STransform::Scale(F32 scale)
	{
		Scale({ scale, scale, scale });
	}

	// TODO.NR: Make transform struct which can store local transform data and make parent from point argument
	void STransform::Orbit(const STransform& /*transform*/, const SMatrix& rotation)
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
	void STransform::Orbit(const SVector& /*point*/, const SMatrix& rotation)
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

	void STransform::Orbit(const SVector4& /*point*/, const SMatrix& rotation)
	{
		SMatrix finalRotation = rotation;
		//finalRotation.Translation(point);
		LocalMatrix *= finalRotation;
	}

	bool STransform::HasParent() const
	{
		return Parent != nullptr;
	}

	void STransform::SetParent(STransform* parent)
	{
		Parent = parent;
		if (!Parent)
			return;

		WorldMatrix = LocalMatrix;
		LocalMatrix *= Parent->GetMatrix().FastInverse();
	}

	void STransform::AddAttachment(STransform* transform)
	{
		if (auto it = std::ranges::find(AttachedTransforms, transform); it != AttachedTransforms.end())
			return;

		AttachedTransforms.push_back(transform);
	}

	void STransform::RemoveAttachment(STransform* transform)
	{
		auto it = std::ranges::find(AttachedTransforms, transform); 
		if (it == AttachedTransforms.end())
			return;

		AttachedTransforms.erase(it);
	}

	const std::vector<STransform*>& STransform::GetAttachedTransforms() const
	{
		return AttachedTransforms;
	}
}
