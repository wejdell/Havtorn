// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core/CoreTypes.h"
#include "EngineMath.h"
#include "Log.h"

#include <assert.h>

namespace Havtorn 
{
	struct SVector;
	struct SVector4;
	struct SQuaternion;

#define SMATRIX_MIN_SCALE 0.001f

	// Left-handed row-major
	struct SMatrix 
	{	
		union { __declspec(align(16)) F32 M[4][4]; F32 data[16];  };

		static const SMatrix Identity;
		static const SMatrix Zero;

		// Creates the identity matrix.
		inline SMatrix();
		// Copy Constructor.
		inline SMatrix(F32 diagonalElement);
		inline SMatrix(const SMatrix& matrix);
		// () operator for accessing element (row, column) for read/write or read, respectively.
		inline F32& operator()(const U8 row, const U8 column);
		inline const F32& operator()(const U8 row, const U8 column) const;
		// Static functions for creating rotation matrices.
		static SMatrix CreateRotationAroundX(F32 angleInRadians);
		static SMatrix CreateRotationAroundY(F32 angleInRadians);
		static SMatrix CreateRotationAroundZ(F32 angleInRadians);
		static SMatrix CreateRotationAroundAxis(F32 angleInRadians, const SVector& axis);
		static SMatrix CreateRotationFromEuler(F32 pitch, F32 yaw, F32 roll);
		static SMatrix CreateRotationFromEuler(const SVector& eulerAngles);
		static SMatrix CreateRotationFromQuaternion(const SQuaternion& quaternion);
		static SMatrix CreateRotationFromAxisAngle(const SVector& axis, F32 angleInRadians);
		// Static function for creating a transpose of a matrix.
		static SMatrix Transpose(const SMatrix& matrixToTranspose);
		// Negates rotation and negates + rotates translation
		inline SMatrix FastInverse() const;
		// Based on XMMatrixInverse
		inline SMatrix Inverse() const;

		static inline void Decompose(const SMatrix& matrix, SVector& translation, SVector& euler, SVector& scale);
		static inline void Decompose(const SMatrix& matrix, F32* translationData,  F32* eulerData,  F32* scaleData);
		static inline void Recompose(const SVector& translation, const SVector& euler, const SVector& scale, SMatrix& outMatrix);
		static inline void Recompose(const F32* translationData, const F32* eulerData, const F32* scaleData, SMatrix& outMatrix);
		
		inline SMatrix GetRHViewMatrix() const;
		inline SMatrix GetRHProjectionMatrix() const;

		inline SMatrix GetRotationMatrix() const;
		inline SVector GetEuler() const;
		inline void SetRotation(const SMatrix& matrix);
		inline void SetRotation(const SVector& eulerAngles);
		inline SMatrix GetTranslationMatrix() const;
		inline F32 GetRotationMatrixTrace() const;
		inline SMatrix GetScalingMatrix() const;
		inline SVector GetScale() const;
		inline void SetScale(const SVector& scale);
		inline void SetScale(F32 xScale, F32 yScale, F32 zScale);
		inline void SetScale(F32 scale);

		inline SMatrix operator+(const SMatrix& matrix);
		inline SMatrix& operator+=(const SMatrix& matrix);
		inline SMatrix operator-(const SMatrix& matrix);
		inline SMatrix& operator-=(const SMatrix& matrix);
		inline SMatrix operator*(const SMatrix& matrix);
		inline SMatrix& operator*=(const SMatrix& matrix);
		inline SVector4 operator*(const SVector4& vector);
		inline SMatrix& operator=(const SMatrix& matrix);
		inline SMatrix operator*(F32 scalar);
		inline SMatrix& operator*=(F32 scalar);
		inline friend SVector4 operator*(const SMatrix& matrix, const SVector4& vector);
		inline friend SVector4 operator*(const SVector4& vector, const SMatrix& matrix);

		inline SVector4 GetRow(U8 index) const;
		inline SVector4 GetColumn(U8 index) const;
		inline void SetRow(U8 index, const SVector4& vector);
		inline void SetColumn(U8 index, const SVector4& vector);

		inline SVector GetUp() const;
		inline SVector GetDown() const;
		inline SVector GetRight() const;
		inline SVector GetLeft() const;
		inline SVector GetForward() const;
		inline SVector GetBackward() const;
		inline SVector GetTranslation() const;
		inline SVector4 GetTranslation4() const;

