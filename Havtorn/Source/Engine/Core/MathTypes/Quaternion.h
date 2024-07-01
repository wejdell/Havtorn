// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core/Core.h"
#include "Core/CoreTypes.h"
#include "EngineMath.h"
#include "Vector.h"
#include "Rotator.h"
#include "Matrix.h"

namespace Havtorn
{
	struct HAVTORN_API SQuaternion
	{
		F32 X = 0.0f, Y = 0.0f, Z = 0.0f, W = 1.0f;

		static const SQuaternion Identity;

		inline SQuaternion();
		inline SQuaternion(F32 x, F32 y, F32 z, F32 w);
		inline SQuaternion(F32 pitch, F32 yaw, F32 roll);
		inline SQuaternion(const SVector& eulerAngles);
		inline SQuaternion(const SVector& axis, F32 angleInDegrees);
		explicit SQuaternion(const SMatrix& M);
		//explicit SQuaternion(const SRotator& R);
		inline F32& operator[](U8 index);
		inline const F32& operator[](U8 index) const;
		SQuaternion(const SQuaternion& other) = default;
		~SQuaternion() = default;

		bool Equals(const SQuaternion& other, F32 tolerance = KINDA_SMALL_NUMBER) const;
		bool IsIdentity(F32 tolerance = KINDA_SMALL_NUMBER) const;

		// Returns the result of composing this quaternion with "other". Order matters, C = A * B
		// results in a quaternion that first applies B then A. Right first, then left.
		inline SQuaternion operator*(const SQuaternion& other) const;

		// Returns the result of composing this quaternion with "other". Order matters, C = A * B
		// results in a quaternion that first applies B then A. Right first, then left.
		inline SQuaternion operator*=(const SQuaternion& other);

		// Component-wise addition. Should NOT be used to compose quaternions, use * and *= for that
		inline SQuaternion operator+(const SQuaternion& other);
		// Component-wise addition. Should NOT be used to compose quaternions, use * and *= for that
		inline SQuaternion operator+=(const SQuaternion& other);

		inline SQuaternion operator*(const F32 scale) const;
		inline SQuaternion operator*=(const F32 scale);

		// Rotate a vector by this quaternion
		inline SVector operator*(const SVector& vector) const;

		// Returns the resulting matrix of rotating a matrix by this quaternion
		// This matrix conversion came from
		// http://www.m-hikari.com/ija/ija-password-2008/ija-password17-20-2008/aristidouIJA17-20-2008.pdf
		// used for non-uniform scaling transform.
		inline SMatrix operator*(const SMatrix& matrix) const;

		// Convert this quaternion to a vector of Euler angles in degrees
		inline SVector ToEuler() const;
		inline void ToAxisAndAngle(SVector& axis, F32& angle) const;

		inline void Normalize(F32 tolerance = SMALL_NUMBER);
		inline SQuaternion GetNormalized(float tolerance = SMALL_NUMBER) const;
		inline bool IsNormalized() const;

		// Assumes a normalized quaternion
		inline SQuaternion Inverse() const;

		// Returns angular distance between this quaternion and "other" in radians
		inline F32 AngularDistance(const SQuaternion& other) const;

		inline std::string ToString() const;

		// NR: this doesn't work as intended, even when inverting the result. Will try to fix later
		//static SQuaternion Lerp(const SQuaternion& a, const SQuaternion& b, F32 t);

		// From Game Engine Architecture, Second Edition
		static SQuaternion Slerp(const SQuaternion& a, const SQuaternion& b, F32 t);
	};

	inline SQuaternion::SQuaternion()
		: X(0.0f), Y(0.0f), Z(0.0f), W(1.0f)
	{}

	inline SQuaternion::SQuaternion(F32 x, F32 y, F32 z, F32 w)
		: X(x), Y(y), Z(z), W(w)
	{}

	inline SQuaternion::SQuaternion(F32 pitch, F32 yaw, F32 roll)
	{
		const F32 cosYaw = UMath::Cos(yaw * 0.5f);
		const F32 sinYaw = UMath::Sin(yaw * 0.5f);
		const F32 cosPitch = UMath::Cos(pitch * 0.5f);
		const F32 sinPitch = UMath::Sin(pitch * 0.5f);
		const F32 cosRoll = UMath::Cos(roll * 0.5f);
		const F32 sinRoll = UMath::Sin(roll * 0.5f);

		X = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw;
		Y = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
		Z = sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw;
		W = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw;
	}

	inline SQuaternion::SQuaternion(const SVector& eulerAngles)
		: SQuaternion(UMath::DegToRad(eulerAngles.X), UMath::DegToRad(eulerAngles.Y), UMath::DegToRad(eulerAngles.Z))
	{}

