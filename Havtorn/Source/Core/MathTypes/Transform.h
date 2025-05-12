// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	struct STransform
	{
	private:
		SMatrix LocalMatrix = SMatrix::Identity;
		SMatrix WorldMatrix = SMatrix::Identity;
		STransform* Parent = nullptr;
		std::vector<STransform*> AttachedTransforms;

	public:
		CORE_API [[nodiscard]] const SMatrix& GetMatrix() const;
		CORE_API [[nodiscard]] const SMatrix& GetLocalMatrix() const;
		CORE_API void SetMatrix(const SMatrix& matrix);
		CORE_API void SetLocalMatrix(const SMatrix& matrix);

		// TODO.NR: Make all use of EulerAngles expect degrees, convert to radians internally, never force radians

		CORE_API void Rotate(const SMatrix& rotationMatrix);
		CORE_API void Rotate(const SVector& eulerAngles);
		CORE_API void Translate(const SVector& v);
		CORE_API void Translate(const SVector4& v);
		CORE_API void Move(const SVector& v);
		CORE_API void Move(const SVector4& v);
		// N.B: Multiplicative scaling
		CORE_API void Scale(const SVector& v);
		// N.B: Multiplicative scaling
		CORE_API void Scale(F32 xScale, F32 yScale, F32 zScale);
		// N.B: Multiplicative scaling
		CORE_API void Scale(F32 scale);
		CORE_API void Orbit(const STransform& transform, const SMatrix& rotation);
		CORE_API void Orbit(const SVector& point, const SMatrix& rotation);
		CORE_API void Orbit(const SVector4& point, const SMatrix& rotation);

		CORE_API bool HasParent() const;
		CORE_API void SetParent(STransform* parent);

		CORE_API void AddAttachment(STransform* transform);
		CORE_API void RemoveAttachment(STransform* transform);
	};
}
