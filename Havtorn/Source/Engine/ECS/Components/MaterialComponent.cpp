// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "MaterialComponent.h"

namespace Havtorn
{
    void SMaterialComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(static_cast<U32>(Materials.size()), toData, pointerPosition);
        
        for (auto& material : Materials)
        {
            SerializeData(material.AlbedoR, toData, pointerPosition);
            SerializeData(material.AlbedoG, toData, pointerPosition);
            SerializeData(material.AlbedoB, toData, pointerPosition);
            SerializeData(material.AlbedoA, toData, pointerPosition);
            SerializeData(material.NormalX, toData, pointerPosition);
            SerializeData(material.NormalY, toData, pointerPosition);
            SerializeData(material.NormalZ, toData, pointerPosition);
            SerializeData(material.AmbientOcclusion, toData, pointerPosition);
            SerializeData(material.Metalness, toData, pointerPosition);
            SerializeData(material.Roughness, toData, pointerPosition);
            SerializeData(material.Emissive, toData, pointerPosition);
            
            SerializeData(static_cast<U32>(material.Name.length()), toData, pointerPosition);
            SerializeData(material.Name.c_str(), toData, pointerPosition);

            SerializeData(material.RecreateNormalZ, toData, pointerPosition);
        }
    }

    void SMaterialComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        U32 materialNumber = 0;
        DeserializeData(materialNumber, fromData, pointerPosition);

        for (U32 index = 0; index < materialNumber; index++)
        {
            auto& material = Materials.emplace_back();

            DeserializeData(material.AlbedoR, fromData, pointerPosition);
            DeserializeData(material.AlbedoG, fromData, pointerPosition);
            DeserializeData(material.AlbedoB, fromData, pointerPosition);
            DeserializeData(material.AlbedoA, fromData, pointerPosition);
            DeserializeData(material.NormalX, fromData, pointerPosition);
            DeserializeData(material.NormalY, fromData, pointerPosition);
            DeserializeData(material.NormalZ, fromData, pointerPosition);
            DeserializeData(material.AmbientOcclusion, fromData, pointerPosition);
            DeserializeData(material.Metalness, fromData, pointerPosition);
            DeserializeData(material.Roughness, fromData, pointerPosition);
            DeserializeData(material.Emissive, fromData, pointerPosition);

            U32 nameLength = 0;
            DeserializeData(nameLength, fromData, pointerPosition);
            DeserializeData(material.Name, fromData, nameLength, pointerPosition);

            DeserializeData(material.RecreateNormalZ, fromData, pointerPosition);
        }
    }

    U32 SMaterialComponent::GetSize() const
    {
        U32 size = 0;
        
        size += sizeof(U32);

        for (auto& material : Materials)
        {
            size += static_cast<U32>(EMaterialProperty::Count) * sizeof(SRuntimeGraphicsMaterialProperty);
            
            size += sizeof(U32);
            size += static_cast<U32>(material.Name.length()) * sizeof(char);
            
            size += sizeof(bool);
        }

        return size;
    }
}
