// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "ModelImporter.h"

// Assimp
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Engine.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileHeaderDeclarations.h"

#define NUM_BONES_PER_VERTEX 4
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

namespace Havtorn
{
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
			for (U32 i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++) 
			{
				if (Weights[i] == 0.0) 
				{
					IDs[i] = boneID;
					Weights[i] = weight;
					return;
				}
			}

			// should never get here - more bones than we have space for
			//assert(0);
		}
	};

	SMatrix ToHavtornMatrix(const aiMatrix4x4& assimpMatrix)
	{
		SMatrix mat;
		mat(0, 0) = assimpMatrix.a1; mat(0, 1) = assimpMatrix.a2; mat(0, 2) = assimpMatrix.a3; mat(0, 3) = assimpMatrix.a4;
		mat(1, 0) = assimpMatrix.b1; mat(1, 1) = assimpMatrix.b2; mat(1, 2) = assimpMatrix.b3; mat(1, 3) = assimpMatrix.b4;
		mat(2, 0) = assimpMatrix.c1; mat(2, 1) = assimpMatrix.c2; mat(2, 2) = assimpMatrix.c3; mat(2, 3) = assimpMatrix.c4;
		mat(3, 0) = assimpMatrix.d1; mat(3, 1) = assimpMatrix.d2; mat(3, 2) = assimpMatrix.d3; mat(3, 3) = assimpMatrix.d4;
		return mat;
	}

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

	std::string UModelImporter::ImportFBX(const std::string& filePath, const EAssetType assetType)
	{
		if (!CFileSystem::DoesFileExist(filePath))
		{
			HV_LOG_ERROR("ModelImporter could not import %s. File does not exist!", filePath.c_str());
			return "ERROR: File does not exist.";
		}

		const aiScene* assimpScene = aiImportFile(filePath.c_str(), aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded);
		
		if (!assimpScene)
		{
			HV_LOG_ERROR("ModelImporter failed to import %s! Assimp Error: %s", filePath.c_str(), aiGetErrorString());
			return "ERROR: Failed to import.";
		}

		if (assetType == EAssetType::Animation)
		{
			if (!assimpScene->HasAnimations() /*|| assimpScene->HasMeshes()*/)
			{
				HV_LOG_ERROR("ModelImporter expected %s to be an animation file, but it either has no animations or contains meshes!", filePath.c_str());
				return "ERROR: Failed to import.";
			}

			return ImportAnimation(filePath, assimpScene);
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

		if (assetType == EAssetType::SkeletalMesh)
		{
			if (!hasBones)
			{
				HV_LOG_ERROR("ModelImporter expected %s to be a skeletal mesh file, but it lacks bone information!", filePath.c_str());
				return "ERROR: Failed to import";
			}

			return ImportSkeletalMesh(filePath, assimpScene);
		}

		// Static Mesh
		if (hasBones)
			HV_LOG_WARN("ModelImporter expected %s to be a static mesh file, but it contains bone information!", filePath.c_str());

		return ImportStaticMesh(filePath, assimpScene);
	}

	std::string UModelImporter::ImportStaticMesh(const std::string& filePath, const aiScene* assimpScene)
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
			// TODO.NR: Make import options rather soon
			constexpr F32 scaleModifier = 0.01f;
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

		std::string newFileName = filePath.substr(0, filePath.length() - 4);
		newFileName.append(".hva");
		const auto fileData = new char[fileHeader.GetSize()];
		fileHeader.Serialize(fileData);
		GEngine::GetFileSystem()->Serialize(newFileName, &fileData[0], fileHeader.GetSize());
		return newFileName;
	}

	std::string UModelImporter::ImportSkeletalMesh(const std::string& filePath, const aiScene* assimpScene)
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
					//fileHeaderMesh.BoneOffsetMatrices.push_back(ToHavtornMatrix(fbxMesh->mBones[i]->mOffsetMatrix));

					tempBoneNameToIndexMap[boneName] = boneIndex;
					fileHeader.BoneNames.push_back(boneName);
				}
				else
				{
					boneIndex = tempBoneNameToIndexMap[boneName];
				}

				for (U32 j = 0; j < UMath::Min(STATIC_U32(fbxMesh->mBones[i]->mNumWeights), STATIC_U32(NUM_BONES_PER_VERTEX)); j++)
				{
					U32 vertexID = fbxMesh->mBones[i]->mWeights[j].mVertexId;
					F32 weight = fbxMesh->mBones[i]->mWeights[j].mWeight;
					collectedBoneData[vertexID].AddBoneData(boneIndex, weight);
				}
			}

			// Vertices
			// TODO.NR: Make import options rather soon
			//constexpr F32 scaleModifier = 0.01f;
			constexpr F32 scaleModifier = 1.0f;
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

		std::string newFileName = filePath.substr(0, filePath.length() - 4);
		newFileName.append(".hva");
		const auto fileData = new char[fileHeader.GetSize()];
		fileHeader.Serialize(fileData);
		GEngine::GetFileSystem()->Serialize(newFileName, &fileData[0], fileHeader.GetSize());
		return newFileName;
	}

	std::string UModelImporter::ImportAnimation(const std::string& filePath, const aiScene* assimpScene)
	{
		const aiAnimation* animation = assimpScene->mAnimations[0];

		// TODO.NR: Support multiple animations per file? Support montages somehow. Could be separate file using these headers (SSkeletalAnimationMontageFileHeader)
		SSkeletalAnimationFileHeader fileHeader;
		fileHeader.AssetType = EAssetType::Animation;
		fileHeader.Name = UGeneralUtils::ExtractFileNameFromPath(filePath);
		fileHeader.DurationInTicks = STATIC_U32(animation->mDuration);
		fileHeader.TickRate = STATIC_U32(animation->mTicksPerSecond);
			
		std::vector<CHavtornStaticString<32>> boneNames;
		{
			// TODO.NW: Make the rig/skeleton part of import settings
			std::string rigFilePath = "ArtSource/Tests/TestMesh.hva";
			const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(rigFilePath);
			char* data = new char[fileSize];

			GEngine::GetFileSystem()->Deserialize(rigFilePath, data, STATIC_U32(fileSize));

			SSkeletalModelFileHeader rigHeader;
			rigHeader.Deserialize(data);

			boneNames = rigHeader.BoneNames;

			delete[] data;
		}

		fileHeader.NumberOfTracks = STATIC_U32(boneNames.size());
		fileHeader.BoneAnimationTracks.reserve(fileHeader.NumberOfTracks);

		//constexpr F32 scaleModifier = 0.01f;
		constexpr F32 scaleModifier = 1.0f;
		for (const CHavtornStaticString<32>& boneName : boneNames)
		{
			std::string channelName = boneName.AsString();
			const aiNodeAnim* channel = nullptr;

			for (U32 i = 0; i < animation->mNumChannels; i++)
			{
				if (strcmp(animation->mChannels[i]->mNodeName.C_Str(), channelName.c_str()) != 0)
					continue;

				channel = animation->mChannels[i];
			}
			
			fileHeader.BoneAnimationTracks.emplace_back();

			if (channel != nullptr)
			{
				SBoneAnimationTrack& track = fileHeader.BoneAnimationTracks.back();
				track.BoneName = boneName.AsString();

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
			else
			{
				// NR: Should this be <= durationInTicks?
				for (U32 k = 0; k <= fileHeader.DurationInTicks; k++)
				{
					SBoneAnimationTrack& track = fileHeader.BoneAnimationTracks.back();
					track.BoneName = boneName.AsString();

					if (track.TranslationKeys.size() > 0)
						track.TranslationKeys.emplace_back(track.TranslationKeys.back());
					else
						track.TranslationKeys.emplace_back(SVecBoneAnimationKey{ SVector::Zero, 0.0f });
					
					if (track.RotationKeys.size() > 0)
						track.RotationKeys.emplace_back(track.RotationKeys.back());
					else
						track.RotationKeys.emplace_back(SQuatBoneAnimationKey{ SQuaternion::Identity, 0.0f });

					if (track.ScaleKeys.size() > 0)
						track.ScaleKeys.emplace_back(track.ScaleKeys.back());
					else
						track.ScaleKeys.emplace_back(SVecBoneAnimationKey{ SVector(1.0f), 0.0f});
				}
			}
		}

		std::string newFileName = filePath.substr(0, filePath.length() - 4);
		newFileName.append(".hva");
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

		//	float* data = new float[(vertexBufferSize / 4) * fbxMesh->mNumVertices];
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
		//			data[dataIndex + 18] = (float)boneData.IDs[0];
		//			data[dataIndex + 19] = (float)boneData.IDs[1];
		//			data[dataIndex + 20] = (float)boneData.IDs[2];
		//			data[dataIndex + 21] = (float)boneData.IDs[3];
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
		//	model->myAnimationDuration = (float)scene->mAnimations[0]->mDuration;
		//}

		//LoadMaterials(scene, model);


		//model->myGlobalInverseTransform = ConvertToEngineMatrix44(scene->mRootNode->mTransformation);
