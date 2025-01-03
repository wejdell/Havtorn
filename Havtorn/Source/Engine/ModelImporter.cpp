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
			assert(0);
		}
	};

	std::string CModelImporter::ImportFBX(const std::string& filePath)
	{
		if (!CFileSystem::DoesFileExist(filePath))
			return "ERROR: File Does Not Exist";

		const aiScene* assimpScene = aiImportFile(filePath.c_str(), aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded);

		if (!assimpScene)
		{
			HV_LOG_ERROR("ModelImporter failed to import %s!", filePath.c_str());
			return "ERROR: Failed to import.";
		}

		const aiMesh* fbxMesh = assimpScene->mMeshes[0];

		const bool hasPositions = fbxMesh->HasPositions();
		const bool hasNormals = fbxMesh->HasNormals();
		const bool hasTangents = fbxMesh->HasTangentsAndBitangents();
		const bool hasTextures = fbxMesh->HasTextureCoords(0);
		const bool hasBones = fbxMesh->HasBones();

		EAssetType assetType = EAssetType::None;
		if (hasPositions && hasNormals && hasTangents && hasTextures && !hasBones)
			assetType = EAssetType::StaticMesh;
		else if (hasPositions && hasNormals && hasTangents && hasTextures && hasBones)
			assetType = EAssetType::SkeletalMesh;

		std::string newFileName = filePath.substr(0, filePath.length() - 4);
		newFileName.append(".hva");

		if (assetType == EAssetType::StaticMesh)
		{
			SStaticModelFileHeader fileHeader;
			fileHeader.AssetType = assetType;
			fileHeader.Name = assimpScene->mName.C_Str();
			fileHeader.NumberOfMeshes = assimpScene->mNumMeshes;
			fileHeader.Meshes.reserve(fileHeader.NumberOfMeshes);

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

			const auto fileData = new char[fileHeader.GetSize()];
			fileHeader.Serialize(fileData);
			GEngine::GetFileSystem()->Serialize(newFileName, &fileData[0], fileHeader.GetSize());
		}
		//else if (assetType == EAssetType::SSkeletalMesh)
		//{
		//	SSkeletalMeshFileHeader fileHeader;
		//	fileHeader.AssetType = assetType;
		//	fileHeader.Name = assimpScene->mName.C_Str();
		//	fileHeader.NumberOfMeshes = assimpScene->mNumMeshes;
		//	fileHeader.Meshes.reserve(fileHeader.NumberOfMeshes);

		//	// Pre-loading Pass
		//	for (U8 n = 0; n < assimpScene->mNumMeshes; n++)
		//	{
		//		fileHeader.Meshes.emplace_back();
		//		fbxMesh = assimpScene->mMeshes[n];
		//		fileHeader.Meshes[n].Vertices.reserve(fbxMesh->mNumVertices);
		//	}

		//	// Bone pre-loading
		//	std::vector<SVertexBoneData> collectedBoneData;
		//	collectedBoneData.resize(fbxMesh->mNumVertices);

		//	U32 boneIndex = 0;
		//	for (U32 i = 0; i < fbxMesh->mNumBones; i++)
		//	{
		//		std::string boneName(fbxMesh->mBones[i]->mName.data);
		//		if (aLoaderMesh->myModel->myBoneNameToIndex.find(boneName) == aLoaderMesh->myModel->myBoneNameToIndex.end())
		//		{
		//			boneIndex = aLoaderMesh->myModel->myNumBones;
		//			aLoaderMesh->myModel->myNumBones++;
		//			BoneInfo bi;
		//			aLoaderMesh->myModel->myBoneInfo.emplace_back(bi);

		//			Matrix44f NodeTransformation = ConvertToEngineMatrix44(fbxMesh->mBones[i]->mOffsetMatrix);

		//			aLoaderMesh->myModel->myBoneInfo[boneIndex].BoneOffset = NodeTransformation;
		//			aLoaderMesh->myModel->myBoneNameToIndex[boneName] = boneIndex;
		//		}
		//		else
		//		{
		//			boneIndex = aLoaderMesh->myModel->myBoneNameToIndex[boneName];
		//		}

		//		for (unsigned int j = 0; j < fbxMesh->mBones[i]->mNumWeights; j++)
		//		{
		//			unsigned int VertexID = fbxMesh->mBones[i]->mWeights[j].mVertexId;
		//			float Weight = fbxMesh->mBones[i]->mWeights[j].mWeight;
		//			collectedBoneData[VertexID].AddBoneData(boneIndex, Weight);
		//		}
		//	}

		//	for (U8 n = 0; n < assimpScene->mNumMeshes; n++)
		//	{
		//		auto& fileHeaderMesh = fileHeader.Meshes[n];
		//		fbxMesh = assimpScene->mMeshes[n];

		//		// Vertices
		//		// TODO.NR: Make import options rather soon
		//		constexpr F32 scaleModifier = 0.01f;
		//		for (U32 i = 0; i < fbxMesh->mNumVertices; i++)
		//		{
		//			SSkeletalMeshVertex newVertex;

		//			aiVector3D& pos = fbxMesh->mVertices[i];
		//			pos *= scaleModifier;
		//			newVertex.x = pos.x;
		//			newVertex.y = pos.y;
		//			newVertex.z = pos.z;

		//			const aiVector3D& norm = fbxMesh->mNormals[i];
		//			newVertex.nx = norm.x;
		//			newVertex.ny = norm.y;
		//			newVertex.nz = norm.z;

		//			const aiVector3D& tangent = fbxMesh->mTangents[i];
		//			newVertex.tx = tangent.x;
		//			newVertex.ty = tangent.y;
		//			newVertex.tz = tangent.z;

		//			const aiVector3D& biTangent = fbxMesh->mBitangents[i];
		//			newVertex.bx = biTangent.x;
		//			newVertex.by = biTangent.y;
		//			newVertex.bz = biTangent.z;

		//			newVertex.u = fbxMesh->mTextureCoords[0][i].x;
		//			newVertex.v = fbxMesh->mTextureCoords[0][i].y;

		//			//fbxMesh->m
		//			fileHeaderMesh.Vertices.emplace_back(newVertex);
		//		}

		//		// Indices
		//		for (U32 i = 0; i < fbxMesh->mNumFaces; i++)
		//		{
		//			fileHeaderMesh.Indices.insert(fileHeaderMesh.Indices.end(), std::make_move_iterator(&fbxMesh->mFaces[i].mIndices[0]), std::make_move_iterator(&fbxMesh->mFaces[i].mIndices[fbxMesh->mFaces[i].mNumIndices]));
		//		}
		//	}

		//	// Material Count
		//	fileHeader.NumberOfMaterials = STATIC_U8(assimpScene->mNumMaterials);

		//	const auto fileData = new char[fileHeader.GetSize()];
		//	fileHeader.Serialize(fileData);
		//	GEngine::GetFileSystem()->Serialize(newFileName, &fileData[0], fileHeader.GetSize());
		//}
		
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
