// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RenderStateManager.h"

#include "GeometryPrimitives.h"
#include "GraphicsUtilities.h"

#include "FileSystem/FileWatcher.h"

#include <RHI/RHI.h>
#include <GeneralUtilities.h>

#include <d3dcompiler.h>

namespace Havtorn
{
    CRenderStateManager::~CRenderStateManager()
    {
        Context = nullptr;
    }

    bool CRenderStateManager::Init(CRHI* rhi)
    {
        RHI = rhi;
        Context = rhi->GetContext();
        ID3D11Device* device = rhi->GetDevice();

        ENGINE_ERROR_BOOL_MESSAGE(Context, "Could not bind context.");
        ENGINE_ERROR_BOOL_MESSAGE(device, "Device is null.");

        ENGINE_ERROR_BOOL_MESSAGE(CreateBlendStates(RHI), "Could not create Blend States.");
        ENGINE_ERROR_BOOL_MESSAGE(CreateDepthStencilStates(RHI), "Could not create Depth Stencil States.");
        ENGINE_ERROR_BOOL_MESSAGE(CreateRasterizerStates(RHI), "Could not create Rasterizer States.");

        // Load default resources
        InitVertexShadersAndInputLayouts();
        InitPixelShaders();
        InitGeometryShaders();
        InitSamplers();
        InitVertexBuffers();
        InitIndexBuffers();
        InitMeshVertexStrides();
        InitMeshVertexOffset();

        return true;
    }

    void CRenderStateManager::InitVertexShadersAndInputLayouts()
    {
        struct SVertexShaderInitData
        {
            std::string FileName = "InitVertexShadersAndInputLayouts::UnmappedShader";
            bool ShouldAddLayout = false;
            EInputLayoutType InputLayout = EInputLayoutType::Null;
        };

        std::array<SVertexShaderInitData, STATIC_U64(EVertexShaders::Count)> initData;
        {
            initData[STATIC_U64(EVertexShaders::Fullscreen)]                    = { ShaderRoot + "FullscreenVertexShader_VS.cso", false };
            initData[STATIC_U64(EVertexShaders::StaticMesh)]                    = { ShaderRoot + "DeferredStaticMesh_VS.cso", true, EInputLayoutType::Pos3Nor3Tan3Bit3UV2 };
            initData[STATIC_U64(EVertexShaders::StaticMeshInstanced)]           = { ShaderRoot + "DeferredInstancedMesh_VS.cso", true, EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans };
            initData[STATIC_U64(EVertexShaders::Decal)]                         = { ShaderRoot + "Decal_VS.cso", false };
            initData[STATIC_U64(EVertexShaders::PointAndSpotLight)]             = { ShaderRoot + "PointLight_VS.cso", true, EInputLayoutType::Position4 };
            initData[STATIC_U64(EVertexShaders::EditorPreviewStaticMesh)]       = { ShaderRoot + "EditorPreview_VS.cso", false };
            initData[STATIC_U64(EVertexShaders::EditorPreviewSkeletalMesh)]     = { ShaderRoot + "EditorPreviewSkeletal_VS.cso", false };
            initData[STATIC_U64(EVertexShaders::Line)]                          = { ShaderRoot + "Line_VS.cso", false };
            initData[STATIC_U64(EVertexShaders::SpriteInstanced)]               = { ShaderRoot + "SpriteInstanced_VS.cso", true, EInputLayoutType::TransUVRectColor };
            initData[STATIC_U64(EVertexShaders::StaticMeshInstancedEditor)]     = { ShaderRoot + "DeferredInstancedMeshEditor_VS.cso", true, EInputLayoutType::Pos3Nor3Tan3Bit3UV2Entity2Trans };
            initData[STATIC_U64(EVertexShaders::SpriteInstancedEditor)]         = { ShaderRoot + "SpriteInstancedEditor_VS.cso", true, EInputLayoutType::TransUVRectColorEntity2 };
            initData[STATIC_U64(EVertexShaders::SkeletalMeshInstanced)]         = { ShaderRoot + "DeferredInstancedAnimation_VS.cso", true, EInputLayoutType::Pos3Nor3Tan3Bit3UV2BoneID4BoneWeight4AnimDataTrans };
            initData[STATIC_U64(EVertexShaders::SkeletalMeshInstancedEditor)]   = { ShaderRoot + "DeferredInstancedAnimationEditor_VS.cso", true, EInputLayoutType::Pos3Nor3Tan3Bit3UV2BoneID4BoneWeight4Entity2AnimDataTrans };
            initData[STATIC_U64(EVertexShaders::Skybox)]                        = { ShaderRoot + "Skybox_VS.cso", true, EInputLayoutType::Position4 };
            initData[STATIC_U64(EVertexShaders::StaticMeshVertexPaint)]         = { ShaderRoot + "DeferredVertexPaint_VS.cso", true, EInputLayoutType::Pos3Nor3Tan3Bit3UV2Color4 };
            initData[STATIC_U64(EVertexShaders::StaticMeshVertexPaintEditor)]   = { ShaderRoot + "DeferredVertexPaintEditor_VS.cso", true, EInputLayoutType::Pos3Nor3Tan3Bit3UV2Color4Entity2};
        }

        for (U64 i = 0; i < STATIC_U64(EVertexShaders::Count); i++)
        {
            std::string vsData = AddShader(initData[i].FileName, i, EShaderType::Vertex);
            if (initData[i].ShouldAddLayout)
                AddInputLayout(vsData, initData[i].InputLayout);
        }

        InputLayouts.emplace_back(nullptr);
    }

