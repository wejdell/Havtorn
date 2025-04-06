// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "ModelImporter.h"

// Assimp
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Engine.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileHeaderDeclarations.h"

#include <../Editor/EditorManager.h>
#include <../Editor/EditorResourceManager.h>

#define NUM_BONES_PER_VERTEX 4
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

namespace Havtorn
{
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string& nodeName)
	{
		for (U32 i = 0; i < pAnimation->mNumChannels; ++i)
		{
			if (strcmp(pAnimation->mChannels[i]->mNodeName.C_Str(), nodeName.c_str()) == 0)
			{
				return pAnimation->mChannels[i];
			}
		}
		return NULL;
	}

	U32 FindRotation(F32 AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		assert(pNodeAnim->mNumRotationKeys > 0);

		for (U32 i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
		{
			if (AnimationTime < (F32)pNodeAnim->mRotationKeys[i + 1].mTime)
			{
				return i;
			}
		}
		// This is an 'ugly-fix'
		// In short: bypasses the error by returning the last working key
		return pNodeAnim->mNumRotationKeys - 2;

		//assert(0);
		//return 0xFFFFFFFF;
	}

	void CalcInterpolatedRotation(aiQuaternion& Out, F32 AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		// we need at least two values to interpolate...
		if (pNodeAnim->mNumRotationKeys == 1)
		{
			Out = pNodeAnim->mRotationKeys[0].mValue;
			return;
		}

		U32 RotationIndex = FindRotation(AnimationTime, pNodeAnim);
		U32 NextRotationIndex = (RotationIndex + 1);
		assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
		F32 DeltaTime = static_cast<F32>(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
		F32 Factor = (AnimationTime - (F32)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
		// This if just stops the assert below it from triggering. SP6 animations had some anims with issues and this was faster than having SG debug their animations.
		if (!(Factor >= 0.0f && Factor <= 1.0f))
		{
			Factor = 0.0f;
		}
		// ! If, that stops the assert below it 
		assert(Factor >= 0.0f && Factor <= 1.0f);
		aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
		StartRotationQ.w = UMath::NearlyZero(StartRotationQ.w, 1.e-3f) ? 0.0f : StartRotationQ.w;
		StartRotationQ.x = UMath::NearlyZero(StartRotationQ.x, 1.e-3f) ? 0.0f : StartRotationQ.x;
		StartRotationQ.y = UMath::NearlyZero(StartRotationQ.y, 1.e-3f) ? 0.0f : StartRotationQ.y;
		StartRotationQ.z = UMath::NearlyZero(StartRotationQ.z, 1.e-3f) ? 0.0f : StartRotationQ.z;
		aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
		EndRotationQ.w = UMath::NearlyZero(EndRotationQ.w, 1.e-3f) ? 0.0f : EndRotationQ.w;
		EndRotationQ.x = UMath::NearlyZero(EndRotationQ.x, 1.e-3f) ? 0.0f : EndRotationQ.x;
		EndRotationQ.y = UMath::NearlyZero(EndRotationQ.y, 1.e-3f) ? 0.0f : EndRotationQ.y;
		EndRotationQ.z = UMath::NearlyZero(EndRotationQ.z, 1.e-3f) ? 0.0f : EndRotationQ.z;
		aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
		Out = Out.Normalize();
	}

	U32 FindScaling(F32 AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		assert(pNodeAnim->mNumScalingKeys > 0);

		// 2021 02 02 Testing/ Figuring out animation speed
		//for (U32 i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
		//{
		//	std::cout << (F32)pNodeAnim->mScalingKeys[i + 1].mTime << std::endl;
		//}

		for (U32 i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
		{
			if (AnimationTime < (F32)pNodeAnim->mScalingKeys[i + 1].mTime)
			{
				return i;
			}
		}

		// This is an 'ugly-fix'
		// In short: bypasses the error by returning the last working key
		return pNodeAnim->mNumScalingKeys - 2;

		//assert(0);
		//return 0xFFFFFFFF;
	}

	void CalcInterpolatedScaling(aiVector3D& Out, F32 AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		// we need at least two values to interpolate...
		if (pNodeAnim->mNumScalingKeys == 1)
		{
			Out = pNodeAnim->mScalingKeys[0].mValue;
			return;
		}

		U32 ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
		U32 NextScalingIndex = (ScalingIndex + 1);
		assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
		F32 DeltaTime = static_cast<F32>(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
		F32 Factor = (AnimationTime - (F32)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
		// This if just stops the assert below it from triggering. SP6 animations had some anims with issues and this was faster than having SG debug their animations.
		if (!(Factor >= 0.0f && Factor <= 1.0f))
		{
			Factor = 0.0f;
		}
		// ! If, that stops the assert below it 
		assert(Factor >= 0.0f && Factor <= 1.0f);
		aiVector3D& StartScaling = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
		StartScaling.x = UMath::NearlyZero(StartScaling.x, 1.e-3f) ? 0.0f : StartScaling.x;
		StartScaling.y = UMath::NearlyZero(StartScaling.y, 1.e-3f) ? 0.0f : StartScaling.y;
		StartScaling.z = UMath::NearlyZero(StartScaling.z, 1.e-3f) ? 0.0f : StartScaling.z;
		aiVector3D& EndScaling = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
		EndScaling.x = UMath::NearlyZero(EndScaling.x, 1.e-3f) ? 0.0f : EndScaling.x;
		EndScaling.y = UMath::NearlyZero(EndScaling.y, 1.e-3f) ? 0.0f : EndScaling.y;
		EndScaling.z = UMath::NearlyZero(EndScaling.z, 1.e-3f) ? 0.0f : EndScaling.z;
		Out = StartScaling * (1 - Factor) + EndScaling * Factor;
	}

	U32 FindPosition(F32 AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		assert(pNodeAnim->mNumPositionKeys > 0);

		for (U32 i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
		{
			if (AnimationTime < (F32)pNodeAnim->mPositionKeys[i + 1].mTime)
			{
				return i;
			}
		}

		// This is an 'ugly-fix'
		// In short: bypasses the error by returning the last working key
		return pNodeAnim->mNumPositionKeys - 2;

		//assert(0);
		//return 0xFFFFFFFF;
	}

	void CalcInterpolatedPosition(aiVector3D& Out, F32 AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		// we need at least two values to interpolate...
		if (pNodeAnim->mNumPositionKeys == 1)
		{
			Out = pNodeAnim->mPositionKeys[0].mValue;
			return;
		}

		U32 PositionIndex = FindPosition(AnimationTime, pNodeAnim);
		U32 NextPositionIndex = (PositionIndex + 1);
		assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
		F32 DeltaTime = static_cast<F32>(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
		F32 Factor = (AnimationTime - (F32)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
		// This if just stops the assert below it from triggering. SP6 animations had some anims with issues and this was faster than having SG debug their animations.
		if (!(Factor >= 0.0f && Factor <= 1.0f))
		{
			Factor = 0.0f;
		}
		// ! If, that stops the assert below it 
		assert(Factor >= 0.0f && Factor <= 1.0f);
		aiVector3D& StartPosition = pNodeAnim->mPositionKeys[PositionIndex].mValue;
		StartPosition.x = UMath::NearlyZero(StartPosition.x, 1.e-3f) ? 0.0f : StartPosition.x;
		StartPosition.y = UMath::NearlyZero(StartPosition.y, 1.e-3f) ? 0.0f : StartPosition.y;
		StartPosition.z = UMath::NearlyZero(StartPosition.z, 1.e-3f) ? 0.0f : StartPosition.z;
		aiVector3D& EndPosition = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
		EndPosition.x = UMath::NearlyZero(EndPosition.x, 1.e-3f) ? 0.0f : EndPosition.x;
		EndPosition.y = UMath::NearlyZero(EndPosition.y, 1.e-3f) ? 0.0f : EndPosition.y;
		EndPosition.z = UMath::NearlyZero(EndPosition.z, 1.e-3f) ? 0.0f : EndPosition.z;
		Out = StartPosition * (1 - Factor) + EndPosition * Factor;
	}

	void InitM4FromM3(aiMatrix4x4& out, const aiMatrix3x3& in)
	{
		out.a1 = UMath::RoundToZero(in.a1, 1.e-3f); out.a2 = UMath::RoundToZero(in.a2, 1.e-3f); out.a3 = UMath::RoundToZero(in.a3, 1.e-3f); out.a4 = 0.f;
		out.b1 = UMath::RoundToZero(in.b1, 1.e-3f); out.b2 = UMath::RoundToZero(in.b2, 1.e-3f); out.b3 = UMath::RoundToZero(in.b3, 1.e-3f); out.b4 = 0.f;
		out.c1 = UMath::RoundToZero(in.c1, 1.e-3f); out.c2 = UMath::RoundToZero(in.c2, 1.e-3f); out.c3 = UMath::RoundToZero(in.c3, 1.e-3f); out.c4 = 0.f;
		out.d1 = 0.f;   out.d2 = 0.f;   out.d3 = 0.f;   out.d4 = 1.f;
	}


	void InitIdentityM4(aiMatrix4x4& m)
	{
		m.a1 = 1.f; m.a2 = 0.f; m.a3 = 0.f; m.a4 = 0.f;
		m.b1 = 0.f; m.b2 = 1.f; m.b3 = 0.f; m.b4 = 0.f;
		m.c1 = 0.f; m.c2 = 0.f; m.c3 = 1.f; m.c4 = 0.f;
		m.d1 = 0.f; m.d2 = 0.f; m.d3 = 0.f; m.d4 = 1.f;
		assert(m.IsIdentity());
	}

	void MulM4(aiMatrix4x4& out, aiMatrix4x4& in, float m)
	{
		out.a1 += in.a1 * m; out.a2 += in.a2 * m; out.a3 += in.a3 * m; out.a4 += in.a4 * m;
		out.b1 += in.b1 * m; out.b2 += in.b2 * m; out.b3 += in.b3 * m; out.b4 += in.b4 * m;
		out.c1 += in.c1 * m; out.c2 += in.c2 * m; out.c3 += in.c3 * m; out.c4 += in.c4 * m;
		out.d1 += in.d1 * m; out.d2 += in.d2 * m; out.d3 += in.d3 * m; out.d4 += in.d4 * m;
	}

	void ShortMulM4(aiVector3D& out, const aiMatrix4x4& m, const aiVector3D& in)
	{
		out.x = m.a1 * in.x + m.a2 * in.y + m.a3 * in.z;
		out.y = m.b1 * in.x + m.b2 * in.y + m.b3 * in.z;
		out.z = m.c1 * in.x + m.c2 * in.y + m.c3 * in.z;
	}

	SMatrix ToHavtornMatrix(const aiMatrix4x4& assimpMatrix)
	{
		SMatrix mat;
		mat(0, 0) = UMath::RoundToZero(assimpMatrix.a1, 1.e-3f); mat(0, 1) = UMath::RoundToZero(assimpMatrix.a2, 1.e-3f); mat(0, 2) = UMath::RoundToZero(assimpMatrix.a3, 1.e-3f); mat(0, 3) = UMath::RoundToZero(assimpMatrix.a4, 1.e-3f);
		mat(1, 0) = UMath::RoundToZero(assimpMatrix.b1, 1.e-3f); mat(1, 1) = UMath::RoundToZero(assimpMatrix.b2, 1.e-3f); mat(1, 2) = UMath::RoundToZero(assimpMatrix.b3, 1.e-3f); mat(1, 3) = UMath::RoundToZero(assimpMatrix.b4, 1.e-3f);
		mat(2, 0) = UMath::RoundToZero(assimpMatrix.c1, 1.e-3f); mat(2, 1) = UMath::RoundToZero(assimpMatrix.c2, 1.e-3f); mat(2, 2) = UMath::RoundToZero(assimpMatrix.c3, 1.e-3f); mat(2, 3) = UMath::RoundToZero(assimpMatrix.c4, 1.e-3f);
		mat(3, 0) = UMath::RoundToZero(assimpMatrix.d1, 1.e-3f); mat(3, 1) = UMath::RoundToZero(assimpMatrix.d2, 1.e-3f); mat(3, 2) = UMath::RoundToZero(assimpMatrix.d3, 1.e-3f); mat(3, 3) = UMath::RoundToZero(assimpMatrix.d4, 1.e-3f);
		return mat;
	}

	aiMatrix4x4 ToAssimpMatrix(const SMatrix& havtornMatrix)
	{
		aiMatrix4x4 mat;
		mat.a1 = havtornMatrix(0, 0); mat.a2 = havtornMatrix(0, 1); mat.a3 = havtornMatrix(0, 2); mat.a4 = havtornMatrix(0, 3);
		mat.b1 = havtornMatrix(1, 0); mat.b2 = havtornMatrix(1, 1); mat.b3 = havtornMatrix(1, 2); mat.b4 = havtornMatrix(1, 3);
		mat.c1 = havtornMatrix(2, 0); mat.c2 = havtornMatrix(2, 1); mat.c3 = havtornMatrix(2, 2); mat.c4 = havtornMatrix(2, 3);
		mat.d1 = havtornMatrix(3, 0); mat.d2 = havtornMatrix(3, 1); mat.d3 = havtornMatrix(3, 2); mat.d4 = havtornMatrix(3, 3);
		return mat;
	}

	void ReadNodeHeirarchy(const aiScene* aScene, U32 tick, F32 animationTime, F32 scale, const aiNode* aNode, const aiMatrix4x4& aParentTransform, int aStopAnimAtLevel, const std::vector<SSkeletalMeshBone>& bindPose, std::vector<SSkeletalMeshBone>& sequentialPosedBones)
	{
		std::string nodeName(aNode->mName.data);

		static U32 currentTick = 0;
		static U32 numberOfTreatedBones = 0;
		if (currentTick != tick)
			numberOfTreatedBones = 0;
		currentTick = tick;
		HV_LOG_WARN("Read Bone %i at tick %i: %s", numberOfTreatedBones, tick, nodeName.c_str());
		numberOfTreatedBones++;

		const aiAnimation* animation = aScene->mAnimations[0];

		aiMatrix4x4 NodeTransformation(aNode->mTransformation);

		const aiNodeAnim* nodeAnimation = FindNodeAnim(animation, nodeName);

		if (nodeAnimation)
		{
			// Interpolate scaling and generate scaling transformation matrix
			aiVector3D Scaling = {1.0f, 1.0f, 1.0f};
			//CalcInterpolatedScaling(Scaling, animationTime, nodeAnimation);
			aiMatrix4x4 ScalingM;
			aiMatrix4x4::Scaling(Scaling, ScalingM);

			// Interpolate rotation and generate rotation transformation matrix
			aiQuaternion RotationQ;
			CalcInterpolatedRotation(RotationQ, animationTime, nodeAnimation);
			aiMatrix4x4 RotationM;
			InitM4FromM3(RotationM, RotationQ.GetMatrix());

			// Interpolate translation and generate translation transformation matrix
			aiVector3D Translation;
			//{
				//F32 timeStop(aStopAnimAtLevel <= 0 ? animationTime : 0.f);
				CalcInterpolatedPosition(Translation, animationTime, nodeAnimation);
			//}
			aiMatrix4x4 TranslationM;
			aiMatrix4x4::Translation(Translation * scale, TranslationM);

			// Combine the above transformations
			NodeTransformation = TranslationM * RotationM * ScalingM;
		}
		aStopAnimAtLevel--;

		// GLobalTransformation is the joints animation for this frame. Multiply with the original joint orientation.
		aiMatrix4x4 GlobalTransformation;
		GlobalTransformation = aParentTransform * NodeTransformation;

		if (auto it = std::ranges::find(bindPose, nodeName, &SSkeletalMeshBone::Name); it != bindPose.end())
		{
			U64 BoneIndex = std::distance(std::begin(bindPose), it) + (tick * bindPose.size());
			
			SMatrix bindPoseTransform = it->InverseBindPoseTransform;
			SMatrix globalTransform = ToHavtornMatrix(GlobalTransformation);
			SMatrix globalInverseTransform = ToHavtornMatrix(aScene->mRootNode->mTransformation.Inverse());
			
			//HV_LOG_FATAL("Bone index: %i", BoneIndex);
			//HV_LOG_FATAL("Bind Pose: %s", bindPoseTransform.ToString().c_str());
			//HV_LOG_FATAL("Global Transform: %s", globalTransform.ToString().c_str());
			//HV_LOG_FATAL("Root Node Inverse: %s", globalInverseTransform.ToString().c_str());

			//sequentialPosedBones[BoneIndex].Transform = globalInverseTransform * globalTransform * bindPoseTransform;
			//sequentialPosedBones[BoneIndex].Transform = bindPoseTransform * globalTransform * globalInverseTransform;
			auto rootNodeInverse = aScene->mRootNode->mTransformation.Inverse();
			auto globalTransformation = GlobalTransformation;
			auto boneOffset = ToAssimpMatrix(bindPoseTransform);
			auto finalTransform = rootNodeInverse * GlobalTransformation * boneOffset;
			auto havtornFinalTransform = SMatrix::Transpose(ToHavtornMatrix(finalTransform));
			HV_LOG_FATAL("Final Transform: %s", havtornFinalTransform.ToString().c_str());
			sequentialPosedBones[BoneIndex].InverseBindPoseTransform = havtornFinalTransform;
			sequentialPosedBones[BoneIndex].Name = nodeName;
		}
		
		for (U32 i = 0; i < aNode->mNumChildren; i++)
		{
			ReadNodeHeirarchy(aScene, tick, animationTime, scale, aNode->mChildren[i], GlobalTransformation, aStopAnimAtLevel, bindPose, sequentialPosedBones);
		}

		//for (U32 i = 0; i < aNode->mNumChildren; i++)
		//{
		//	if (auto it = std::ranges::find(bindPose, std::string(aNode->mChildren[i]->mName.C_Str()), &SSkeletalMeshBone::Name); it != bindPose.end())
		//	{
		//		ReadNodeHeirarchy(aScene, tick, animationTime, scale, aNode->mChildren[i], GlobalTransformation, aStopAnimAtLevel, bindPose, sequentialPosedBones);
		//	}
		//	else
		//	{
		//		aiNode* childNode = aNode->mChildren[i];
		//		for (U32 j = 0; j < childNode->mNumChildren; j++)
		//		{
		//			if (auto childIt = std::ranges::find(bindPose, std::string(childNode->mChildren[j]->mName.C_Str()), &SSkeletalMeshBone::Name); childIt != bindPose.end())
		//			{
		//				ReadNodeHeirarchy(aScene, tick, animationTime, scale, childNode->mChildren[j], GlobalTransformation, aStopAnimAtLevel, bindPose, sequentialPosedBones);
		//			}
		//		}
		//	}
		//}
	}

	/////////////////////////////////////////////
	/////////////////////////////////////////////

	struct SVertexBoneData
	{
		U32 IDs[NUM_BONES_PER_VERTEX];
		F32 Weights[NUM_BONES_PER_VERTEX];

		SVertexBoneData()
		{
			Reset();
		};

		void Reset()
		{
			ZERO_MEM(IDs);
			ZERO_MEM(Weights);
		}

		void AddBoneData(U32 boneID, F32 weight)
		{
			//F32 smallestWeight = UMath::MaxFloat;
			//U32 smallestIndex = 0;
			for (U32 i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++) 
			{
				if (Weights[i] == 0.0) 
				{
					IDs[i] = boneID;
					Weights[i] = weight;
					return;
				}

				// NW: Algo to find any element smaller than weight
				
				//if (smallestWeight > Weights[i])
				//{
				//	smallestWeight = Weights[i];
				//	smallestIndex = i;
				//}
				
			}

			//if (weight > smallestWeight)
			//{
			//	IDs[smallestIndex] = boneID;
			//	Weights[smallestIndex] = weight;
			//}

			// should never get here - more bones than we have space for
			//assert(0);
		}
	};

	SVecBoneAnimationKey ToHavtornVecAnimationKey(const aiVectorKey& assimpKey)
	{
		SVecBoneAnimationKey havtornKey;
		havtornKey.Value = { assimpKey.mValue.x, assimpKey.mValue.y, assimpKey.mValue.z };
		havtornKey.Time = STATIC_F32(assimpKey.mTime);
		return havtornKey;
	}

	SQuatBoneAnimationKey ToHavtornQuatAnimationKey(const aiQuatKey& assimpKey)
	{
		SQuatBoneAnimationKey havtornKey;
		havtornKey.Value = { assimpKey.mValue.x, assimpKey.mValue.y, assimpKey.mValue.z, assimpKey.mValue.w };
		havtornKey.Time = STATIC_F32(assimpKey.mTime);
		return havtornKey;
	}

	SBoneAnimationKey ToHavtornBoneAnimationKey(const aiVectorKey& translationKey, const aiQuatKey& rotationKey, const aiVectorKey& scaleKey, const F32 time)
	{
		SBoneAnimationKey havtornKey;
		SMatrix::Recompose(ToHavtornVecAnimationKey(translationKey).Value, ToHavtornQuatAnimationKey(rotationKey).Value.ToEuler(), ToHavtornVecAnimationKey(scaleKey).Value, havtornKey.Transform);
		havtornKey.Time = time;
		return havtornKey;
	}

	std::string UModelImporter::ImportFBX(const std::string& filePath, const std::string& destinationPath, const SAssetImportOptions& importOptions)
	{
		if (!CFileSystem::DoesFileExist(filePath))
		{
			HV_LOG_ERROR("ModelImporter could not import %s. File does not exist!", filePath.c_str());
			return "ERROR: File does not exist.";
		}

		const aiScene* assimpScene = aiImportFile(filePath.c_str(), aiProcess_PopulateArmatureData | aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded);
		
		if (!assimpScene)
		{
			HV_LOG_ERROR("ModelImporter failed to import %s! Assimp Error: %s", filePath.c_str(), aiGetErrorString());
			return "ERROR: Failed to import.";
		}

		if (importOptions.AssetType == EAssetType::Animation)
		{
			if (!assimpScene->HasAnimations() /*|| assimpScene->HasMeshes()*/)
			{
				HV_LOG_ERROR("ModelImporter expected %s to be an animation file, but it either has no animations or contains meshes!", filePath.c_str());
				return "ERROR: Failed to import.";
			}

			return ImportAnimation(filePath, destinationPath, importOptions, assimpScene);
		}

		if (!assimpScene->HasMeshes())
		{
			HV_LOG_ERROR("ModelImporter expected %s to be a mesh file, but it has no meshes!", filePath.c_str());
			return "ERROR: Failed to import";
		}

		const aiMesh* fbxMesh = assimpScene->mMeshes[0];

		const bool hasPositions = fbxMesh->HasPositions();
		const bool hasNormals = fbxMesh->HasNormals();
		const bool hasTangents = fbxMesh->HasTangentsAndBitangents();
		const bool hasTextures = fbxMesh->HasTextureCoords(0);
		const bool hasBones = fbxMesh->HasBones();

		if (!hasPositions || !hasNormals || !hasTangents || !hasTextures)
		{
			HV_LOG_ERROR("ModelImporter expected %s to be a mesh file, but it is lacking position, normal, tangent or UV information!", filePath.c_str());
			return "ERROR: Failed to import";
		}

		if (importOptions.AssetType == EAssetType::SkeletalMesh)
		{
			if (!hasBones)
			{
				HV_LOG_ERROR("ModelImporter expected %s to be a skeletal mesh file, but it lacks bone information!", filePath.c_str());
				return "ERROR: Failed to import";
			}

			return ImportSkeletalMesh(filePath, destinationPath, importOptions, assimpScene);
		}

		// Static Mesh
		if (hasBones)
			HV_LOG_WARN("ModelImporter expected %s to be a static mesh file, but it contains bone information!", filePath.c_str());

		return ImportStaticMesh(filePath, destinationPath, importOptions, assimpScene);
	}

	std::string UModelImporter::ImportStaticMesh(const std::string& filePath, const std::string& destinationPath, const SAssetImportOptions& importOptions, const aiScene* assimpScene)
	{
		SStaticModelFileHeader fileHeader;
		fileHeader.AssetType = EAssetType::StaticMesh;
		fileHeader.Name = assimpScene->mName.C_Str();
		fileHeader.NumberOfMeshes = assimpScene->mNumMeshes;
		fileHeader.Meshes.reserve(fileHeader.NumberOfMeshes);

		const aiMesh* fbxMesh = assimpScene->mMeshes[0];

		// Pre-loading Pass
		for (U8 n = 0; n < assimpScene->mNumMeshes; n++)
		{
			fileHeader.Meshes.emplace_back();
			fbxMesh = assimpScene->mMeshes[n];
			fileHeader.Meshes[n].Vertices.reserve(fbxMesh->mNumVertices);
		}

		for (U8 n = 0; n < assimpScene->mNumMeshes; n++)
		{
			auto& fileHeaderMesh = fileHeader.Meshes[n];
			fbxMesh = assimpScene->mMeshes[n];

			// Vertices
			const F32 scaleModifier = importOptions.Scale;
			for (U32 i = 0; i < fbxMesh->mNumVertices; i++)
			{
				SStaticMeshVertex newVertex;

				aiVector3D& pos = fbxMesh->mVertices[i];
				pos *= scaleModifier;
				newVertex.x = pos.x;
				newVertex.y = pos.y;
				newVertex.z = pos.z;

				const aiVector3D& norm = fbxMesh->mNormals[i];
				newVertex.nx = norm.x;
				newVertex.ny = norm.y;
				newVertex.nz = norm.z;

				const aiVector3D& tangent = fbxMesh->mTangents[i];
				newVertex.tx = tangent.x;
				newVertex.ty = tangent.y;
				newVertex.tz = tangent.z;

				const aiVector3D& biTangent = fbxMesh->mBitangents[i];
				newVertex.bx = biTangent.x;
				newVertex.by = biTangent.y;
				newVertex.bz = biTangent.z;

				newVertex.u = fbxMesh->mTextureCoords[0][i].x;
				newVertex.v = fbxMesh->mTextureCoords[0][i].y;

				fileHeaderMesh.Vertices.emplace_back(newVertex);
			}

			// Indices
			for (U32 i = 0; i < fbxMesh->mNumFaces; i++)
			{
				fileHeaderMesh.Indices.insert(fileHeaderMesh.Indices.end(), std::make_move_iterator(&fbxMesh->mFaces[i].mIndices[0]), std::make_move_iterator(&fbxMesh->mFaces[i].mIndices[fbxMesh->mFaces[i].mNumIndices]));
			}
		}

		// Material Count
		fileHeader.NumberOfMaterials = STATIC_U8(assimpScene->mNumMaterials);

		std::string newFileName = destinationPath + UGeneralUtils::ExtractFileBaseNameFromPath(filePath) + ".hva";
		const auto fileData = new char[fileHeader.GetSize()];
		fileHeader.Serialize(fileData);
		GEngine::GetFileSystem()->Serialize(newFileName, &fileData[0], fileHeader.GetSize());
		return newFileName;
	}

	void ExtractNodes(aiNode* node, const SMatrix& /*parentTransform*/, const std::vector<SSkeletalMeshBone>& bindPose, std::vector<SSkeletalMeshNode>& nodesToPopulate)
	{
		//static U32 index = 0;
		//CHavtornStaticString<32> nodeName = CHavtornStaticString<32>(node->mName.C_Str());
		//SMatrix scaledMatrix = SMatrix::Transpose(ToHavtornMatrix(node->mTransformation));
		//SQuaternion rot = SQuaternion::Identity;
		//SVector pos = SVector::Zero;
		//SVector scl = SVector(1.0f);
		//SMatrix::Decompose(scaledMatrix, pos, rot, scl);
		//SMatrix unScaledMatrix = SMatrix::Identity; 
		//SMatrix::Recompose(pos, rot, SVector(1.0f), unScaledMatrix);
		//SMatrix transform = unScaledMatrix * parentTransform;
		//
		//if (auto it = std::ranges::find(bindPose, nodeName, &SSkeletalMeshBone::Name); it != bindPose.end())
		//{
		//	HV_LOG_ERROR("Node %i: %s, %s", ++index, nodeName.AsString().c_str(), transform.ToString().c_str());
		//	nodesToPopulate.emplace_back(SSkeletalMeshNode(nodeName, transform));
		//}

		//for (U32 i = 0; i < node->mNumChildren; i++)
		//	ExtractNodes(node->mChildren[i], transform, bindPose, nodesToPopulate);

		static U32 index = 0;
		CHavtornStaticString<32> nodeName = CHavtornStaticString<32>(node->mName.C_Str());
		SMatrix scaledMatrix = SMatrix::Transpose(ToHavtornMatrix(node->mTransformation));
		SQuaternion rot = SQuaternion::Identity;
		SVector pos = SVector::Zero;
		SVector scl = SVector(1.0f);
		SMatrix::Decompose(scaledMatrix, pos, rot, scl);
		SMatrix unScaledMatrix = SMatrix::Identity;
		SMatrix::Recompose(pos, rot, SVector(1.0f), unScaledMatrix);
		//SMatrix transform = unScaledMatrix * parentTransform;
		SMatrix transform = unScaledMatrix;
		
		if (nodesToPopulate.size() > 0)
		{
			CHavtornStaticString<32> parentName = node->mParent ? std::string(node->mParent->mName.data) : "";
			//for (U64 j = 0; j < nodesToPopulate.size(); j++)
			//{
			//	if (parentName == nodesToPopulate[j].Name.AsString())
			//}
			if (auto it = std::ranges::find(nodesToPopulate, parentName, &SSkeletalMeshNode::Name); it != nodesToPopulate.end())
			{
				U32 childIndex = STATIC_U32(std::distance(std::begin(nodesToPopulate), it));
				nodesToPopulate[childIndex].ChildIndices.push_back(STATIC_U32(nodesToPopulate.size()));
			}
		}

		HV_LOG_ERROR("Node %i: %s, %s", ++index, nodeName.AsString().c_str(), transform.ToString().c_str());
		nodesToPopulate.emplace_back(SSkeletalMeshNode(nodeName, transform, {}));


		for (U32 i = 0; i < node->mNumChildren; i++)
			ExtractNodes(node->mChildren[i], transform, bindPose, nodesToPopulate);
	}

	std::string UModelImporter::ImportSkeletalMesh(const std::string& filePath, const std::string& destinationPath, const SAssetImportOptions& importOptions, const aiScene* assimpScene)
	{
		SSkeletalModelFileHeader fileHeader;
		fileHeader.AssetType = EAssetType::SkeletalMesh;
		fileHeader.Name = assimpScene->mName.C_Str();
		fileHeader.NumberOfMeshes = assimpScene->mNumMeshes;
		fileHeader.Meshes.reserve(fileHeader.NumberOfMeshes);

		const aiMesh* fbxMesh = nullptr;

		// Pre-loading Pass
		for (U8 n = 0; n < assimpScene->mNumMeshes; n++)
		{
			fileHeader.Meshes.emplace_back();
			fbxMesh = assimpScene->mMeshes[n];
			fileHeader.Meshes[n].Vertices.reserve(fbxMesh->mNumVertices);
		}

		for (U8 n = 0; n < assimpScene->mNumMeshes; n++)
		{
			auto& fileHeaderMesh = fileHeader.Meshes[n];
			fbxMesh = assimpScene->mMeshes[n];
			
			// Bone pre-loading
			std::vector<SVertexBoneData> collectedBoneData;
			collectedBoneData.resize(fbxMesh->mNumVertices);
			std::map<std::string, U32> tempBoneNameToIndexMap;

			U32 boneIndex = 0;
			U32 numBones = 0;
			for (U32 i = 0; i < fbxMesh->mNumBones; i++)
			{
				std::string boneName(fbxMesh->mBones[i]->mName.data);
				if (tempBoneNameToIndexMap.find(boneName) == tempBoneNameToIndexMap.end())
				{
					boneIndex = numBones++;

					tempBoneNameToIndexMap[boneName] = boneIndex;

					std::string parentName = fbxMesh->mBones[i]->mNode->mParent ? std::string(fbxMesh->mBones[i]->mNode->mParent->mName.data) : "";
					I32 parentIndex = tempBoneNameToIndexMap.contains(parentName) ? tempBoneNameToIndexMap[parentName] : -1;

					// Mesh Space -> Bone Space in Bind Pose, [INVERSE BIND MATRIX]. Use with bone [WORLD SPACE TRANSFORM] to find vertex pos
					fileHeader.BindPoseBones.push_back(SSkeletalMeshBone(boneName, SMatrix::Transpose(ToHavtornMatrix(fbxMesh->mBones[i]->mOffsetMatrix)), parentIndex));
					//HV_LOG_ERROR("Node %i: %s, %s", i+1, fbxMesh->mBones[i]->mNode->mName.C_Str(), SMatrix::Transpose(ToHavtornMatrix(fbxMesh->mBones[i]->mNode->mTransformation)).ToString().c_str());
				}
				else
				{
					boneIndex = tempBoneNameToIndexMap[boneName];
				}

				for (U32 j = 0; j < STATIC_U32(fbxMesh->mBones[i]->mNumWeights); j++)
				{
					U32 vertexID = fbxMesh->mBones[i]->mWeights[j].mVertexId;
					F32 weight = fbxMesh->mBones[i]->mWeights[j].mWeight;
					collectedBoneData[vertexID].AddBoneData(boneIndex, weight);
				}
			}

			ExtractNodes(assimpScene->mRootNode, SMatrix::Identity, fileHeader.BindPoseBones, fileHeader.Nodes);

			// Vertices
			const F32 scaleModifier = importOptions.Scale;
			for (U32 i = 0; i < fbxMesh->mNumVertices; i++)
			{
				SSkeletalMeshVertex newVertex;

				aiVector3D& pos = fbxMesh->mVertices[i];
				pos *= scaleModifier;
				newVertex.x = pos.x;
				newVertex.y = pos.y;
				newVertex.z = pos.z;

				const aiVector3D& norm = fbxMesh->mNormals[i];
				newVertex.nx = norm.x;
				newVertex.ny = norm.y;
				newVertex.nz = norm.z;

				const aiVector3D& tangent = fbxMesh->mTangents[i];
				newVertex.tx = tangent.x;
				newVertex.ty = tangent.y;
				newVertex.tz = tangent.z;

				const aiVector3D& biTangent = fbxMesh->mBitangents[i];
				newVertex.bx = biTangent.x;
				newVertex.by = biTangent.y;
				newVertex.bz = biTangent.z;

				newVertex.u = fbxMesh->mTextureCoords[0][i].x;
				newVertex.v = fbxMesh->mTextureCoords[0][i].y;

				const SVertexBoneData& boneData = collectedBoneData[i];
				newVertex.bix = STATIC_F32(boneData.IDs[0]);
				newVertex.biy = STATIC_F32(boneData.IDs[1]);
				newVertex.biz = STATIC_F32(boneData.IDs[2]);
				newVertex.biw = STATIC_F32(boneData.IDs[3]);

				newVertex.bwx = boneData.Weights[0];
				newVertex.bwy = boneData.Weights[1];
				newVertex.bwz = boneData.Weights[2];
				newVertex.bww = boneData.Weights[3];

				fileHeaderMesh.Vertices.emplace_back(newVertex);
			}

			// Indices
			for (U32 i = 0; i < fbxMesh->mNumFaces; i++)
			{
				fileHeaderMesh.Indices.insert(fileHeaderMesh.Indices.end(), std::make_move_iterator(&fbxMesh->mFaces[i].mIndices[0]), std::make_move_iterator(&fbxMesh->mFaces[i].mIndices[fbxMesh->mFaces[i].mNumIndices]));
			}
		}

		// Material Count
		fileHeader.NumberOfMaterials = STATIC_U8(assimpScene->mNumMaterials);
		fileHeader.NumberOfNodes = STATIC_U32(fileHeader.Nodes.size());

		for (const auto& bone : fileHeader.BindPoseBones)
		{
			HV_LOG_WARN("Bone %s, %s", bone.Name.AsString().c_str(), bone.InverseBindPoseTransform.ToString().c_str());
		}
		std::string newFileName = destinationPath + UGeneralUtils::ExtractFileBaseNameFromPath(filePath) + ".hva";
		const auto fileData = new char[fileHeader.GetSize()];
		fileHeader.Serialize(fileData);
		GEngine::GetFileSystem()->Serialize(newFileName, &fileData[0], fileHeader.GetSize());
		return newFileName;
	}

	std::string UModelImporter::ImportAnimation(const std::string& filePath, const std::string& destinationPath, const SAssetImportOptions& importOptions, const aiScene* assimpScene)
	{
		const aiAnimation* animation = assimpScene->mAnimations[0];

		// TODO.NR: Support multiple animations per file? Support montages somehow. Could be separate file using these headers (SSkeletalAnimationMontageFileHeader)
		SSkeletalAnimationFileHeader fileHeader;
		fileHeader.AssetType = EAssetType::Animation;
		fileHeader.Name = UGeneralUtils::ExtractFileNameFromPath(filePath);
		fileHeader.DurationInTicks = STATIC_U32(animation->mDuration);
		fileHeader.TickRate = STATIC_U32(animation->mTicksPerSecond);
			
		std::vector<SSkeletalMeshBone> bones;
		{
			// Need full path here, not just filename
			std::string rigFilePath = importOptions.AssetRep->DirectoryEntry.path().string();
			const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(rigFilePath);
			char* data = new char[fileSize];

			GEngine::GetFileSystem()->Deserialize(rigFilePath, data, STATIC_U32(fileSize));

			SSkeletalModelFileHeader rigHeader;
			rigHeader.Deserialize(data);

			bones = rigHeader.BindPoseBones;

			delete[] data;
		}

		fileHeader.NumberOfBones = STATIC_U32(bones.size());
		//fileHeader.SequentialPosedBones.resize(fileHeader.NumberOfBones);
		//fileHeader.BoneAnimationTracks.resize(fileHeader.NumberOfBones);

		const F32 scaleModifier = importOptions.Scale;

		for (const SSkeletalMeshBone& bone : bones)
		{
			//const aiNodeAnim* nodeAnimation = FindNodeAnim(animation, bone.Name.AsString());


			std::string channelName = bone.Name.AsString();
			const aiNodeAnim* channel = nullptr;

			for (U32 i = 0; i < animation->mNumChannels; i++)
			{
				if (strcmp(animation->mChannels[i]->mNodeName.C_Str(), channelName.c_str()) != 0)
					continue;

				channel = animation->mChannels[i];
				break;
			}
	
			fileHeader.BoneAnimationTracks.emplace_back();
			SBoneAnimationTrack& track = fileHeader.BoneAnimationTracks.back();
			track.BoneName = bone.Name.AsString();

			if (channel != nullptr)
			{
				for (U32 t = 0; t < channel->mNumPositionKeys; t++)
				{
					track.TranslationKeys.emplace_back(ToHavtornVecAnimationKey(channel->mPositionKeys[t]));
					track.TranslationKeys.back().Value *= scaleModifier;
				}

				for (U32 q = 0; q < channel->mNumRotationKeys; q++)
					track.RotationKeys.emplace_back(ToHavtornQuatAnimationKey(channel->mRotationKeys[q]));

				for (U32 s = 0; s < channel->mNumScalingKeys; s++)
					track.ScaleKeys.emplace_back(ToHavtornVecAnimationKey(channel->mScalingKeys[s]));
			}
		}
		
		//for (U64 i = 0; i < assimpScene->mRootNode.chi)
		//{
		//	HV_LOG_INFO("AnimTrack: %s", track.BoneName.c_str());
		//}
		for (auto track : fileHeader.BoneAnimationTracks)
		{
			HV_LOG_INFO("AnimTrack: %s", track.BoneName.c_str());
		}

		std::string newFileName = destinationPath + UGeneralUtils::ExtractFileBaseNameFromPath(filePath) + ".hva";
		const auto fileData = new char[fileHeader.GetSize()];
		fileHeader.Serialize(fileData);
		GEngine::GetFileSystem()->Serialize(newFileName, &fileData[0], fileHeader.GetSize());
		return newFileName;
	}
}

		//	bool hasPositions = false;
		//	bool hasNormals = false;
		//	bool hasTangents = false;
		//	bool hasTextures = false;
		//	bool hasBones = false;

		//	hasPositions = fbxMesh->HasPositions();
		//	hasNormals = fbxMesh->HasNormals();
		//	hasTangents = fbxMesh->HasTangentsAndBitangents();
		//	hasTextures = fbxMesh->HasTextureCoords(TEXTURE_SET_0);
		//	hasBones = fbxMesh->HasBones();

		//	F32* data = new F32[(vertexBufferSize / 4) * fbxMesh->mNumVertices];
		//	if (hasPositions && hasNormals && hasTangents && hasTextures && hasBones) 
		//	{
		//		for (unsigned int i = 0, dataIndex = 0; i < fbxMesh->mNumVertices; i++, dataIndex += (vertexBufferSize / 4)) 
		//		{

		//			aiVector3D& mVertice = fbxMesh->mVertices[i];
		//			data[dataIndex] = mVertice.x;
		//			data[dataIndex + 1] = mVertice.y;
		//			data[dataIndex + 2] = mVertice.z;
		//			data[dataIndex + 3] = 1.0f;

		//			aiVector3D& mNorm = fbxMesh->mNormals[i];
		//			data[dataIndex + 4] = mNorm.x;
		//			data[dataIndex + 5] = mNorm.y;
		//			data[dataIndex + 6] = mNorm.z;
		//			data[dataIndex + 7] = 1.0f;

		//			aiVector3D& mTangent = fbxMesh->mTangents[i];
		//			data[dataIndex + 8] = mTangent.x;
		//			data[dataIndex + 9] = mTangent.y;
		//			data[dataIndex + 10] = mTangent.z;
		//			data[dataIndex + 11] = 1.0f;

		//			aiVector3D& biTangent = fbxMesh->mBitangents[i];
		//			data[dataIndex + 12] = biTangent.x;
		//			data[dataIndex + 13] = biTangent.y;
		//			data[dataIndex + 14] = biTangent.z;
		//			data[dataIndex + 15] = 1.0f;

		//			data[dataIndex + 16] = fbxMesh->mTextureCoords[TEXTURE_SET_0][i].x;
		//			data[dataIndex + 17] = fbxMesh->mTextureCoords[TEXTURE_SET_0][i].y;

		//			SVertexBoneData& boneData = collectedBoneData[i];
		//			data[dataIndex + 18] = (F32)boneData.IDs[0];
		//			data[dataIndex + 19] = (F32)boneData.IDs[1];
		//			data[dataIndex + 20] = (F32)boneData.IDs[2];
		//			data[dataIndex + 21] = (F32)boneData.IDs[3];
		//			//CONFJURMED by Haqvin

		//			data[dataIndex + 22] = boneData.Weights[0];
		//			data[dataIndex + 23] = boneData.Weights[1];
		//			data[dataIndex + 24] = boneData.Weights[2];
		//			data[dataIndex + 25] = boneData.Weights[3];
		//		}
		//	}
		//	else if (hasPositions && hasNormals && hasTangents && hasTextures) 
		//	{
		//		for (unsigned int i = 0, dataIndex = 0; i < fbxMesh->mNumVertices; i++, dataIndex += (vertexBufferSize / 4)) 
		//		{
		//			aiVector3D& mVertice = fbxMesh->mVertices[i];
		//			data[dataIndex] = mVertice.x;
		//			data[dataIndex + 1] = mVertice.y;
		//			data[dataIndex + 2] = mVertice.z;
		//			data[dataIndex + 3] = 1.0f;

		//			aiVector3D& mNorm = fbxMesh->mNormals[i];
		//			data[dataIndex + 4] = mNorm.x;
		//			data[dataIndex + 5] = mNorm.y;
		//			data[dataIndex + 6] = mNorm.z;
		//			data[dataIndex + 7] = 1.0f;

		//			aiVector3D& mTangent = fbxMesh->mTangents[i];
		//			data[dataIndex + 8] = mTangent.x;
		//			data[dataIndex + 9] = mTangent.y;
		//			data[dataIndex + 10] = mTangent.z;
		//			data[dataIndex + 11] = 1.0f;

		//			aiVector3D& biTangent = fbxMesh->mBitangents[i];
		//			data[dataIndex + 12] = biTangent.x;
		//			data[dataIndex + 13] = biTangent.y;
		//			data[dataIndex + 14] = biTangent.z;
		//			data[dataIndex + 15] = 1.0f;

		//			data[dataIndex + 16] = fbxMesh->mTextureCoords[TEXTURE_SET_0][i].x;
		//			data[dataIndex + 17] = fbxMesh->mTextureCoords[TEXTURE_SET_0][i].y;
		//		}
		//	}
		//	else if (hasPositions && hasNormals && hasTangents) 
		//	{
		//		for (unsigned int i = 0, dataIndex = 0; i < fbxMesh->mNumVertices; i++, dataIndex += (vertexBufferSize / 4)) 
		//		{
		//			aiVector3D& mVertice = fbxMesh->mVertices[i];
		//			data[dataIndex] = mVertice.x;
		//			data[dataIndex + 1] = mVertice.y;
		//			data[dataIndex + 2] = mVertice.z;
		//			data[dataIndex + 3] = 1.0f;

		//			aiVector3D& mNorm = fbxMesh->mNormals[i];
		//			data[dataIndex + 4] = mNorm.x;
		//			data[dataIndex + 5] = mNorm.y;
		//			data[dataIndex + 6] = mNorm.z;
		//			data[dataIndex + 7] = 1.0f;

		//			aiVector3D& mTangent = fbxMesh->mTangents[i];
		//			data[dataIndex + 8] = mTangent.x;
		//			data[dataIndex + 9] = mTangent.y;
		//			data[dataIndex + 10] = mTangent.z;
		//			data[dataIndex + 11] = 1.0f;

		//			aiVector3D& biTangent = fbxMesh->mBitangents[i];
		//			data[dataIndex + 12] = biTangent.x;
		//			data[dataIndex + 13] = biTangent.y;
		//			data[dataIndex + 14] = biTangent.z;
		//			data[dataIndex + 15] = 1.0f;
		//		}
		//	}
		//	else if (hasPositions && hasNormals) 
		//	{
		//		for (unsigned int i = 0, dataIndex = 0; i < fbxMesh->mNumVertices; i++, dataIndex += (vertexBufferSize / 4)) 
		//		{

		//			aiVector3D& mVertice = fbxMesh->mVertices[i];
		//			data[dataIndex] = mVertice.x;
		//			data[dataIndex + 1] = mVertice.y;
		//			data[dataIndex + 2] = mVertice.z;
		//			data[dataIndex + 3] = 1.0f;

		//			aiVector3D& mNorm = fbxMesh->mNormals[i];
		//			data[dataIndex + 4] = mNorm.x;
		//			data[dataIndex + 5] = mNorm.y;
		//			data[dataIndex + 6] = mNorm.z;
		//			data[dataIndex + 7] = 1.0f;
		//		}
		//	}
		//	else if (hasPositions) 
		//	{
		//		for (unsigned int i = 0, dataIndex = 0; i < fbxMesh->mNumVertices; i++, dataIndex += (vertexBufferSize / 4)) 
		//		{
		//			aiVector3D& mVertice = fbxMesh->mVertices[i];
		//			data[dataIndex] = mVertice.x;
		//			data[dataIndex + 1] = mVertice.y;
		//			data[dataIndex + 2] = mVertice.z;
		//			data[dataIndex + 3] = 1.0f;
		//		}
		//	}

		//	memmove(aLoaderMesh->myVerticies, data, vertexBufferSize * fbxMesh->mNumVertices);
		//	delete data;
		//	//return vertexBufferSize;


		//	for (unsigned int i = 0; i < fbxMesh->mNumFaces; i++)
		//	{
		//		mesh->myIndexes.insert(mesh->myIndexes.end(), std::make_move_iterator(&fbxMesh->mFaces[i].mIndices[0]), std::make_move_iterator(&fbxMesh->mFaces[i].mIndices[fbxMesh->mFaces[i].mNumIndices]));
		//	}
		//}
		//// Change to support multiple animations
		//if (scene->mNumAnimations > 0)
		//{
		//	model->myAnimationDuration = (F32)scene->mAnimations[0]->mDuration;
		//}

		//LoadMaterials(scene, model);


		//model->myGlobalInverseTransform = ConvertToEngineMatrix44(scene->mRootNode->mTransformation);
