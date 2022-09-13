// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "ModelImporter.h"

// Assimp
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>

#include "Engine.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileHeaderDeclarations.h"

#ifdef _DEBUG
#pragma comment(lib, "assimp-vc142-mtd.lib")
#pragma comment(lib, "DirectXTex_Debug.lib")
#else
#pragma comment(lib, "assimp-vc142-mt.lib")
#pragma comment(lib, "DirectXTex_Release.lib")
#endif

namespace Havtorn
{
	SMatrix ConvertToSMatrix(const aiMatrix4x4& AssimpMatrix)
	{
		SMatrix mat;
		mat(0, 0) = AssimpMatrix.a1; mat(0, 1) = AssimpMatrix.a2; mat(0, 2) = AssimpMatrix.a3; mat(0, 3) = AssimpMatrix.a4;
		mat(1, 0) = AssimpMatrix.b1; mat(1, 1) = AssimpMatrix.b2; mat(1, 2) = AssimpMatrix.b3; mat(1, 3) = AssimpMatrix.b4;
		mat(2, 0) = AssimpMatrix.c1; mat(2, 1) = AssimpMatrix.c2; mat(2, 2) = AssimpMatrix.c3; mat(2, 3) = AssimpMatrix.c4;
		mat(3, 0) = AssimpMatrix.d1; mat(3, 1) = AssimpMatrix.d2; mat(3, 2) = AssimpMatrix.d3; mat(3, 3) = AssimpMatrix.d4;
		return mat;
	}

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
		const bool hasNormals	= fbxMesh->HasNormals();
		const bool hasTangents	= fbxMesh->HasTangentsAndBitangents();
		const bool hasTextures	= fbxMesh->HasTextureCoords(0);
		const bool hasBones		= fbxMesh->HasBones();
		//const bool hasBones		= false;

		SStaticMeshFileHeader fileHeader;
		if (hasPositions && hasNormals && hasTangents && hasTextures && !hasBones)
		{
			fileHeader.AssetType = EAssetType::StaticMesh;
		}
		else if (hasPositions && hasNormals && hasTangents && hasTextures && hasBones)
		{
			fileHeader.AssetType = EAssetType::SkeletalMesh;
		}

		fileHeader.NameLength = assimpScene->mName.length;
		fileHeader.Name = assimpScene->mName.C_Str();
		fileHeader.NumberOfMeshes = assimpScene->mNumMeshes;
		fileHeader.Meshes.reserve(fileHeader.NumberOfMeshes);

		// Pre-loading Pass
		for (U8 n = 0; n < assimpScene->mNumMeshes; n++)
		{
			fileHeader.Meshes.emplace_back();
			fbxMesh = assimpScene->mMeshes[n];
			fileHeader.Meshes[n].NumberOfVertices = fbxMesh->mNumVertices;
			fileHeader.Meshes[n].Vertices.reserve(fbxMesh->mNumVertices);
		}

