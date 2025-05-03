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
	void InitM4FromM3(aiMatrix4x4& out, const aiMatrix3x3& in)
	{
		out.a1 = in.a1; out.a2 = in.a2; out.a3 = in.a3; out.a4 = 0.f;
		out.b1 = in.b1; out.b2 = in.b2; out.b3 = in.b3; out.b4 = 0.f;
		out.c1 = in.c1; out.c2 = in.c2; out.c3 = in.c3; out.c4 = 0.f;
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
		mat(0, 0) = assimpMatrix.a1; mat(0, 1) = assimpMatrix.a2; mat(0, 2) = assimpMatrix.a3; mat(0, 3) = assimpMatrix.a4;
		mat(1, 0) = assimpMatrix.b1; mat(1, 1) = assimpMatrix.b2; mat(1, 2) = assimpMatrix.b3; mat(1, 3) = assimpMatrix.b4;
		mat(2, 0) = assimpMatrix.c1; mat(2, 1) = assimpMatrix.c2; mat(2, 2) = assimpMatrix.c3; mat(2, 3) = assimpMatrix.c4;
		mat(3, 0) = assimpMatrix.d1; mat(3, 1) = assimpMatrix.d2; mat(3, 2) = assimpMatrix.d3; mat(3, 3) = assimpMatrix.d4;
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
			for (U32 i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++)
			{
				if (IDs[i] == boneID)
					return;
			}

			if (weight == 0)
				return;

			for (U32 i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++) 
			{
				if (Weights[i] == 0.0) 
				{
					IDs[i] = boneID;
					Weights[i] = weight;
					return;
				}
			}
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

		// NW: To make use of destructible meshes, add DontJoinIdentical (vertices)
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

	void ExtractNodes(const aiScene* scene, const SAssetImportOptions& importOptions, aiNode* node, const std::vector<SSkeletalMeshBone>& bindPose, std::vector<SSkeletalMeshNode>& nodesToPopulate)
	{
		if (nodesToPopulate.size() > 0)
		{
			CHavtornStaticString<255> parentName = node->mParent ? std::string(node->mParent->mName.data) : "";
			if (auto it = std::ranges::find(nodesToPopulate, parentName, &SSkeletalMeshNode::Name); it != nodesToPopulate.end())
			{
				U32 childIndex = STATIC_U32(std::distance(std::begin(nodesToPopulate), it));
				nodesToPopulate[childIndex].ChildIndices.push_back(STATIC_U32(nodesToPopulate.size()));
			}
		}

		CHavtornStaticString<255> nodeName = CHavtornStaticString<255>(node->mName.C_Str());
		SMatrix transform = SMatrix::Transpose(ToHavtornMatrix(node->mTransformation));
		transform.SetTranslation(transform.GetTranslation() * importOptions.Scale);
		nodesToPopulate.emplace_back(SSkeletalMeshNode(nodeName, transform, {}));

		for (U32 i = 0; i < node->mNumChildren; i++)
			ExtractNodes(scene, importOptions, node->mChildren[i], bindPose, nodesToPopulate);
	}

	std::string UModelImporter::ImportSkeletalMesh(const std::string& filePath, const std::string& destinationPath, const SAssetImportOptions& importOptions, const aiScene* assimpScene)
	{
		SSkeletalModelFileHeader fileHeader;
		fileHeader.AssetType = EAssetType::SkeletalMesh;
		fileHeader.Name = assimpScene->mName.C_Str();
		fileHeader.NumberOfMeshes = assimpScene->mNumMeshes;
		fileHeader.Meshes.reserve(fileHeader.NumberOfMeshes);

		const aiMesh* fbxMesh = nullptr;

		// TODO.NW: Figure out how to scale the armature so that we can reliably scale skeletal meshes

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
					SMatrix inverseBindPose = SMatrix::Transpose(ToHavtornMatrix(fbxMesh->mBones[i]->mOffsetMatrix));
					inverseBindPose.SetTranslation(inverseBindPose.GetTranslation4() * importOptions.Scale);
					fileHeader.BindPoseBones.push_back(SSkeletalMeshBone(boneName, inverseBindPose, parentIndex));
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

			ExtractNodes(assimpScene, importOptions, assimpScene->mRootNode, fileHeader.BindPoseBones, fileHeader.Nodes);

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

		std::string newFileName = destinationPath + UGeneralUtils::ExtractFileBaseNameFromPath(filePath) + ".hva";
		const auto fileData = new char[fileHeader.GetSize()];
		fileHeader.Serialize(fileData);
		GEngine::GetFileSystem()->Serialize(newFileName, &fileData[0], fileHeader.GetSize());
		return newFileName;
	}

	std::string UModelImporter::ImportAnimation(const std::string& filePath, const std::string& destinationPath, const SAssetImportOptions& importOptions, const aiScene* assimpScene)
	{
		const aiAnimation* animation = assimpScene->mAnimations[0];

		// TODO.NW: Figure out how to scale the armature so that we can reliably scale skeletal meshes

		// TODO.NW: Support multiple animations per file? Support montages somehow. Could be separate file using these headers (SSkeletalAnimationMontageFileHeader)
		SSkeletalAnimationFileHeader fileHeader;
		fileHeader.AssetType = EAssetType::Animation;
		fileHeader.Name = UGeneralUtils::ExtractFileNameFromPath(filePath);
		fileHeader.DurationInTicks = STATIC_U32(animation->mDuration);
		fileHeader.TickRate = STATIC_U32(animation->mTicksPerSecond);
		fileHeader.ImportScale = importOptions.Scale;
			
		std::vector<SSkeletalMeshBone> bones;
		{
			// Need full path here, not just filename
			std::string rigFilePath = importOptions.AssetRep->DirectoryEntry.path().string();
			fileHeader.SkeletonName = rigFilePath;
			const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(rigFilePath);
			char* data = new char[fileSize];

			GEngine::GetFileSystem()->Deserialize(rigFilePath, data, STATIC_U32(fileSize));

			SSkeletalModelFileHeader rigHeader;
			rigHeader.Deserialize(data);

			bones = rigHeader.BindPoseBones;

			delete[] data;
		}

		fileHeader.NumberOfBones = STATIC_U32(animation->mNumChannels);

		for (U32 i = 0; i < animation->mNumChannels; i++)
		{
			const aiNodeAnim* channel = animation->mChannels[i];

			fileHeader.BoneAnimationTracks.emplace_back();
			SBoneAnimationTrack& track = fileHeader.BoneAnimationTracks.back();
			track.TrackName = std::string(channel->mNodeName.C_Str());

			if (channel != nullptr)
			{
				for (U32 t = 0; t < channel->mNumPositionKeys; t++)
					track.TranslationKeys.emplace_back(ToHavtornVecAnimationKey(channel->mPositionKeys[t]));

				for (U32 q = 0; q < channel->mNumRotationKeys; q++)
					track.RotationKeys.emplace_back(ToHavtornQuatAnimationKey(channel->mRotationKeys[q]));

				for (U32 s = 0; s < channel->mNumScalingKeys; s++)
					track.ScaleKeys.emplace_back(ToHavtornVecAnimationKey(channel->mScalingKeys[s]));
			}
		}

		std::string newFileName = destinationPath + UGeneralUtils::ExtractFileBaseNameFromPath(filePath) + ".hva";
		const auto fileData = new char[fileHeader.GetSize()];
		fileHeader.Serialize(fileData);
		GEngine::GetFileSystem()->Serialize(newFileName, &fileData[0], fileHeader.GetSize());
		return newFileName;
	}
}