		inline void SetUp(const SVector& v);
		inline void SetDown(const SVector& v);
		inline void SetRight(const SVector& v);
		inline void SetLeft(const SVector& v);
		inline void SetForward(const SVector& v);
		inline void SetBackward(const SVector& v);
		inline void SetTranslation(const SVector& v);
		inline void SetTranslation(const SVector4& v);

		inline void OrthoNormalize();

		inline bool operator==(const SMatrix& matrix) const;

		static SMatrix PerspectiveFovLH(F32 fovAngleY, F32 aspectRatio, F32 nearZ, F32 farZ);
		static SMatrix OrthographicLH(F32 viewWidth, F32 viewHeight, F32 nearZ, F32 farZ);
		static SMatrix LookAtLH(const SVector& eyePosition, const SVector& focusPosition, const SVector& upDirection);
		static SMatrix LookToLH(const SVector& eyePosition, const SVector& eyeDirection, const SVector& upDirection);
		// Modified version of LookAtLH for use with non-view transforms.
		static SMatrix Face(const SVector& position, const SVector& direction, const SVector& upDirection);
	};

	SMatrix::SMatrix()
	{
		for (U8 row = 0; row < 4; ++row)
		{
			for (U8 column = 0; column < 4; ++column)
			{
				M[row][column] = (row == column) ? 1.0f : 0.0f;
			}
		}
	}

	SMatrix::SMatrix(F32 diagonalElement)
	{
		for (U8 row = 0; row < 4; ++row)
		{
			for (U8 column = 0; column < 4; ++column)
			{
				M[row][column] = (row == column) ? diagonalElement : 0.0f;
			}
		}
	}

	SMatrix::SMatrix(const SMatrix& matrix)
	{
		for (U8 i = 0; i < 16; ++i)
		{
			data[i] = matrix.data[i];
		}
	}

	F32& SMatrix::operator()(const U8 row, const U8 column)
	{
		return M[row][column];
	}


	const F32& SMatrix::operator()(const U8 row, const U8 column) const
	{
		return M[row][column];
	}

	inline SMatrix SMatrix::CreateRotationAroundX(F32 angleInRadians)
	{
		SMatrix matrix = SMatrix();
		matrix(1, 1) = UMath::Cos(angleInRadians);
		matrix(1, 2) = UMath::Sin(angleInRadians);
		matrix(2, 1) = -UMath::Sin(angleInRadians);
		matrix(2, 2) = UMath::Cos(angleInRadians);
		return matrix;
	}

	inline SMatrix SMatrix::CreateRotationAroundY(F32 angleInRadians)
	{
		SMatrix matrix = SMatrix();
		matrix(0, 0) = UMath::Cos(angleInRadians);
		matrix(0, 2) = -UMath::Sin(angleInRadians);
		matrix(2, 0) = UMath::Sin(angleInRadians);
		matrix(2, 2) = UMath::Cos(angleInRadians);
		return matrix;
	}

	inline SMatrix SMatrix::CreateRotationAroundZ(F32 angleInRadians)
	{
		SMatrix matrix = SMatrix();
		matrix(0, 0) = UMath::Cos(angleInRadians);
		matrix(0, 1) = UMath::Sin(angleInRadians);
		matrix(1, 0) = -UMath::Sin(angleInRadians);
		matrix(1, 1) = UMath::Cos(angleInRadians);
		return matrix;
	}

	inline SMatrix SMatrix::CreateRotationAroundAxis(F32 angleInRadians, const SVector& axis)
	{
		F32 lengthSq = axis.LengthSquared();
		if (lengthSq < FLT_EPSILON)
			return SMatrix();
		
		SVector n = axis.GetNormalized();
		F32 cosTerm = UMath::Cos(angleInRadians);
		F32 sinTerm = UMath::Sin(angleInRadians);
		F32 oneMinusCos = 1.0f - cosTerm;

		F32 xx = n.X * n.X * oneMinusCos + cosTerm;
		F32 yy = n.Y * n.Y * oneMinusCos + cosTerm;
		F32 zz = n.Z * n.Z * oneMinusCos + cosTerm;
		F32 xy = n.X * n.Y * oneMinusCos;
		F32 yz = n.Y * n.Z * oneMinusCos;
		F32 zx = n.Z * n.X * oneMinusCos;
		F32 xs = n.X * sinTerm;
		F32 ys = n.Y * sinTerm;
		F32 zs = n.Z * sinTerm;

		SMatrix matrix;
		matrix(0, 0) = xx;
		matrix(0, 1) = xy - zs;
		matrix(0, 2) = zx + ys;
		matrix(0, 3) = 0.f;
		matrix(1, 0) = xy + zs;
		matrix(1, 1) = yy;
		matrix(1, 2) = yz - xs;
		matrix(1, 3) = 0.f;
		matrix(2, 0) = zx - ys;
		matrix(2, 1) = yz + xs;
		matrix(2, 2) = zz;
		matrix(2, 3) = 0.f;
		matrix(3, 0) = 0.f;
		matrix(3, 1) = 0.f;
		matrix(3, 2) = 0.f;
		matrix(3, 3) = 1.f;

		return matrix;
	}

