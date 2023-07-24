// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "MaterialComponent.h"

namespace Havtorn
{
    void SMaterialComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeSimple(static_cast<U32>(Materials.size()), toData, pointerPosition);
        
        for (auto& material : Materials)
        {
            SerializeSimple(material.AlbedoR, toData, pointerPosition);
            SerializeSimple(material.AlbedoG, toData, pointerPosition);
            SerializeSimple(material.AlbedoB, toData, pointerPosition);
            SerializeSimple(material.AlbedoA, toData, pointerPosition);
            SerializeSimple(material.NormalX, toData, pointerPosition);
            SerializeSimple(material.NormalY, toData, pointerPosition);
            SerializeSimple(material.NormalZ, toData, pointerPosition);
            SerializeSimple(material.AmbientOcclusion, toData, pointerPosition);
            SerializeSimple(material.Metalness, toData, pointerPosition);
            SerializeSimple(material.Roughness, toData, pointerPosition);
            SerializeSimple(material.Emissive, toData, pointerPosition);
            
            SerializeSimple(static_cast<U32>(material.Name.length()), toData, pointerPosition);
            SerializeString(material.Name.c_str(), toData, pointerPosition);

            SerializeSimple(material.RecreateNormalZ, toData, pointerPosition);
        }
    }

    void SMaterialComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        U32 materialNumber = 0;
        DeserializeSimple(materialNumber, fromData, pointerPosition);

        for (U32 index = 0; index < materialNumber; index++)
        {
            auto& material = Materials.emplace_back();

            DeserializeSimple(material.AlbedoR, fromData, pointerPosition);
            DeserializeSimple(material.AlbedoG, fromData, pointerPosition);
            DeserializeSimple(material.AlbedoB, fromData, pointerPosition);
            DeserializeSimple(material.AlbedoA, fromData, pointerPosition);
            DeserializeSimple(material.NormalX, fromData, pointerPosition);
            DeserializeSimple(material.NormalY, fromData, pointerPosition);
            DeserializeSimple(material.NormalZ, fromData, pointerPosition);
            DeserializeSimple(material.AmbientOcclusion, fromData, pointerPosition);
            DeserializeSimple(material.Metalness, fromData, pointerPosition);
            DeserializeSimple(material.Roughness, fromData, pointerPosition);
            DeserializeSimple(material.Emissive, fromData, pointerPosition);

            U32 nameLength = 0;
            DeserializeSimple(nameLength, fromData, pointerPosition);
            DeserializeString(material.Name, fromData, nameLength, pointerPosition);

            DeserializeSimple(material.RecreateNormalZ, fromData, pointerPosition);
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