	SQuaternion::SQuaternion(const SVector& axis, F32 angleInDegrees)
	{
		F32 halfAngleInRadians = UMath::DegToRad(angleInDegrees) * 0.5f;
		F32 sine = UMath::Sin(halfAngleInRadians);
		X = axis.X * sine;
		Y = axis.Y * sine;
		Z = axis.Z * sine;
		W = UMath::Cos(halfAngleInRadians);
	}

	inline SQuaternion::SQuaternion(const SMatrix& M)
	{
		F32 trace = M.GetRotationMatrixTrace();

		if (trace > 0.0f)
		{
			F32 s = UMath::Sqrt(trace + 1.0f);
			W = 0.5f * s;

			F32 t = 0.5f / s;
			X = (M(2, 1) - M(1, 2)) * t;
			Y = (M(0, 2) - M(2, 0)) * t;
			Z = (M(1, 0) - M(0, 1)) * t;
		}
		else
		{
			U8 i = 0;
			if (M(1, 1) > M(0, 0)) i = 1;
			if (M(2, 2) > M(i, i)) i = 2;

			static const U8 next[3] = { 1, 2, 0 };
			U8 j = next[i];
			U8 k = next[j];

			F32 s = UMath::Sqrt((M(i, j) - (M(j, j) + M(k, k))) + 1.0f);
			this->operator[](i) = 0.5f * s;

			F32 t = (s != 0.0f) ? 0.5f / s : s;

			this->operator[](3) = (M(k, j) - M(j, k)) * t;
			this->operator[](j) = (M(j, i) + M(i, j)) * t;
			this->operator[](k) = (M(k, i) + M(i, k)) * t;
		}
	}

	inline F32& SQuaternion::operator[](U8 index)
	{
		assert((index >= 0 && index < 4));
		
		switch (index)
		{
		case 0:
			return X;
		case 1:
			return Y;
		case 2:
			return Z;
		default:
			return W;
		}
	}

	inline const F32& SQuaternion::operator[](U8 index) const
	{
		assert((index >= 0 && index < 4));

		switch (index)
		{
		case 0:
			return X;
		case 1:
			return Y;
		case 2:
			return Z;
		default:
			return W;
		}
	}

	inline bool SQuaternion::Equals(const SQuaternion& other, F32 tolerance) const
	{
		return (UMath::Abs(X - other.X) <= tolerance && UMath::Abs(Y - other.Y) <= tolerance && UMath::Abs(Z - other.Z) <= tolerance && UMath::Abs(W - other.W) <= tolerance)
			|| (UMath::Abs(X + other.X) <= tolerance && UMath::Abs(Y + other.Y) <= tolerance && UMath::Abs(Z + other.Z) <= tolerance && UMath::Abs(W + other.W) <= tolerance);
	}
	
	inline bool SQuaternion::IsIdentity(F32 tolerance) const
	{
		return Equals(SQuaternion::Identity, tolerance);
	}
	
	inline SQuaternion SQuaternion::operator*(const SQuaternion& other) const
	{
		SVector vectorA = { X, Y, Z };
		SVector vectorB = { other.X, other.Y, other.Z };
		SVector resultingVector = W * vectorB + other.W * vectorA + vectorA.Cross(vectorB);
		F32 resultingScalar = W * other.W - vectorA.Dot(vectorB);

		SQuaternion resultingQuaternion = { resultingVector.X, resultingVector.Y, resultingVector.Z, resultingScalar };
		return resultingQuaternion.GetNormalized();
	}
	
	inline SQuaternion SQuaternion::operator*=(const SQuaternion& other)
	{
		*this = *this * other;
		return *this;
	}

	inline SQuaternion SQuaternion::operator+(const SQuaternion& other)
	{
		SQuaternion result = *this;
		result.X += other.X;
		result.Y += other.Y;
		result.Z += other.Z;
		result.W += other.W;

		return result;
	}

	inline SQuaternion SQuaternion::operator+=(const SQuaternion& other)
	{
		*this = *this + other;
		return *this;
	}

	inline SQuaternion SQuaternion::operator*(const F32 scale) const
	{
		SQuaternion result = *this;
		result.X *= scale;
		result.Y *= scale;
		result.Z *= scale;
		result.W *= scale;

		return result;
	}

	inline SQuaternion SQuaternion::operator*=(const F32 scale)
	{
		*this = *this * scale;
		return *this;
	}
	