	SMatrix SMatrix::GetRotationMatrix() const
	{
		SMatrix rotationMatrix = *this;
		rotationMatrix.OrthoNormalize();
		rotationMatrix.SetTranslation(SVector::Zero);
		return rotationMatrix;
	}
	inline SVector SMatrix::GetEuler() const
	{		
		SMatrix rotationMatrix = *this;
		rotationMatrix.OrthoNormalize();

		SVector euler;
		euler.X = UMath::RadToDeg(atan2f(rotationMatrix.M[1][2], rotationMatrix.M[2][2]));
		euler.Y = UMath::RadToDeg(atan2f(-rotationMatrix.M[0][2], sqrtf(rotationMatrix.M[1][2] * rotationMatrix.M[1][2] + rotationMatrix.M[2][2] * rotationMatrix.M[2][2])));
		euler.Z = UMath::RadToDeg(atan2f(rotationMatrix.M[0][1], rotationMatrix.M[0][0]));

		return euler;
	}

	inline void SMatrix::SetRotation(const SMatrix& matrix)
	{
		for (U8 row = 0; row < 3; ++row)
		{
			for (U8 column = 0; column < 3; ++column)
			{
				M[row][column] = matrix(row, column);
			}
		}
	}

	inline void SMatrix::SetRotation(const SVector& eulerAngles)
	{
		SMatrix rotationMatrix = SMatrix::CreateRotationFromEuler(eulerAngles.X, eulerAngles.Y, eulerAngles.Z);
		SetRotation(rotationMatrix);
	}

	SMatrix SMatrix::GetTranslationMatrix() const
	{
		SMatrix translationMatrix = SMatrix();
		translationMatrix.SetTranslation(this->GetTranslation());
		return translationMatrix;
	}

	inline F32 SMatrix::GetRotationMatrixTrace() const
	{
		return M[0][0] + M[1][1] + M[2][2];
	}

	inline SMatrix SMatrix::GetScalingMatrix() const
	{
		SMatrix scalingMatrix = SMatrix();
		scalingMatrix(0, 0) = GetRight().Length();
		scalingMatrix(1, 1) = GetUp().Length();
		scalingMatrix(2, 2) = GetForward().Length();
		return scalingMatrix;
	}

	inline SVector SMatrix::GetScale() const
	{
		SVector scale;
		scale.X = GetRight().Length();
		scale.Y = GetUp().Length();
		scale.Z = GetForward().Length();
		return scale;
	}

	inline void SMatrix::SetScale(const SVector& scale)
	{
		F32 validScale[3];
		validScale[0] = scale.X < FLT_EPSILON ? SMATRIX_MIN_SCALE : scale.X;
		validScale[1] = scale.Y < FLT_EPSILON ? SMATRIX_MIN_SCALE : scale.Y;
		validScale[2] = scale.Z < FLT_EPSILON ? SMATRIX_MIN_SCALE : scale.Z;

		M[0][0] = validScale[0];
		M[1][1] = validScale[1];
		M[2][2] = validScale[2];
	}

	inline void SMatrix::SetScale(F32 xScale, F32 yScale, F32 zScale)
	{
		SetScale({ xScale, yScale, zScale });
	}

	inline void SMatrix::SetScale(F32 scale)
	{
		SetScale({ scale, scale, scale });
	}

	SMatrix SMatrix::operator+(const SMatrix& matrix)
	{
		SMatrix result = SMatrix();
		for (U8 row = 0; row < 4; ++row)
		{
			for (U8 column = 0; column < 4; ++column)
			{
				result(row, column) = M[row][column] + matrix(row, column);
			}
		}
		return result;
	}