    void CRenderStateManager::InitPixelShaders()
    {
        std::array<std::string, STATIC_U64(EPixelShaders::Count)> filepaths;
        filepaths.fill("InitPixelShaders::UnmappedShader");
        {
            filepaths[STATIC_U64(EPixelShaders::GBuffer)]                           = ShaderRoot + "GBuffer_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::DecalAlbedo)]                       = ShaderRoot + "Decal_Albedo_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::DecalMaterial)]                     = ShaderRoot + "Decal_Material_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::DecalNormal)]                       = ShaderRoot + "Decal_Normal_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::DeferredDirectional)]               = ShaderRoot + "DeferredLightDirectionalAndEnvironment_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::DeferredPoint)]                     = ShaderRoot + "DeferredLightPoint_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::DeferredSpot)]                      = ShaderRoot + "DeferredLightSpot_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::VolumetricDirectional)]             = ShaderRoot + "DeferredLightDirectionalVolumetric_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::VolumetricPoint)]                   = ShaderRoot + "DeferredLightPointVolumetric_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::VolumetricSpot)]                    = ShaderRoot + "DeferredLightSpotVolumetric_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::EditorPreview)]                     = ShaderRoot + "EditorPreview_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::Line)]                              = ShaderRoot + "Line_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::SpriteScreenSpace)]                 = ShaderRoot + "SpriteScreenSpace_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::SpriteWorldSpace)]                  = ShaderRoot + "SpriteWorldSpace_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::GBufferInstanceEditor)]             = ShaderRoot + "GBufferEditor_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::SpriteWorldSpaceEditor)]            = ShaderRoot + "SpriteWorldSpaceEditor_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::SpriteWorldSpaceEditorWidget)]      = ShaderRoot + "SpriteWorldSpaceEditorWidget_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::Skybox)]                            = ShaderRoot + "Skybox_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenMultiply)]                = ShaderRoot + "FullscreenMultiply_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenCopy)]                    = ShaderRoot + "FullscreenCopy_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenCopyDepth)]               = ShaderRoot + "FullscreenCopyDepth_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenCopyGBuffer)]             = ShaderRoot + "FullscreenCopyGBuffer_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenDifference)]              = ShaderRoot + "FullscreenDifference_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenLuminance)]               = ShaderRoot + "FullscreenLuminance_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenGaussianHorizontal)]      = ShaderRoot + "FullscreenGaussianBlurHorizontal_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenGaussianVertical)]        = ShaderRoot + "FullscreenGaussianBlurVertical_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenBilateralHorizontal)]     = ShaderRoot + "FullscreenBilateralBlurHorizontal_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenBilateralVertical)]       = ShaderRoot + "FullscreenBilateralBlurVertical_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenBloom)]                   = ShaderRoot + "FullscreenBloom_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenVignette)]                = ShaderRoot + "FullscreenVignette_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenTonemap)]                 = ShaderRoot + "FullscreenTonemap_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenGammaCorrection)]         = ShaderRoot + "FullscreenGammaCorrection_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenFXAA)]                    = ShaderRoot + "FullscreenFXAA_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenSSAO)]                    = ShaderRoot + "FullscreenSSAO_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenSSAOBlur)]                = ShaderRoot + "FullscreenSSAOBlur_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenDownsampleDepth)]         = ShaderRoot + "FullscreenDepthDownSample_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenDepthAwareUpsampling)]    = ShaderRoot + "FullscreenDepthAwareUpsample_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenEditorData)]              = ShaderRoot + "FullscreenEditorData_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::FullscreenWorldPosition)]           = ShaderRoot + "FullscreenWorldPosition_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::GBufferVertexPaint)]                = ShaderRoot + "DeferredVertexPaint_PS.cso";
            filepaths[STATIC_U64(EPixelShaders::GBufferVertexPaintEditor)]          = ShaderRoot + "DeferredVertexPaintEditor_PS.cso";
        }

        for (U64 i = 0; i < STATIC_U64(EPixelShaders::Count); i++)
            AddShader(filepaths[i], i, EShaderType::Pixel);
    }

    void CRenderStateManager::InitGeometryShaders()
    {
        AddShader(ShaderRoot + "Line_GS.cso", 0, EShaderType::Geometry);
        AddShader(ShaderRoot + "LineScreenSpace_GS.cso", 1, EShaderType::Geometry);
        AddShader(ShaderRoot + "SpriteScreenSpace_GS.cso", 2, EShaderType::Geometry);
        AddShader(ShaderRoot + "SpriteWorldSpace_GS.cso", 3, EShaderType::Geometry);
        AddShader(ShaderRoot + "SpriteWorldSpaceEditor_GS.cso", 4, EShaderType::Geometry);
    }

    void CRenderStateManager::InitSamplers()
    {
        AddSampler(ESamplerType::Wrap);
        AddSampler(ESamplerType::Border);
        AddSampler(ESamplerType::Clamp);
    }

    void CRenderStateManager::InitVertexBuffers()
    {
        AddVertexBuffer(GeometryPrimitives::DecalProjector);
        AddVertexBuffer(GeometryPrimitives::PointLightCube);
        AddVertexBuffer(GeometryPrimitives::Icosphere.Vertices);
        AddVertexBuffer(GeometryPrimitives::Line.Vertices);
        AddVertexBuffer(GeometryPrimitives::Pyramid.Vertices);
        AddVertexBuffer(GeometryPrimitives::BoundingBox.Vertices);
        AddVertexBuffer(GeometryPrimitives::Camera.Vertices);
        AddVertexBuffer(GeometryPrimitives::Circle8.Vertices);
        AddVertexBuffer(GeometryPrimitives::Circle16.Vertices);
        AddVertexBuffer(GeometryPrimitives::Circle32.Vertices);
        AddVertexBuffer(GeometryPrimitives::HalfCircle16.Vertices);
        AddVertexBuffer(GeometryPrimitives::Grid.Vertices);
        AddVertexBuffer(GeometryPrimitives::Axis.Vertices);
        AddVertexBuffer(GeometryPrimitives::Octahedron.Vertices);
        AddVertexBuffer(GeometryPrimitives::Square.Vertices);
        AddVertexBuffer(GeometryPrimitives::UVSphere.Vertices);
        AddVertexBuffer(GeometryPrimitives::SkyboxCube);
    }

    void CRenderStateManager::InitIndexBuffers()
    {
        AddIndexBuffer(GeometryPrimitives::DecalProjectorIndices);
        AddIndexBuffer(GeometryPrimitives::PointLightCubeIndices);
        AddIndexBuffer(GeometryPrimitives::Icosphere.Indices);
        AddIndexBuffer(GeometryPrimitives::Line.Indices);
        AddIndexBuffer(GeometryPrimitives::Pyramid.Indices);
        AddIndexBuffer(GeometryPrimitives::BoundingBox.Indices);
        AddIndexBuffer(GeometryPrimitives::Camera.Indices);
        AddIndexBuffer(GeometryPrimitives::Circle8.Indices);
        AddIndexBuffer(GeometryPrimitives::Circle16.Indices);
        AddIndexBuffer(GeometryPrimitives::Circle32.Indices);
        AddIndexBuffer(GeometryPrimitives::HalfCircle16.Indices);
        AddIndexBuffer(GeometryPrimitives::Grid.Indices);
        AddIndexBuffer(GeometryPrimitives::Axis.Indices);
        AddIndexBuffer(GeometryPrimitives::Octahedron.Indices);
        AddIndexBuffer(GeometryPrimitives::Square.Indices);
        AddIndexBuffer(GeometryPrimitives::UVSphere.Indices);
        AddIndexBuffer(GeometryPrimitives::SkyboxCubeIndices);
    }

    void CRenderStateManager::InitMeshVertexStrides()
    {
        AddMeshVertexStride(sizeof(SStaticMeshVertex));
        AddMeshVertexStride(sizeof(SPositionVertex));
        AddMeshVertexStride(sizeof(SSkeletalMeshVertex));
    }

    void CRenderStateManager::InitMeshVertexOffset()
    {
        AddMeshVertexOffset(0);
    }

    U16 CRenderStateManager::AddIndexBuffer(const std::vector<U32>& indices)
    {
        IndexBuffers.emplace_back(CDataBuffer());
        IndexBuffers.back().CreateBuffer("Index Buffer", RHI, sizeof(U32) * STATIC_U32(indices.size()), indices.data(), EDataBufferType::Index, EDataBufferUsage::Immutable, EDataBufferCPUAccess::None);

        return STATIC_U16(IndexBuffers.size() - 1);
    }

    U16 CRenderStateManager::AddMeshVertexStride(U32 stride)
    {
        MeshVertexStrides.emplace_back(stride);
        return STATIC_U16(MeshVertexStrides.size() - 1);
    }

    U16 CRenderStateManager::AddMeshVertexOffset(U32 offset)
    {
        MeshVertexOffsets.emplace_back(offset);
        return STATIC_U16(MeshVertexOffsets.size() - 1);
    }

    U16 CRenderStateManager::AddPipelineStateObject(const SPSODescription& description)
    {
        PSOs.emplace_back(new CPipelineStateObject(RHI, description));
        return STATIC_U16(PSOs.size() - 1);
    }

    std::string CRenderStateManager::AddShader(const std::string& fileName, const U64 index, const EShaderType shaderType)
    {
        std::string outData = "";

        switch (shaderType)
        {
        case EShaderType::Vertex:
        {
            if (VertexShaders[index] != nullptr)
            {
                VertexShaders[index]->Release();
                delete VertexShaders[index];
            }

            VertexShaders[index] = new CShader(fileName, RHI, shaderType);
            outData = VertexShaders[index]->GetCompiledData();
        }
        break;
        case EShaderType::Geometry:
        {
            if (GeometryShaders[index] != nullptr)
            {
                GeometryShaders[index]->Release();
                delete GeometryShaders[index];
            }

            GeometryShaders[index] = new CShader(fileName, RHI, shaderType);
        }
        break;
        case EShaderType::Pixel:
        {
            if (PixelShaders[index] != nullptr)
            {
                PixelShaders[index]->Release();
                delete PixelShaders[index];
            }

            PixelShaders[index] = new CShader(fileName, RHI, shaderType);
        }
        break;
        default:
        {
            HV_LOG_ERROR("CRenderStateManager::AddShader: Could not add shader of type '%s', not supported yet!", magic_enum::enum_name(shaderType).data());
            return "";
        }
        }

        const std::string prefix = UGeneralUtils::ExtractParentDirectoryFromPath(UFileSystem::GetWorkingPath()) + "Source/Engine/Graphics/";
        const std::string extension = "hlsl";
        const std::string sourceFile = prefix + fileName.substr(0, fileName.size() - UGeneralUtils::ExtractFileExtensionFromPath(fileName).size()) + extension;
        if (!ShaderInitData.contains(sourceFile))
        {
            GEngine::GetFileWatcher()->WatchFileChange(sourceFile, SFileChangeCallback(std::bind(&CRenderStateManager::OnShaderSourceChange, this, std::placeholders::_1), OnShaderSourceChangeFunctionHandle));
            ShaderInitData.emplace(sourceFile, SShaderInitData{ fileName, shaderType, index });
        }

        return outData;
    }

    void CRenderStateManager::AddInputLayout(const std::string& vsData, EInputLayoutType layoutType)
    {
        if (InputLayouts.size() > STATIC_U64(layoutType))
            return;

        std::vector<SInputElementDescription> layout;
        switch (layoutType)
        {
        case EInputLayoutType::Pos3Nor3Tan3Bit3UV2:
            layout =
            {
                { .SemanticName = "POSITION",   .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "NORMAL",     .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "TANGENT",    .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "BINORMAL",   .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "UV",         .Format = ERenderResourceFormat::R32G32_Float }
            };
            break;

        case EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans:
            layout =
            {
                { .SemanticName = "POSITION",   .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "NORMAL",     .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "TANGENT",    .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "BINORMAL",   .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "UV",         .Format = ERenderResourceFormat::R32G32_Float },
                { .SemanticName = "INSTANCETRANSFORM", .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 0, .InputSlot = 1, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM", .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 1, .InputSlot = 1, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM", .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 2, .InputSlot = 1, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM", .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 3, .InputSlot = 1, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData }
            };
            break;

        case EInputLayoutType::Pos3Nor3Tan3Bit3UV2BoneID4BoneWeight4AnimDataTrans:
            layout =
            {
                { .SemanticName = "POSITION",               .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "NORMAL",                 .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "TANGENT",                .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "BINORMAL",               .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "UV",                     .Format = ERenderResourceFormat::R32G32_Float },
                { .SemanticName = "BONEID",                 .Format = ERenderResourceFormat::R32G32B32A32_Float },
                { .SemanticName = "BONEWEIGHT",             .Format = ERenderResourceFormat::R32G32B32A32_Float },
                { .SemanticName = "INSTANCEANIMATIONDATA",  .Format = ERenderResourceFormat::R32G32_UnsignedInt, .SemanticIndex = 0, .InputSlot = 1, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 0, .InputSlot = 2, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 1, .InputSlot = 2, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 2, .InputSlot = 2, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 3, .InputSlot = 2, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData }
            };
            break;

        case EInputLayoutType::Pos3Nor3Tan3Bit3UV2Entity2Trans:
            layout =
            {
                { .SemanticName = "POSITION",               .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "NORMAL",                 .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "TANGENT",                .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "BINORMAL",               .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "UV",                     .Format = ERenderResourceFormat::R32G32_Float },
                { .SemanticName = "ENTITY",                 .Format = ERenderResourceFormat::R32G32_UnsignedInt, .SemanticIndex = 0, .InputSlot = 1, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 0, .InputSlot = 2, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 1, .InputSlot = 2, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 2, .InputSlot = 2, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 3, .InputSlot = 2, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData }
            };
            break;

        case EInputLayoutType::Pos3Nor3Tan3Bit3UV2BoneID4BoneWeight4Entity2AnimDataTrans:
            layout =
            {
                { .SemanticName = "POSITION",               .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "NORMAL",                 .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "TANGENT",                .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "BINORMAL",               .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "UV",                     .Format = ERenderResourceFormat::R32G32_Float },
                { .SemanticName = "BONEID",                 .Format = ERenderResourceFormat::R32G32B32A32_Float },
                { .SemanticName = "BONEWEIGHT",             .Format = ERenderResourceFormat::R32G32B32A32_Float },
                { .SemanticName = "ENTITY",                 .Format = ERenderResourceFormat::R32G32_UnsignedInt, .SemanticIndex = 0, .InputSlot = 1, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCEANIMATIONDATA",  .Format = ERenderResourceFormat::R32G32_UnsignedInt, .SemanticIndex = 0, .InputSlot = 2, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 0, .InputSlot = 3, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 1, .InputSlot = 3, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 2, .InputSlot = 3, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 3, .InputSlot = 3, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData }
            };
            break;

        case EInputLayoutType::Position4:
            layout =
            {
                { .SemanticName = "POSITION", .Format = ERenderResourceFormat::R32G32B32A32_Float }
            };
            break;

        case EInputLayoutType::TransUVRectColor:
            layout =
            {
                { .SemanticName = "INSTANCETRANSFORM",  .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 0, .InputSlot = 0, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",  .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 1, .InputSlot = 0, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",  .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 2, .InputSlot = 0, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",  .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 3, .InputSlot = 0, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCEUVRECT",     .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 0, .InputSlot = 1, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCECOLOR",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 0, .InputSlot = 2, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData }
            };
            break;

        case EInputLayoutType::TransUVRectColorEntity2:
            layout =
            {
                { .SemanticName = "INSTANCETRANSFORM",  .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 0, .InputSlot = 0, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",  .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 1, .InputSlot = 0, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",  .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 2, .InputSlot = 0, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCETRANSFORM",  .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 3, .InputSlot = 0, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCEUVRECT",     .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 0, .InputSlot = 1, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "INSTANCECOLOR",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 0, .InputSlot = 2, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData },
                { .SemanticName = "ENTITY",             .Format = ERenderResourceFormat::R32G32_UnsignedInt, .SemanticIndex = 0, .InputSlot = 3, .InstanceDataStepRate = 1, .InputClassification = ERenderInputClassification::InputPerInstanceData }
            };
            break;
        
        case EInputLayoutType::Pos3Nor3Tan3Bit3UV2Color4:
            layout =
            {
                { .SemanticName = "POSITION",   .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "NORMAL",     .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "TANGENT",    .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "BINORMAL",   .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "UV",         .Format = ERenderResourceFormat::R32G32_Float },
                { .SemanticName = "COLOR",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 0, .InputSlot = 1 }
            };
            break;

        case EInputLayoutType::Pos3Nor3Tan3Bit3UV2Color4Entity2:
            layout =
            {
                { .SemanticName = "POSITION",   .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "NORMAL",     .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "TANGENT",    .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "BINORMAL",   .Format = ERenderResourceFormat::R32G32B32_Float },
                { .SemanticName = "UV",         .Format = ERenderResourceFormat::R32G32_Float },
                { .SemanticName = "COLOR",      .Format = ERenderResourceFormat::R32G32B32A32_Float, .SemanticIndex = 0, .InputSlot = 1 },
                { .SemanticName = "ENTITY",     .Format = ERenderResourceFormat::R32G32_UnsignedInt, .SemanticIndex = 0, .InputSlot = 2 }
            };
            break;
        }

        SInputLayoutDescription layoutDescription = SInputLayoutDescription{ .Layout = layout };
        InputLayouts.emplace_back(new CVertexInputLayout(RHI, layoutDescription, vsData));
    }

    void CRenderStateManager::AddSampler(ESamplerType samplerType)
    {
        // TODO.NR: Extend to different LOD levels and filters
        SSamplerDescription samplerDescription;
        samplerDescription.BorderColor[0] = 1.0f;
        samplerDescription.BorderColor[1] = 1.0f;
        samplerDescription.BorderColor[2] = 1.0f;
        samplerDescription.BorderColor[3] = 1.0f;
        samplerDescription.Filter = ESamplerFilter::MinMagMipLinear;
        samplerDescription.MinLOD = 0;
        samplerDescription.MaxLOD = 10;

        switch (samplerType)
        {
        case ESamplerType::Border:
            samplerDescription.AddressU = ETextureAddressMode::Border;
            samplerDescription.AddressV = ETextureAddressMode::Border;
            samplerDescription.AddressW = ETextureAddressMode::Border;
            break;
        case ESamplerType::Clamp:
            samplerDescription.AddressU = ETextureAddressMode::Clamp;
            samplerDescription.AddressV = ETextureAddressMode::Clamp;
            samplerDescription.AddressW = ETextureAddressMode::Clamp;
            break;
        case ESamplerType::Mirror:
            samplerDescription.AddressU = ETextureAddressMode::Mirror;
            samplerDescription.AddressV = ETextureAddressMode::Mirror;
            samplerDescription.AddressW = ETextureAddressMode::Mirror;
            break;
        case ESamplerType::Wrap:
            samplerDescription.AddressU = ETextureAddressMode::Wrap;
            samplerDescription.AddressV = ETextureAddressMode::Wrap;
            samplerDescription.AddressW = ETextureAddressMode::Wrap;
            break;
        }

        Samplers.emplace_back(new CSamplerState(RHI, samplerDescription));
    }

    void CRenderStateManager::AddViewport(SVector2<F32> topLeftCoordinate, SVector2<F32> widthAndHeight, SVector2<F32> depth)
    {
        Viewports.emplace_back(CRenderViewport(RHI, topLeftCoordinate.X, topLeftCoordinate.Y, widthAndHeight.X, widthAndHeight.Y, depth.X, depth.Y));
    }

    void CRenderStateManager::IASetTopology(ETopologies topology) const
    {
        Context->IASetPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(topology));
    }

    void CRenderStateManager::IASetInputLayout(EInputLayoutType layout) const
    {
        if (layout == EInputLayoutType::Null)
        {
            CVertexInputLayout::ResetInputLayout(RHI);
            return;
        }

        InputLayouts[STATIC_U8(layout)]->SetInputLayout();
    }

    void CRenderStateManager::IASetVertexBuffer(U8 startSlot, const CDataBuffer& buffer, U32 stride, U32 offset) const
    {
        Context->IASetVertexBuffers(startSlot, 1, &buffer.Buffer, &stride, &offset);
    }

    void CRenderStateManager::IASetVertexBuffers(U8 startSlot, U8 numberOfBuffers, const std::vector<CDataBuffer>& buffers, const U32* strides, const U32* offsets) const
    {
        std::vector<ID3D11Buffer*> bufferPointers;
        for (const CDataBuffer& buffer : buffers)
            bufferPointers.emplace_back(buffer.Buffer);

        Context->IASetVertexBuffers(startSlot, numberOfBuffers, bufferPointers.data(), strides, offsets);
    }

    void CRenderStateManager::IASetIndexBuffer(const CDataBuffer& buffer) const
    {
        if (buffer.Buffer == nullptr)
        {
			Context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
            return;
        }

    	Context->IASetIndexBuffer(buffer.Buffer, DXGI_FORMAT_R32_UINT, 0);
    }

    void CRenderStateManager::VSSetShader(EVertexShaders shader) const
    {
        if (shader == EVertexShaders::Null)
        {
            CShader::ResetShader(RHI, EShaderType::Vertex);
            return;
        }

        VertexShaders[STATIC_U8(shader)]->SetShader();
    }

    void CRenderStateManager::VSSetConstantBuffer(U8 slot, const CDataBuffer& buffer)
    {
        Context->VSSetConstantBuffers(slot, 1, &buffer.Buffer);
    }

    void CRenderStateManager::VSSetResources(U8 startSlot, U8 numberOfResources, ID3D11ShaderResourceView* const* resources)
    {
        Context->VSSetShaderResources(startSlot, numberOfResources, resources);
    }

    void CRenderStateManager::GSSetShader(EGeometryShaders shader) const
    {
        if (shader == EGeometryShaders::Null)
        {
            CShader::ResetShader(RHI, EShaderType::Geometry);
            return;
        }

        GeometryShaders[STATIC_U8(shader)]->SetShader();
    }

    void CRenderStateManager::GSSetConstantBuffer(U8 slot, const CDataBuffer& buffer) const
    {
        Context->GSSetConstantBuffers(slot, 1, &buffer.Buffer);
    }

    void CRenderStateManager::PSSetSampler(U8 slot, ESamplers sampler) const
    {
        Samplers[STATIC_U8(sampler)]->SetSamplerState(slot);
    }

    void CRenderStateManager::PSSetShader(EPixelShaders shader) const
    {
        if (shader == EPixelShaders::Null)
        {
            CShader::ResetShader(RHI, EShaderType::Pixel);
            return;
        }

        PixelShaders[STATIC_U8(shader)]->SetShader();
    }

    void CRenderStateManager::PSSetConstantBuffer(U8 slot, const CDataBuffer& buffer) const
    {
        Context->PSSetConstantBuffers(slot, 1, &buffer.Buffer);
    }

    void CRenderStateManager::PSSetResources(U8 startSlot, U8 numberOfResources, ID3D11ShaderResourceView* const* resources)
    {
        Context->PSSetShaderResources(startSlot, numberOfResources, resources);
    }

    void CRenderStateManager::RSSetRasterizerState(ERasterizerStates rasterizerState) const
    {
        if (rasterizerState == ERasterizerStates::Default)
        {
            CRasterizerState::ResetRasterizerState(RHI);
            return;
        }

        RasterizerStates[STATIC_U8(rasterizerState)]->SetRasterizerState();
    }

    void CRenderStateManager::OMSetBlendState(EBlendStates blendState) const
    {
        if (blendState == EBlendStates::Disable)
        {
            CBlendState::ResetBlendState(RHI);
            return;
        }

        BlendStates[STATIC_U8(blendState)]->SetBlendState();
    }

    void CRenderStateManager::OMSetDepthStencilState(EDepthStencilStates depthStencilState, U32 stencilRef) const
    {
        if (depthStencilState == EDepthStencilStates::Default)
        {
            CDepthStencilState::ResetDepthStencilState(RHI);
            return;
        }

        DepthStencilStates[STATIC_U8(depthStencilState)]->SetDepthStencilState(stencilRef);
    }

    void CRenderStateManager::OMSetRenderTargets(U8 numberOfTargets, ID3D11RenderTargetView* const* targetViews, ID3D11DepthStencilView* depthStencilView) const
    {
        Context->OMSetRenderTargets(numberOfTargets, targetViews, depthStencilView);
    }

    void CRenderStateManager::Draw(U32 vertexCount, U32 startVertexLocation) const
    {
        // TODO.NW: Increase draw calls in all the draw functions instead of at the call sites
        Context->Draw(vertexCount, startVertexLocation);
    }

    void CRenderStateManager::DrawIndexed(U32 indexCount, U32 startIndexLocation, U32 baseVertexLocation) const
    {
        Context->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void CRenderStateManager::DrawInstanced(U32 vertexCountPerInstance, U32 numberOfInstances, U32 startVertexLocation, U32 startInstanceLocation) const
    {
        Context->DrawInstanced(vertexCountPerInstance, numberOfInstances, startVertexLocation, startInstanceLocation);
    }

    void CRenderStateManager::DrawIndexedInstanced(U32 indexCountPerInstance, U32 instanceCount, U32 startIndexLocation, U32 baseVertexLocation, U32 startInstanceLocation) const
    {
        Context->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }

    U64 CRenderStateManager::TrySetPipelineStateObject(const U16 psoIndex, const U64 currentPSOHash)
    {
        return PSOs[psoIndex]->TrySetPipelineState(currentPSOHash);
    }

    void CRenderStateManager::SetAllStates(EBlendStates blendState, EDepthStencilStates depthStencilState, ERasterizerStates rasterizerState) const
    {
        OMSetBlendState(blendState);
        OMSetDepthStencilState(depthStencilState);
        RSSetRasterizerState(rasterizerState);
    }

    void CRenderStateManager::SetAllDefault() const
    {
        OMSetBlendState(EBlendStates::Disable);
        OMSetDepthStencilState(EDepthStencilStates::Default);
        RSSetRasterizerState(ERasterizerStates::Default);
    }

    void CRenderStateManager::ClearState()
    {
        Context->ClearState();
    }

    void CRenderStateManager::ClearShaderResources() const
    {
        ID3D11ShaderResourceView* nullView = NULL;
        Context->PSSetShaderResources(0, 1, &nullView);
        Context->PSSetShaderResources(1, 1, &nullView);
        Context->PSSetShaderResources(2, 1, &nullView);
        Context->PSSetShaderResources(3, 1, &nullView);
        Context->PSSetShaderResources(4, 1, &nullView);
        Context->PSSetShaderResources(5, 1, &nullView);
        Context->PSSetShaderResources(8, 1, &nullView);
        Context->PSSetShaderResources(9, 1, &nullView);
        Context->PSSetShaderResources(21, 1, &nullView);
        Context->PSSetShaderResources(22, 1, &nullView);
    }

    void CRenderStateManager::Release()
    {
        for (U8 i = 0; i < STATIC_U8(EBlendStates::Count); ++i)
        {
            BlendStates[i]->Release();
        }

        for (U8 i = 0; i < STATIC_U8(EDepthStencilStates::Count); ++i)
        {
            DepthStencilStates[i]->Release();
        }

        for (U8 i = 0; i < STATIC_U8(ERasterizerStates::Count); ++i)
        {
            RasterizerStates[i]->Release();
        }
    }

    void CRenderStateManager::OnShaderSourceChange(const std::string& filePath)
    {
        std::lock_guard<std::mutex> lock(ShaderRecompileMutex);
        QueuedShaderRecompiles.push(filePath);
    }

    class UShaderIncludeHandler : public ID3DInclude
    {
        HRESULT Open(D3D_INCLUDE_TYPE /*includeType*/, LPCSTR pFileName, LPCVOID /*pParentData*/, LPCVOID* ppData, UINT* pBytes) override
        {
            // NW: Only include files in the Shaders/Includes folder in shaders.
            const std::string shaderIncludeSource = UGeneralUtils::ExtractParentDirectoryFromPath(UFileSystem::GetWorkingPath()) + "Source/Engine/Graphics/Shaders/Includes/";
            const std::string inputFileName = UGeneralUtils::ExtractFileNameFromPath(pFileName);
            const std::string filePath = shaderIncludeSource + inputFileName;

            if (!UFileSystem::Exists(filePath))
                return E_FAIL;

            U32 fileSize = STATIC_U32(UFileSystem::GetFileSize(filePath));
            char* data = new char[fileSize];
            UFileSystem::Deserialize(filePath, data, fileSize);

            *pBytes = fileSize;
            *ppData = data;
            
            return S_OK;
        }

        HRESULT Close(LPCVOID pData) override
        {
            delete[] pData;
            return S_OK;
        }
    };

    void CRenderStateManager::FlushShaderChanges()
    {
        // NW: Use DXC.exe for shader models 6 and above, or one of the vulkan shader compilers to compile into SPIR-V, e.g. glslc.exe or glslang https://github.com/KhronosGroup/glslang
        // https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-part1

        std::lock_guard<std::mutex> lock(ShaderRecompileMutex);
        while (!QueuedShaderRecompiles.empty())
        {
            const std::string& recompiledSourceFile = QueuedShaderRecompiles.front();

            const SShaderInitData initData = ShaderInitData.at(recompiledSourceFile);
            const std::wstring wideSourceFilePath = { recompiledSourceFile.begin(), recompiledSourceFile.end() };
            const std::wstring wideOutputFilePath = { initData.OutputFileName.begin(), initData.OutputFileName.end() };

            ID3DBlob* blob = nullptr;
            ID3DBlob* errorMessages = nullptr;

            std::string shaderModel;
            switch (initData.ShaderType)
            {
            case EShaderType::Pixel:
                shaderModel = "ps_5_0";
                break;
            case EShaderType::Geometry:
                shaderModel = "gs_5_0";
                break;
            case EShaderType::Compute:
                shaderModel = "cs_5_0";
                break;
            case EShaderType::Vertex:
                [[fallthrough]];
            default:
                shaderModel = "vs_5_0";
            }

            UShaderIncludeHandler includeHandler;
            const HRESULT compileResult = D3DCompileFromFile(wideSourceFilePath.c_str(), nullptr, &includeHandler, "main", shaderModel.c_str(), 0, 0, &blob, &errorMessages);
            if (compileResult != S_OK)
            {
                HV_LOG_ERROR("CRenderStateManager::OnShaderSourceChange: Shader %s could not be recompiled: %s", recompiledSourceFile.c_str(), (char*)errorMessages->GetBufferPointer());
                QueuedShaderRecompiles.pop();
                errorMessages->Release();
                break;
            }

            const HRESULT rewriteResult = D3DWriteBlobToFile(blob, wideOutputFilePath.c_str(), TRUE);
            if (rewriteResult != S_OK)
            {
                HV_LOG_ERROR("CRenderStateManager::OnShaderSourceChange: Shader %s was successfully recompiled, but output file could not be overwritten.", recompiledSourceFile.c_str());
                QueuedShaderRecompiles.pop();
                blob->Release();
                break;
            }

            blob->Release();
            AddShader(initData.OutputFileName, initData.ShaderIndex, initData.ShaderType);

            HV_LOG_INFO("Shader source file %s recompiled.", recompiledSourceFile.c_str());

            QueuedShaderRecompiles.pop();
        }
    }

    bool CRenderStateManager::CreateBlendStates(CRHI* rhi)
    {
        SBlendStateDescription alphaBlend;
        alphaBlend.EnableIndependentBlend = true;
        for (U8 i = 0; i < 8; i++)
        {
            SRenderTargetBlendDescription& targetBlendDescription = alphaBlend.RenderTargetDescriptions[i];
            targetBlendDescription.EnableBlend = true;
            targetBlendDescription.SourceBlend = EBlendFactor::SourceAlpha;
            targetBlendDescription.DestinationBlend = EBlendFactor::InverseSourceAlpha;
            targetBlendDescription.BlendOperation = EBlendOperation::Add;
            targetBlendDescription.SourceAlphaBlend = EBlendFactor::One;
            targetBlendDescription.DestinationAlphaBlend = EBlendFactor::One;
            targetBlendDescription.AlphaBlendOperation = EBlendOperation::Maximum;
            targetBlendDescription.RenderTargetWriteMask = STATIC_U8(EBlendColorWriteEnable::All);
        }

        SBlendStateDescription additiveBlend; 
        {
            SRenderTargetBlendDescription& targetBlendDescription = additiveBlend.RenderTargetDescriptions[0];
            targetBlendDescription.EnableBlend = true;
            targetBlendDescription.SourceBlend = EBlendFactor::SourceAlpha;
            targetBlendDescription.DestinationBlend = EBlendFactor::One;
            targetBlendDescription.BlendOperation = EBlendOperation::Add;
            targetBlendDescription.SourceAlphaBlend = EBlendFactor::One;
            targetBlendDescription.DestinationAlphaBlend = EBlendFactor::One;
            targetBlendDescription.AlphaBlendOperation = EBlendOperation::Maximum;
            targetBlendDescription.RenderTargetWriteMask = STATIC_U8(EBlendColorWriteEnable::All);
        }
        
        SBlendStateDescription gBufferBlend;
        gBufferBlend.EnableIndependentBlend = true;
        for (U8 i = 0; i < 4; i++) // 4 targets in GBuffer
        {
            SRenderTargetBlendDescription& targetBlendDescription = gBufferBlend.RenderTargetDescriptions[i];
            targetBlendDescription.EnableBlend = true;
            targetBlendDescription.SourceBlend = EBlendFactor::SourceAlpha;
            targetBlendDescription.DestinationBlend = EBlendFactor::InverseSourceAlpha;
            targetBlendDescription.BlendOperation = EBlendOperation::Add;
            targetBlendDescription.SourceAlphaBlend = EBlendFactor::One;
            targetBlendDescription.DestinationAlphaBlend = EBlendFactor::Zero;
            targetBlendDescription.AlphaBlendOperation = EBlendOperation::Add;
            targetBlendDescription.RenderTargetWriteMask = STATIC_U8(EBlendColorWriteEnable::All);
        }

        BlendStates[(U64)EBlendStates::AlphaBlend] = new CBlendState(rhi, alphaBlend);
        BlendStates[(U64)EBlendStates::AdditiveBlend] = new CBlendState(rhi, additiveBlend);
        BlendStates[(U64)EBlendStates::GBufferAlphaBlend] = new CBlendState(rhi, gBufferBlend);

        return true;
    }

    bool CRenderStateManager::CreateDepthStencilStates(CRHI* rhi)
    {
        SDepthStencilDescription onlyReadDepthStencilDescription;
        onlyReadDepthStencilDescription.EnableDepth = true;
        onlyReadDepthStencilDescription.DepthWriteMask = EDepthWriteMask::Zero;
        onlyReadDepthStencilDescription.DepthFunction = ERenderComparisonFunction::Less;
        onlyReadDepthStencilDescription.EnableStencil = false;

        SDepthStencilDescription stencilWriteDescription;
        stencilWriteDescription.EnableDepth = true;
        stencilWriteDescription.DepthWriteMask = EDepthWriteMask::All;
        stencilWriteDescription.DepthFunction = ERenderComparisonFunction::Less;
        stencilWriteDescription.EnableStencil = true;
        stencilWriteDescription.StencilReadMask = 0xFF;
        stencilWriteDescription.StencilWriteMask = 0xFF;
        stencilWriteDescription.FrontFaceStencilOperation.StencilFailOperation = EDepthStencilOperation::Keep;
        stencilWriteDescription.FrontFaceStencilOperation.StencilDepthFailOperation = EDepthStencilOperation::Keep;
        stencilWriteDescription.FrontFaceStencilOperation.StencilPassOperation = EDepthStencilOperation::Replace;
        stencilWriteDescription.FrontFaceStencilOperation.StencilFunction = ERenderComparisonFunction::Always;

        SDepthStencilDescription stencilMaskDescription;
        stencilMaskDescription.EnableDepth = false;
        stencilMaskDescription.DepthWriteMask = EDepthWriteMask::Zero;
        stencilMaskDescription.EnableStencil = true;
        stencilMaskDescription.StencilReadMask = 0xFF;
        stencilMaskDescription.StencilWriteMask = 0xFF;
        stencilMaskDescription.FrontFaceStencilOperation.StencilPassOperation = EDepthStencilOperation::Keep;
        stencilMaskDescription.FrontFaceStencilOperation.StencilFailOperation = EDepthStencilOperation::Keep;
        stencilMaskDescription.FrontFaceStencilOperation.StencilFunction = ERenderComparisonFunction::NotEqual;

        SDepthStencilDescription depthFirstDescription;
        depthFirstDescription.EnableDepth = true;
        depthFirstDescription.DepthWriteMask = EDepthWriteMask::Zero;
        depthFirstDescription.DepthFunction = ERenderComparisonFunction::LessOrEqual;
        depthFirstDescription.EnableStencil = false;

        DepthStencilStates[STATIC_U8(EDepthStencilStates::OnlyRead)] = new CDepthStencilState(rhi, onlyReadDepthStencilDescription);
        DepthStencilStates[STATIC_U8(EDepthStencilStates::StencilWrite)] = new CDepthStencilState(rhi, stencilWriteDescription);
        DepthStencilStates[STATIC_U8(EDepthStencilStates::StencilMask)] = new CDepthStencilState(rhi, stencilMaskDescription);;
        DepthStencilStates[STATIC_U8(EDepthStencilStates::DepthFirst)] = new CDepthStencilState(rhi, depthFirstDescription);;

        return true;
    }

    bool CRenderStateManager::CreateRasterizerStates(CRHI* rhi)
    {
        SRasterizerDescription wireframeRasterizerStateDescription;
        wireframeRasterizerStateDescription.FillMode = ERasterizerFillMode::Wireframe;
        wireframeRasterizerStateDescription.CullMode = ERasterizerCullMode::Backface;

        SRasterizerDescription backfaceRasterizerStateDescription;

        SRasterizerDescription frontfaceRasterizerStateDescription;
        frontfaceRasterizerStateDescription.FillMode = ERasterizerFillMode::Solid;
        frontfaceRasterizerStateDescription.CullMode = ERasterizerCullMode::Frontface;

        SRasterizerDescription noFaceCullingRasterizerStateDescription;
        noFaceCullingRasterizerStateDescription.FillMode = ERasterizerFillMode::Solid;
        noFaceCullingRasterizerStateDescription.CullMode = ERasterizerCullMode::None;

        RasterizerStates[STATIC_U8(ERasterizerStates::Wireframe)] = new CRasterizerState(rhi, wireframeRasterizerStateDescription);
        RasterizerStates[STATIC_U8(ERasterizerStates::BackfaceCulling)] = new CRasterizerState(rhi, backfaceRasterizerStateDescription);
        RasterizerStates[STATIC_U8(ERasterizerStates::FrontfaceCulling)] = new CRasterizerState(rhi, frontfaceRasterizerStateDescription);
        RasterizerStates[STATIC_U8(ERasterizerStates::NoFaceCulling)] = new CRasterizerState(rhi, noFaceCullingRasterizerStateDescription);

        return true;
    }
}