		for (U8 n = 0; n < assimpScene->mNumMeshes; n++)
		{
			auto& fileHeaderMesh = fileHeader.Meshes[n];
			fbxMesh = assimpScene->mMeshes[n];
			//model->myMaterialIndices.push_back(fbxMesh->mMaterialIndex);

			// Bones 
			std::vector<SVertexBoneData> boneData;
			if (hasBones)
			{
				boneData.resize(fbxMesh->mNumVertices);

				unsigned int BoneIndex = 0;
				for (unsigned int i = 0; i < fbxMesh->mNumBones; i++)
				{
					// TODO.AS/AG: Variable names, and more.
					SJointInfo jointInfo(fbxMesh->mBones[i]->mName.data);
					if (aLoaderMesh->myModel->myBoneNameToIndex.find(jointInfo.JointName.c_str()) == aLoaderMesh->myModel->myBoneNameToIndex.end())// If BoneName is at the end of list, I.E does not exist in list
					{
						BoneIndex = aLoaderMesh->myModel->myNumBones;
						aLoaderMesh->myModel->myNumBones++;
						BoneInfo bi;
						aLoaderMesh->myModel->myBoneInfo.emplace_back(bi);


						SMatrix NodeTransformation = ConvertToSMatrix(fbxMesh->mBones[i]->mOffsetMatrix);// Convert InverseBind pose to Engine Matrix

						aLoaderMesh->myModel->myBoneInfo[BoneIndex].BoneOffset = NodeTransformation;
						aLoaderMesh->myModel->myBoneNameToIndex[jointInfo.JointName.c_str()] = BoneIndex;
					}
					else
					{
						BoneIndex = aLoaderMesh->myModel->myBoneNameToIndex[jointInfo.JointName.c_str()];
					}

					for (unsigned int j = 0; j < fbxMesh->mBones[i]->mNumWeights; j++)
					{
						unsigned int VertexID = fbxMesh->mBones[i]->mWeights[j].mVertexId;
						float Weight = fbxMesh->mBones[i]->mWeights[j].mWeight;
						boneData[VertexID].AddBoneData(BoneIndex, Weight);
					}
				}
			}

			// Vertices
			constexpr F32 scaleModifier = 0.01f;
			for (U32 i = 0; i < fbxMesh->mNumVertices; i++)
			{
				switch (fileHeader.AssetType)
				{
				case EAssetType::StaticMesh:
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
					break;
				case EAssetType::SkeletalMesh:
					{
						SSkeletalMeshVertex newVertex;

						aiVector3D& pos = fbxMesh->mVertices[i];
						pos *= scaleModifier;
						newVertex.StaticMeshVertex.x = pos.x;
						newVertex.StaticMeshVertex.y = pos.y;
						newVertex.StaticMeshVertex.z = pos.z;

						const aiVector3D& norm = fbxMesh->mNormals[i];
						newVertex.StaticMeshVertex.nx = norm.x;
						newVertex.StaticMeshVertex.ny = norm.y;
						newVertex.StaticMeshVertex.nz = norm.z;

						const aiVector3D& tangent = fbxMesh->mTangents[i];
						newVertex.StaticMeshVertex.tx = tangent.x;
						newVertex.StaticMeshVertex.ty = tangent.y;
						newVertex.StaticMeshVertex.tz = tangent.z;

						const aiVector3D& biTangent = fbxMesh->mBitangents[i];
						newVertex.StaticMeshVertex.bx = biTangent.x;
						newVertex.StaticMeshVertex.by = biTangent.y;
						newVertex.StaticMeshVertex.bz = biTangent.z;

						newVertex.StaticMeshVertex.u = fbxMesh->mTextureCoords[0][i].x;
						newVertex.StaticMeshVertex.v = fbxMesh->mTextureCoords[0][i].y;

						auto bones = fbxMesh->mBones;
						bones[0]->
						//VertexBoneData& boneData = collectedBoneData[i];
						newVertex.ID0 = boneData.IDs[0];
						newVertex.ID1 = boneData.IDs[1];
						newVertex.ID2 = boneData.IDs[2];
						newVertex.ID3 = boneData.IDs[3];

						newVertex.Weight0 = boneData.Weights[0];
						newVertex.Weight1 = boneData.Weights[1];
						newVertex.Weight2 = boneData.Weights[2];
						newVertex.Weight3 = boneData.Weights[3];

						fileHeaderMesh.Vertices.emplace_back(newVertex);
					}
					break;
				case EAssetType::Animation:
					{
						
					}
					break;
				default:
					return "ERROR: Can't find EAssetType.";
				}
			}

			// Indices
			for (U32 i = 0; i < fbxMesh->mNumFaces; i++)
			{
				fileHeaderMesh.NumberOfIndices += fbxMesh->mFaces[i].mNumIndices;
				fileHeaderMesh.Indices.insert(fileHeaderMesh.Indices.end(), std::make_move_iterator(&fbxMesh->mFaces[i].mIndices[0]), std::make_move_iterator(&fbxMesh->mFaces[i].mIndices[fbxMesh->mFaces[i].mNumIndices]));
			}

			// Material Count
			fileHeader.NumberOfMaterials = static_cast<U8>(assimpScene->mNumMaterials);

			// Bone stuff
			//std::vector<VertexBoneData> collectedBoneData;
			//if (fbxMesh->HasBones())
			//{
			//	collectedBoneData.resize(fbxMesh->mNumVertices);

			//	unsigned int BoneIndex = 0;
			//	for (unsigned int i = 0; i < fbxMesh->mNumBones; i++)
			//	{
			//		std::string BoneName(fbxMesh->mBones[i]->mName.data);
			//		if (aLoaderMesh->myModel->myBoneNameToIndex.find(BoneName) == aLoaderMesh->myModel->myBoneNameToIndex.end())
			//		{
			//			BoneIndex = aLoaderMesh->myModel->myNumBones;
			//			aLoaderMesh->myModel->myNumBones++;
			//			BoneInfo bi;
			//			aLoaderMesh->myModel->myBoneInfo.emplace_back(bi);


			//			Matrix44f NodeTransformation = ConvertToEngineMatrix44(fbxMesh->mBones[i]->mOffsetMatrix);

			//			//Vector3 scale;
			//			//Vector3 translation;
			//			//Quaternion rotation;
			//			//myTransform.Decompose(scale, rotation, translation);
			//			//myTransform = Matrix44f::CreateFromQuaternion(rotation);
			//			//myTransform *= Matrix44f::CreateScale(myScale);
			//			//myTransform.Translation(translation);

			//			aLoaderMesh->myModel->myBoneInfo[BoneIndex].BoneOffset = NodeTransformation;
			//			aLoaderMesh->myModel->myBoneNameToIndex[BoneName] = BoneIndex;
			//		}
			//		else
			//		{
			//			BoneIndex = aLoaderMesh->myModel->myBoneNameToIndex[BoneName];
			//		}

			//		for (unsigned int j = 0; j < fbxMesh->mBones[i]->mNumWeights; j++)
			//		{
			//			unsigned int VertexID = fbxMesh->mBones[i]->mWeights[j].mVertexId;
			//			float Weight = fbxMesh->mBones[i]->mWeights[j].mWeight;
			//			collectedBoneData[VertexID].AddBoneData(BoneIndex, Weight);
			//		}
			//	}
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

		//			VertexBoneData& boneData = collectedBoneData[i];
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