	SMatrix& SMatrix::operator+=(const SMatrix& matrix)
	{
		for (U8 row = 0; row < 4; ++row)
		{
			for (U8 column = 0; column < 4; ++column)
			{
				M[row][column] += matrix(row, column);
			}
		}
		return *this;
	}


	SMatrix SMatrix::operator-(const SMatrix& matrix)
	{
		SMatrix result = SMatrix();
		for (U8 row = 0; row < 4; ++row)
		{
			for (U8 column = 0; column < 4; ++column)
			{
				result(row, column) = M[row][column] - matrix(row, column);
			}
		}
		return result;
	}

	SMatrix& SMatrix::operator-=(const SMatrix& matrix)
	{
		for (U8 row = 0; row < 4; ++row)
		{
			for (U8 column = 0; column < 4; ++column)
			{
				M[row][column] -= matrix(row, column);
			}
		}
		return *this;
	}


	SMatrix SMatrix::operator*(const SMatrix& matrix)
	{
		SMatrix result = SMatrix();
		for (U8 row = 0; row < 4; ++row)
		{
			for (U8 column = 0; column < 4; ++column)
			{
				result(row, column) = M[row][0] * matrix(0, column);
				result(row, column) += M[row][1] * matrix(1, column);
				result(row, column) += M[row][2] * matrix(2, column);
				result(row, column) += M[row][3] * matrix(3, column);
			}
		}
		return result;
	}


	SMatrix& SMatrix::operator*=(const SMatrix& matrix)
	{
		SMatrix result = SMatrix();
		for (U8 row = 0; row < 4; ++row)
		{
			for (U8 column = 0; column < 4; ++column)
			{
				result(row, column) = M[row][0] * matrix(0, column);
				result(row, column) += M[row][1] * matrix(1, column);
				result(row, column) += M[row][2] * matrix(2, column);
				result(row, column) += M[row][3] * matrix(3, column);
			}
		}
		*this = result;
		return *this;
	}

	SVector4 SMatrix::operator*(const SVector4& vector)
	{
		SVector4 result;
		SVector4 temp1 = SVector4(M[0][0], M[1][0], M[2][0], M[3][0]);
		SVector4 temp2 = SVector4(M[0][1], M[1][1], M[2][1], M[3][1]);
		SVector4 temp3 = SVector4(M[0][2], M[1][2], M[2][2], M[3][2]);
		SVector4 temp4 = SVector4(M[0][3], M[1][3], M[2][3], M[3][3]);
		result.X = vector.Dot(temp1);
		result.Y = vector.Dot(temp2);
		result.Z = vector.Dot(temp3);
		result.W = vector.Dot(temp4);
		return result;
	}

	SMatrix& SMatrix::operator=(const SMatrix& matrix)
	{
		for (U8 row = 0; row < 4; ++row)
		{
			for (U8 column = 0; column < 4; ++column)
			{
				M[row][column] = matrix(row, column);
			}
		}
		return *this;
	}

	SMatrix SMatrix::operator*(F32 scalar)
	{
		SMatrix matrix = *this;
		for (U8 row = 0; row < 4; ++row)
		{
			for (U8 column = 0; column < 4; ++column)
			{
				matrix(row, column) *= scalar;
			}
		}
		return matrix;
	}

	SMatrix& SMatrix::operator*=(F32 scalar)
	{
		for (U8 row = 0; row < 4; ++row)
		{
			for (U8 column = 0; column < 4; ++column)
			{
				M[row][column] *= scalar;
			}
		}
		return *this;
	}


	bool SMatrix::operator==(const SMatrix& matrix) const
	{
		bool equal = true;
		for (U8 row = 0; row < 4; ++row)
		{
			for (U8 column = 0; column < 4; ++column)
			{
				if (M[row][column] != matrix(row, column))
				{
					equal = false;
				}
			}
		}
		return equal;
	}

	SVector4 SMatrix::GetRow(U8 index) const
	{
		return SVector4(M[index][0], M[index][1], M[index][2], M[index][3]);
	}

	SVector4 SMatrix::GetColumn(U8 index) const
	{
		return SVector4(M[0][index], M[1][index], M[2][index], M[3][index]);
	}

	inline void SMatrix::SetRow(U8 index, const SVector4& vector)
	{
		M[index][0] = vector.X;
		M[index][1] = vector.Y;
		M[index][2] = vector.Z;
		M[index][3] = vector.W;
	}

