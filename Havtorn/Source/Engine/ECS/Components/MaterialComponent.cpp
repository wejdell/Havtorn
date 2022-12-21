#include "hvpch.h"
#include "MaterialComponent.h"

namespace Havtorn
{
    U32 SMaterialComponent::Serialize(char* toData, U32& bufferPosition)
    {
        bufferPosition += SerializeSimple(static_cast<U32>(Materials.size()), toData, bufferPosition);
        
        for (auto& material : Materials)
        {
            bufferPosition += SerializeSimple(material.AlbedoR, toData, bufferPosition);
            bufferPosition += SerializeSimple(material.AlbedoG, toData, bufferPosition);
            bufferPosition += SerializeSimple(material.AlbedoB, toData, bufferPosition);
            bufferPosition += SerializeSimple(material.AlbedoA, toData, bufferPosition);
            bufferPosition += SerializeSimple(material.NormalX, toData, bufferPosition);
            bufferPosition += SerializeSimple(material.NormalY, toData, bufferPosition);
            bufferPosition += SerializeSimple(material.NormalZ, toData, bufferPosition);
            bufferPosition += SerializeSimple(material.AmbientOcclusion, toData, bufferPosition);
            bufferPosition += SerializeSimple(material.Metalness, toData, bufferPosition);
            bufferPosition += SerializeSimple(material.Roughness, toData, bufferPosition);
            bufferPosition += SerializeSimple(material.Emissive, toData, bufferPosition);
            
            bufferPosition += SerializeSimple(static_cast<U32>(material.Name.length()), toData, bufferPosition);
            bufferPosition += SerializeString(material.Name.c_str(), toData, bufferPosition);

            bufferPosition += SerializeSimple(material.RecreateNormalZ, toData, bufferPosition);
        }

        return bufferPosition;
    }

    U32 SMaterialComponent::Deserialize(const char* fromData, U32& bufferPosition)
    {
        U32 materialNumber = 0;
        bufferPosition += DeserializeSimple(materialNumber, fromData, bufferPosition);

        for (U32 index = 0; index < materialNumber; index++)
        {
            auto& material = Materials.emplace_back();

            bufferPosition += DeserializeSimple(material.AlbedoR, fromData, bufferPosition);
            bufferPosition += DeserializeSimple(material.AlbedoG, fromData, bufferPosition);
            bufferPosition += DeserializeSimple(material.AlbedoB, fromData, bufferPosition);
            bufferPosition += DeserializeSimple(material.AlbedoA, fromData, bufferPosition);
            bufferPosition += DeserializeSimple(material.NormalX, fromData, bufferPosition);
            bufferPosition += DeserializeSimple(material.NormalY, fromData, bufferPosition);
            bufferPosition += DeserializeSimple(material.NormalZ, fromData, bufferPosition);
            bufferPosition += DeserializeSimple(material.AmbientOcclusion, fromData, bufferPosition);
            bufferPosition += DeserializeSimple(material.Metalness, fromData, bufferPosition);
            bufferPosition += DeserializeSimple(material.Roughness, fromData, bufferPosition);
            bufferPosition += DeserializeSimple(material.Emissive, fromData, bufferPosition);

            U32 nameLength = 0;
            bufferPosition += DeserializeSimple(nameLength, fromData, bufferPosition);
            bufferPosition += DeserializeString(material.Name, fromData, nameLength, bufferPosition);

            bufferPosition += DeserializeSimple(material.RecreateNormalZ, fromData, bufferPosition);
        }

        return bufferPosition;
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
