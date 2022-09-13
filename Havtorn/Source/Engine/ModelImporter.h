// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	enum class EModelProperty
	{
		HasPositions				= BIT(0),
		HasNormals					= BIT(1),
		HasTangentsAndBitangents	= BIT(2),
		HasUVs						= BIT(3),
		HasBones					= BIT(4)
	};

#define NUM_BONES_PER_VERTEX 4
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

		void AddBoneData(const U32 BoneID, const F32 Weight)
		{
			for (U32 i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++)
			{
				if (Weights[i] == 0.0f)
				{
					IDs[i] = BoneID;
					Weights[i] = Weight;
					return;
				}
			}

			// Should never get here - more bones than we have space for.
			assert(0);
		}
	};

	class CModelImporter
	{
	public:
		static std::string ImportFBX(const std::string& filePath);

	private:
	};
}