	inline void SMatrix::SetColumn(U8 index, const SVector4& vector)
	{
		M[0][index] = vector.X;
		M[1][index] = vector.Y;
		M[2][index] = vector.Z;
		M[3][index] = vector.W;
	}

	inline SVector SMatrix::GetUp() const
	{
		return SVector(M[1][0], M[1][1], M[1][2]);
	}

	inline SVector SMatrix::GetDown() const
	{
		return SVector(-M[1][0], -M[1][1], -M[1][2]);
	}

	inline SVector SMatrix::GetRight() const
	{
		return SVector(M[0][0], M[0][1], M[0][2]);
	}

	inline SVector SMatrix::GetLeft() const
	{
		return SVector(-M[0][0], -M[0][1], -M[0][2]);
	}

	inline SVector SMatrix::GetForward() const
	{
		return SVector(M[2][0], M[2][1], M[2][2]);
	}

	inline SVector SMatrix::GetBackward() const
	{
		return SVector(-M[2][0], -M[2][1], -M[2][2]);
	}

	inline SVector SMatrix::GetTranslation() const
	{
		return SVector(M[3][0], M[3][1], M[3][2]);
	}

	inline SVector4 SMatrix::GetTranslation4() const
	{
		return GetRow(3);
	}

	inline void SMatrix::SetUp(const SVector& v)
	{
		M[1][0] = v.X;
		M[1][1] = v.Y;
		M[1][2] = v.Z;
	}

	inline void SMatrix::SetDown(const SVector& v)
	{
		M[1][0] = -v.X;
		M[1][1] = -v.Y;
		M[1][2] = -v.Z;
	}

	inline void SMatrix::SetRight(const SVector& v)
	{
		M[0][0] = v.X;
		M[0][1] = v.Y;
		M[0][2] = v.Z;
	}

	inline void SMatrix::SetLeft(const SVector& v)
	{
		M[0][0] = -v.X;
		M[0][1] = -v.Y;
		M[0][2] = -v.Z;
	}

	inline void SMatrix::SetForward(const SVector& v)
	{
		M[2][0] = v.X;
		M[2][1] = v.Y;
		M[2][2] = v.Z;
	}

	inline void SMatrix::SetBackward(const SVector& v)
	{
		M[2][0] = -v.X;
		M[2][1] = -v.Y;
		M[2][2] = -v.Z;
	}

	inline void SMatrix::SetTranslation(const SVector& v)
	{
		M[3][0] = v.X;
		M[3][1] = v.Y;
		M[3][2] = v.Z;
	}

	inline void SMatrix::SetTranslation(const SVector4& v)
	{
		M[3][0] = v.X;
		M[3][1] = v.Y;
		M[3][2] = v.Z;
		M[3][3] = v.W;
	}

	inline void SMatrix::OrthoNormalize()
	{
		SetRight(GetRight().GetNormalized());
		SetUp(GetUp().GetNormalized());
		SetForward(GetForward().GetNormalized());
	}

	// Static function for creating a transpose of a matrix.
	inline SMatrix SMatrix::Transpose(const SMatrix& matrixToTranspose)
	{
		SMatrix matrix = SMatrix();
		for (U8 row = 0; row < 4; ++row)
		{
			for (U8 column = 0; column < 4; ++column)
			{
				matrix(row, column) = matrixToTranspose(column, row);
			}
		}
		return matrix;
	}

	// Only use this on matrices representing a 3x3 rotation and 3x1 translation
	inline SMatrix SMatrix::FastInverse() const
	{
		SMatrix rotation = Transpose(this->GetRotationMatrix());
		SVector4 translation = SVector4(this->GetTranslation(), 1.0f);
		translation *= -1.0f;
		translation.W *= -1.0f;
		translation = translation * rotation;

		SMatrix result;
		result.SetRotation(rotation);
		result.SetTranslation(translation);
		return result;
	}