	inline SVector SQuaternion::operator*(const SVector& vector) const
	{
		// http://people.csail.mit.edu/bkph/articles/Quaternions.pdf
		// V' = V + 2w(Q x V) + (2Q x (Q x V))
		// refactor:
		// V' = V + w(2(Q x V)) + (Q x (2(Q x V)))
		// T = 2(Q x V);
		// V' = V + w*(T) + (Q x T)

		const SVector quaternionVectorPart(X, Y, Z);
		const SVector T = 2.0f * quaternionVectorPart.Cross(vector);
		const SVector Result = vector + (W * T) + quaternionVectorPart.Cross(T);
		return Result;
	}
	
	inline SMatrix SQuaternion::operator*(const SMatrix& matrix) const
	{
		SMatrix result;
		SQuaternion vt, vr;
		SQuaternion inverse = Inverse();

		for (U8 index = 0; index < 4; ++index)
		{
			SQuaternion vq(matrix(index, 0), matrix(index, 1), matrix(index, 2), matrix(index, 3));
			vt = *this * vq;
			vr = vt * inverse;
			result(index, 0) = vr.X;
			result(index, 1) = vr.Y;
			result(index, 2) = vr.Z;
			result(index, 3) = vr.W;
		}

		return result;
	}
	
	inline SVector SQuaternion::ToEuler() const
	{
		SMatrix matrix = SMatrix::CreateRotationFromQuaternion(*this);
		return matrix.GetEuler();
	}
	
	inline void SQuaternion::ToAxisAndAngle(SVector& axis, F32& angle) const
	{
		angle = 2.0f * UMath::ACos(W);
	
		const F32 squareSum = X * X + Y * Y + Z * Z;
		if (squareSum < SMALL_NUMBER)
		{
			axis = SVector::Right;
		}
		else
		{
			const F32 scale = 1.0f / UMath::Sqrt(squareSum);
			axis = { X * scale, Y * scale, Z * scale };
		}
	}
	
	inline void SQuaternion::Normalize(F32 tolerance)
	{
		*this = GetNormalized(tolerance);
	}
	
	inline SQuaternion SQuaternion::GetNormalized(float tolerance) const
	{
		SQuaternion result;
		const F32 squareSum = X * X + Y * Y + Z * Z + W * W;

		if (squareSum >= tolerance)
		{
			const F32 scale = 1.0f / UMath::Sqrt(squareSum);

			result.X = X * scale;
			result.Y = Y * scale;
			result.Z = Z * scale;
			result.W = W * scale;
		}
		else
		{
			result = SQuaternion::Identity;
		}

		return result;
	}
	
	inline bool SQuaternion::IsNormalized() const
	{
		const F32 squareSum = X * X + Y * Y + Z * Z + W * W;
		return (UMath::Abs(1.0f - squareSum) < 0.01f);
	}
	
	inline SQuaternion SQuaternion::Inverse() const
	{
		return {-X, -Y, -Z, W};
	}
	
	inline F32 SQuaternion::AngularDistance(const SQuaternion& other) const
	{
		F32 innerProduct = X * other.X + Y * other.Y + Z * other.Z + W * other.W;
		
		// UE method
		//return UMath::ACos((2 * innerProduct * innerProduct) - 1.0f);

		// Game Engine Architecture, assuming normalized Quaternions
		return UMath::ACos(innerProduct);
	}
	
	inline std::string SQuaternion::ToString() const
	{
		char buffer[64];
		sprintf_s(buffer, "{X: %.1f, Y: %.1f, Z: %.1f, W: %.1f}", X, Y, Z, W);
		return buffer;
	}
	
	// NR: this doesn't work as intended, even when inverting the result. Will try to fix later
	//inline SQuaternion SQuaternion::Lerp(const SQuaternion& a, const SQuaternion& b, F32 t)
	//{
	//	// To ensure the 'shortest route', we make sure the dot product between the both rotations is positive.
	//	const F32 dotResult = a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;
	//	const F32 bias = UMath::FloatSelect(dotResult, 1.0f, -1.0f);

	//	return (b * t) + (a * (bias * (1.0f - t))).GetNormalized();
	//}
	
	inline SQuaternion SQuaternion::Slerp(const SQuaternion& a, const SQuaternion& b, F32 t)
	{
		if (a.Equals(b))
			return a;

		F32 angularDistance = a.AngularDistance(b);
		F32 sineOfAngularDistance = UMath::Sin(angularDistance);
		F32 omegaA = UMath::Sin((1.0f - t) * angularDistance) / sineOfAngularDistance;
		F32 omegaB = UMath::Sin(t * angularDistance) / sineOfAngularDistance;

		return (a * omegaA + b * omegaB).GetNormalized().Inverse();
	}
}