	inline SMatrix SMatrix::Inverse() const
	{
		SMatrix matrixTranspose = Transpose(*this);

		SVector4 V0[4], V1[4];
		SVector4 row0 = matrixTranspose.GetRow(0);
		SVector4 row1 = matrixTranspose.GetRow(1);
		SVector4 row2 = matrixTranspose.GetRow(2);
		SVector4 row3 = matrixTranspose.GetRow(3);

		V0[0] = SVector4(row2.X, row2.X, row2.Y, row2.Y);
		V1[0] = SVector4(row3.Z, row3.W, row3.Z, row3.W);
		V0[1] = SVector4(row0.X, row0.X, row0.Y, row0.Y);
		V1[1] = SVector4(row1.Z, row1.W, row1.Z, row1.W);
		V0[2] = SVector4(row2.X, row2.Z, row0.X, row0.Z);
		V1[2] = SVector4(row3.Y, row3.W, row1.Y, row1.W);

		SVector4 D0 = V0[0] * V1[0];
		SVector4 D1 = V0[1] * V1[1];
		SVector4 D2 = V0[2] * V1[2];

		V0[0] = SVector4(row2.Z, row2.W, row2.Z, row2.W);
		V1[0] = SVector4(row3.X, row3.X, row3.Y, row3.Y);
		V0[1] = SVector4(row0.Z, row0.W, row0.Z, row0.W);
		V1[1] = SVector4(row1.X, row1.X, row1.Y, row1.Y);
		V0[2] = SVector4(row2.Y, row2.W, row0.Y, row0.W);
		V1[2] = SVector4(row3.X, row3.Z, row1.X, row1.Z);

		D0 = D0 - (V0[0] * V1[0]);
		D1 = D1 - (V0[1] * V1[1]);
		D2 = D2 - (V0[2] * V1[2]);

		V0[0] = SVector4(row1.Y, row1.Z, row1.X, row1.Y);
		V1[0] = SVector4(D2.Y, D0.Y, D0.W, D0.X);
		V0[1] = SVector4(row0.Z, row0.X, row0.Y, row0.X);
		V1[1] = SVector4(D0.W, D2.Y, D0.Y, D0.Z);
		V0[2] = SVector4(row3.Y, row3.Z, row3.X, row3.Y);
		V1[2] = SVector4(D2.W, D1.Y, D1.W, D1.X);
		V0[3] = SVector4(row2.Z, row2.X, row2.Y, row2.X);
		V1[3] = SVector4(D1.W, D2.W, D1.Y, D1.Z);

		SVector4 C0 = V0[0] * V1[0];
		SVector4 C2 = V0[1] * V1[1];
		SVector4 C4 = V0[2] * V1[2];
		SVector4 C6 = V0[3] * V1[3];

		V0[0] = SVector4(row1.Z, row1.W, row1.Y, row1.Z);
		V1[0] = SVector4(D0.W, D0.X, D0.Y, D2.X);
		V0[1] = SVector4(row0.W, row0.Z, row0.W, row0.Y);
		V1[1] = SVector4(D0.Z, D0.Y, D2.X, D0.X);
		V0[2] = SVector4(row3.Z, row3.W, row3.Y, row3.Z);
		V1[2] = SVector4(D1.W, D1.X, D1.Y, D2.Z);
		V0[3] = SVector4(row2.W, row2.Z, row2.W, row2.Y);
		V1[3] = SVector4(D1.Z, D1.Y, D2.Z, D1.X);

		C0 = C0 - (V0[0] * V1[0]);
		C2 = C2 - (V0[1] * V1[1]);
		C4 = C4 - (V0[2] * V1[2]);
		C6 = C6 - (V0[3] * V1[3]);

		V0[0] = SVector4(row1.W, row1.X, row1.W, row1.X);
		V1[0] = SVector4(D0.Z, D2.Y, D2.X, D0.Z);
		V0[1] = SVector4(row0.Y, row0.W, row0.X, row0.Z);
		V1[1] = SVector4(D2.Y, D0.X, D0.W, D2.X);
		V0[2] = SVector4(row3.W, row3.X, row3.W, row3.X);
		V1[2] = SVector4(D1.Z, D2.W, D2.Z, D1.Z);
		V0[3] = SVector4(row2.Y, row2.W, row2.X, row2.Z);
		V1[3] = SVector4(D2.W, D1.X, D1.W, D2.Z);

		SVector4 C1 = C0 - (V0[0] * V1[0]);
		C0 = (V0[0] * V1[0]) + C0;
		SVector4 C3 = (V0[1] * V1[1]) + C2;
		C2 = C2 - (V0[1] * V1[1]);
		SVector4 C5 = C4 - (V0[2] * V1[2]);
		C4 = (V0[2] * V1[2]) + C4;
		SVector4 C7 = (V0[3] * V1[3]) + C6;
		C6 = C6 - (V0[3] * V1[3]);

		SMatrix R;
		R.SetRow(0, { C0.X, C1.Y, C0.Z, C1.W });
		R.SetRow(1, { C2.X, C3.Y, C2.Z, C3.W });
		R.SetRow(2, { C4.X, C5.Y, C4.Z, C5.W });
		R.SetRow(3, { C6.X, C7.Y, C6.Z, C7.W });

		F32 determinant = R.GetRow(0).Dot(matrixTranspose.GetRow(0));
		SVector4 reciprocal = SVector4(SVector(1.0f / determinant), 1.0f / determinant);

		SMatrix result;
		result.SetRow(0, R.GetRow(0) * reciprocal);
		result.SetRow(1, R.GetRow(1) * reciprocal);
		result.SetRow(2, R.GetRow(2) * reciprocal);
		result.SetRow(3, R.GetRow(3) * reciprocal);
		return result;
	}

	inline void SMatrix::Decompose(const SMatrix& matrix, SVector& translation, SVector& euler, SVector& scale)
	{
		scale = matrix.GetScale();
		euler = matrix.GetEuler();
		translation = matrix.GetTranslation();	
	}

	inline void SMatrix::Decompose(const SMatrix& matrix, F32* translationData, F32* eulerData, F32* scaleData)
	{
		const SVector& t = matrix.GetTranslation();
		translationData[0] = t.X;
		translationData[1] = t.Y;
		translationData[2] = t.Z;

		const SVector& r = matrix.GetEuler();
		eulerData[0] = r.X;
		eulerData[1] = r.Y;
		eulerData[2] = r.Z;

		const SVector& s = matrix.GetScale();
		scaleData[0] = s.X;
		scaleData[1] = s.Y;
		scaleData[2] = s.Z;
	}
	
	inline void SMatrix::Recompose(const SVector& translation, const SVector& euler, const SVector& scale, SMatrix& outMatrix)
	{
		Recompose(&translation.X, &euler.X, &scale.X, outMatrix);
	}

	inline void SMatrix::Recompose(const F32* translationData, const F32* eulerData, const F32* scaleData, SMatrix& outMatrix) 
	{
		HV_ASSERT((translationData), "Passing nullptr [translationData] to Recompose!");
		HV_ASSERT((eulerData), "Passing nullptr [eulerData] to Recompose!");
		HV_ASSERT((scaleData), "Passing nullptr [scaleData] to Recompose!");

		SMatrix scaleMatrix;
		scaleMatrix.SetScale(scaleData[0], scaleData[1], scaleData[2]);
		outMatrix = scaleMatrix;

		// AG: Rotation Order XYZ used to not cause issues with ImGuizmo used in InspectorWindow.
		outMatrix *= CreateRotationAroundX(UMath::DegToRad(eulerData[0]));
		outMatrix *= CreateRotationAroundY(UMath::DegToRad(eulerData[1]));
		outMatrix *= CreateRotationAroundZ(UMath::DegToRad(eulerData[2]));

		outMatrix.SetTranslation(SVector(translationData[0], translationData[1], translationData[2]));
	}

	inline SMatrix SMatrix::GetRHViewMatrix() const
	{
		SMatrix result = *this;
		//result.Right(-1.0f * Right());
		result.SetForward(-1.0f * GetForward());
		SVector translation = result.GetTranslation();
		result.SetTranslation(SVector(translation.X, translation.Y, -translation.Z));
		//result.SetColumn(0, -1.0f * result.SetColumn(0));

		return result;
	}

	inline SMatrix SMatrix::GetRHProjectionMatrix() const
	{
		SMatrix result = *this;
		result(2, 2) *= -1.0f;
		result(3, 2) *= -1.0f;
		return result;
	}

	inline SMatrix SMatrix::LookAtLH(const SVector& eyePosition, const SVector& focusPosition, const SVector& upDirection)
	{
		SVector eyeDirection = focusPosition - eyePosition;
		return LookToLH(eyePosition, eyeDirection, upDirection);
	}

	inline SMatrix SMatrix::LookToLH(const SVector& eyePosition, const SVector& eyeDirection, const SVector& upDirection)
	{
		assert(!eyeDirection.IsEqual(SVector::Zero));
		assert(!upDirection.IsEqual(SVector::Zero));

		// Original
		SVector r2 = eyeDirection.GetNormalized();
		SVector r0 = upDirection.Cross(r2).GetNormalized();
		SVector r1 = r2.Cross(r0);
		r1 = r1.GetNormalized();

		SVector negEyePosition = -eyePosition;

		F32 d0 = r0.Dot(negEyePosition);
		F32 d1 = r1.Dot(negEyePosition);
		F32 d2 = r2.Dot(negEyePosition);

		SMatrix M;
		M.SetRight({ r0.X, r0.Y, r0.Z });
		M.SetUp({ r1.X, r1.Y, r1.Z });
		M.SetForward({ r2.X, r2.Y, r2.Z });
		M(0, 3) = d0;
		M(1, 3) = d1;
		M(2, 3) = d2;

		return Transpose(M);

		// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/bb281710(v=vs.85)
		// AG. Same as Original, but does not require Transpose().
		//const SVector zAxis = eyeDirection.GetNormalized();
		//const SVector xAxis = upDirection.Cross(zAxis).GetNormalized();
		//const SVector yAxis = zAxis.Cross(xAxis).GetNormalized();
		//
		//SMatrix m;
		//m.SetRight({ xAxis.X, yAxis.X, zAxis.X } );
		//m.SetUp({ xAxis.Y, yAxis.Y, zAxis.Y } );
		//m.SetForward({ xAxis.Z, yAxis.Z, zAxis.Z } );
		//m(3, 0) = -xAxis.Dot(eyePosition);
		//m(3, 1) = -yAxis.Dot(eyePosition);
		//m(3, 2) = -zAxis.Dot(eyePosition);
		//return m
	}

	inline SMatrix SMatrix::Face(const SVector& position, const SVector& direction, const SVector& upDirection) 
	{
		return Transpose(LookToLH(position, direction, upDirection));
	}

	inline SMatrix SMatrix::PerspectiveFovLH(F32 fovAngleY, F32 aspectRatio, F32 nearZ, F32 farZ)
	{
		assert(nearZ > 0.f && farZ > 0.f);
		assert(!UMath::NearlyEqual(fovAngleY, 0.0f));
		assert(!UMath::NearlyEqual(aspectRatio, 0.0f));
		assert(!UMath::NearlyEqual(farZ, nearZ));

		F32 sinFov;
		F32 cosFov;
		UMath::MapFov(sinFov, cosFov, 0.5f * fovAngleY);

		F32 height = cosFov / sinFov;
		F32 width = height / aspectRatio;
		F32 fRange = farZ / (farZ - nearZ);

		SMatrix matrix;
		matrix(0, 0) = width;
		matrix(0, 1) = 0.0f;
		matrix(0, 2) = 0.0f;
		matrix(0, 3) = 0.0f;

		matrix(1, 0) = 0.0f;
		matrix(1, 1) = height;
		matrix(1, 2) = 0.0f;
		matrix(1, 3) = 0.0f;

		matrix(2, 0) = 0.0f;
		matrix(2, 1) = 0.0f;
		matrix(2, 2) = fRange;
		matrix(2, 3) = 1.0f;

		matrix(3, 0) = 0.0f;
		matrix(3, 1) = 0.0f;
		matrix(3, 2) = fRange * nearZ * -1.0f;
		matrix(3, 3) = 0.0f;
		return matrix;
	}

	inline SMatrix SMatrix::OrthographicLH(F32 viewWidth, F32 viewHeight, F32 nearZ, F32 farZ)
	{
		assert(!UMath::NearlyEqual(viewWidth, 0.0f, 0.00001f));
		assert(!UMath::NearlyEqual(viewHeight, 0.0f, 0.00001f));
		assert(!UMath::NearlyEqual(farZ, nearZ, 0.00001f));

		F32 range = 1.0f / (farZ - nearZ);

		SMatrix matrix;
		matrix(0, 0) = 2.0f / viewWidth;
		matrix(0, 1) = 0.0f;
		matrix(0, 2) = 0.0f;
		matrix(0, 3) = 0.0f;
			  
		matrix(1, 0) = 0.0f;
		matrix(1, 1) = 2.0f / viewHeight;
		matrix(1, 2) = 0.0f;
		matrix(1, 3) = 0.0f;
			  
		matrix(2, 0) = 0.0f;
		matrix(2, 1) = 0.0f;
		matrix(2, 2) = range;
		matrix(2, 3) = 0.0f;
			  
		matrix(3, 0) = 0.0f;
		matrix(3, 1) = 0.0f;
		matrix(3, 2) = -range * nearZ;
		matrix(3, 3) = 1.0f;
		return matrix;
	}
}
