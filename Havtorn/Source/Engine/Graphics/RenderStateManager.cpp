// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RenderStateManager.h"

#include "GeometryPrimitives.h"
#include "GraphicsFramework.h"
#include "GraphicsUtilities.h"

#include "FileSystem/FileWatcher.h"

#include <d3dcompiler.h>

namespace Havtorn
{
    CRenderStateManager::~CRenderStateManager()
    {
        Context = nullptr;
    }

    bool CRenderStateManager::Init(CGraphicsFramework* framework)
    {
        Framework = framework;
        Context = framework->GetContext();
        ID3D11Device* device = framework->GetDevice();

        ENGINE_ERROR_BOOL_MESSAGE(Context, "Could not bind context.");
        ENGINE_ERROR_BOOL_MESSAGE(device, "Device is null.");

        ENGINE_ERROR_BOOL_MESSAGE(CreateBlendStates(device), "Could not create Blend States.");
        ENGINE_ERROR_BOOL_MESSAGE(CreateDepthStencilStates(device), "Could not create Depth Stencil States.");
        ENGINE_ERROR_BOOL_MESSAGE(CreateRasterizerStates(device), "Could not create Rasterizer States.");

        // Load default resources
        InitVertexShadersAndInputLayouts();
        InitPixelShaders();
        InitGeometryShaders();
        InitSamplers();
        InitVertexBuffers();
        InitIndexBuffers();
        InitTopologies();
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
        }

        for (U64 i = 0; i < STATIC_U64(EVertexShaders::Count); i++)
        {
            std::string vsData = AddShader(initData[i].FileName, i, EShaderType::Vertex);
            if (initData[i].ShouldAddLayout)
                AddInputLayout(vsData, initData[i].InputLayout);
        }

        // NW: Null shader. Adding this to avoid branching in state setting functions
        VertexShaders[STATIC_U64(EVertexShaders::Count)] = nullptr;
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
        }

        for (U64 i = 0; i < STATIC_U64(EPixelShaders::Count); i++)
            AddShader(filepaths[i], i, EShaderType::Pixel);

        // NW: Null shader. Adding this to avoid branching in state setting functions
        PixelShaders[STATIC_U64(EPixelShaders::Count)] = nullptr;
    }

    void CRenderStateManager::InitGeometryShaders()
    {
        AddShader(ShaderRoot + "Line_GS.cso", 0, EShaderType::Geometry);
        AddShader(ShaderRoot + "LineScreenSpace_GS.cso", 1, EShaderType::Geometry);
        AddShader(ShaderRoot + "SpriteScreenSpace_GS.cso", 2, EShaderType::Geometry);
        AddShader(ShaderRoot + "SpriteWorldSpace_GS.cso", 3, EShaderType::Geometry);
        AddShader(ShaderRoot + "SpriteWorldSpaceEditor_GS.cso", 4, EShaderType::Geometry);

        // NW: Null shader. Adding this to avoid branching in state setting functions
        GeometryShaders[STATIC_U64(EGeometryShaders::Count)] = nullptr;
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

    void CRenderStateManager::InitTopologies()
    {
        AddTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        AddTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        AddTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
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
        IndexBuffers.back().CreateBuffer("Index Buffer", Framework, sizeof(U32) * STATIC_U32(indices.size()), indices.data(), EDataBufferType::Index, EDataBufferUsage::Immutable, EDataBufferCPUAccess::None);

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

    std::string CRenderStateManager::AddShader(const std::string& fileName, const U64 index, const EShaderType shaderType)
    {
        std::string outShaderData;

        switch (shaderType)
        {
        case EShaderType::Vertex:
        {
            if (VertexShaders[index] != nullptr)
                VertexShaders[index]->Release();

            ID3D11VertexShader* vertexShader;
            UGraphicsUtils::CreateVertexShader(fileName, Framework, &vertexShader, outShaderData);
            VertexShaders[index] = vertexShader;
        }
        break;
        case EShaderType::Compute:
        case EShaderType::Geometry:
        {
            if (GeometryShaders[index] != nullptr)
                GeometryShaders[index]->Release();

            ID3D11GeometryShader* geometryShader;
            UGraphicsUtils::CreateGeometryShader(fileName, Framework, &geometryShader);
            GeometryShaders[index] = geometryShader;
        }
        break;
        case EShaderType::Pixel:
        {
            if (PixelShaders[index] != nullptr)
                PixelShaders[index]->Release();

            ID3D11PixelShader* pixelShader;
            UGraphicsUtils::CreatePixelShader(fileName, Framework, &pixelShader);
            PixelShaders[index] = pixelShader;
        }
        break;
        }

        const std::string prefix = UGeneralUtils::ExtractParentDirectoryFromPath(UFileSystem::GetWorkingPath()) + "Source/Engine/Graphics/";
        const std::string extension = "hlsl";
        const std::string sourceFile = prefix + fileName.substr(0, fileName.size() - UGeneralUtils::ExtractFileExtensionFromPath(fileName).size()) + extension;
        if (!ShaderInitData.contains(sourceFile))
        {
            GEngine::GetFileWatcher()->WatchFileChange(sourceFile, SFileChangeCallback(std::bind(&CRenderStateManager::OnShaderSourceChange, this, std::placeholders::_1), OnShaderSourceChangeFunctionHandle));
            ShaderInitData.emplace(sourceFile, SShaderInitData{ fileName, shaderType, index });
        }

        return outShaderData;
    }

    void CRenderStateManager::AddInputLayout(const std::string& vsData, EInputLayoutType layoutType)
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
        switch (layoutType)
        {
        case EInputLayoutType::Pos3Nor3Tan3Bit3UV2:
            layout =
            {
                {"POSITION"	,	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"NORMAL"   ,   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TANGENT"  ,   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"BINORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"UV"		,   0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
            };
            break;

        case EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans:
            layout =
            {
                {"POSITION"	,	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"NORMAL"   ,   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TANGENT"  ,   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"BINORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"UV"		,   0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"INSTANCETRANSFORM",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
            };
            break;

        case EInputLayoutType::Pos3Nor3Tan3Bit3UV2BoneID4BoneWeight4AnimDataTrans:
            layout =
            {
                {"POSITION"	,	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"NORMAL"   ,   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TANGENT"  ,   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"BINORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"UV"		,   0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"BONEID",      0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"BONEWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"INSTANCEANIMATIONDATA",   0, DXGI_FORMAT_R32G32_UINT,	1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	1, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	2, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	3, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
            };
            break;

        case EInputLayoutType::Pos3Nor3Tan3Bit3UV2Entity2Trans:
            layout =
            {
                {"POSITION"	,	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"NORMAL"   ,   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TANGENT"  ,   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"BINORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"UV"		,   0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"ENTITY"		,   0, DXGI_FORMAT_R32G32_UINT,	1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	1, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	2, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	3, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
            };
            break;

        case EInputLayoutType::Pos3Nor3Tan3Bit3UV2BoneID4BoneWeight4Entity2AnimDataTrans:
            layout =
            {
                {"POSITION"	,	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"NORMAL"   ,   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TANGENT"  ,   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"BINORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"UV"		,   0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"BONEID",      0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"BONEWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"ENTITY"		,   0, DXGI_FORMAT_R32G32_UINT,	1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCEANIMATIONDATA",   0, DXGI_FORMAT_R32G32_UINT,	2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 3, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	1, DXGI_FORMAT_R32G32B32A32_FLOAT, 3, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	2, DXGI_FORMAT_R32G32B32A32_FLOAT, 3, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	3, DXGI_FORMAT_R32G32B32A32_FLOAT, 3, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
            };
            break;

        case EInputLayoutType::Position4:
            layout =
            {
                {"POSITION"	,	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
            };
            break;

        case EInputLayoutType::TransUVRectColor:
            layout =
            {
                {"INSTANCETRANSFORM",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCEUVRECT",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCECOLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
            };
            break;

        case EInputLayoutType::TransUVRectColorEntity2:
            layout =
            {
                {"INSTANCETRANSFORM",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCETRANSFORM",	3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCEUVRECT",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"INSTANCECOLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
                {"ENTITY"		,       0, DXGI_FORMAT_R32G32_UINT,	       3, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
            };
            break;
        }
        ID3D11InputLayout* inputLayout;
        ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateInputLayout(layout.data(), STATIC_U32(layout.size()), vsData.data(), vsData.size(), &inputLayout), "Input Layout could not be created.")
            InputLayouts.emplace_back(inputLayout);
    }

    void CRenderStateManager::AddSampler(ESamplerType samplerType)
    {
        // TODO.NR: Extend to different LOD levels and filters
        D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
        samplerDesc.BorderColor[0] = 1.0f;
        samplerDesc.BorderColor[1] = 1.0f;
        samplerDesc.BorderColor[2] = 1.0f;
        samplerDesc.BorderColor[3] = 1.0f;
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = 10;

        switch (samplerType)
        {
        case ESamplerType::Border:
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
            break;
        case ESamplerType::Clamp:
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            break;
        case ESamplerType::Mirror:
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
            break;
        case ESamplerType::Wrap:
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            break;
        }

        ID3D11SamplerState* samplerState;
        ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateSamplerState(&samplerDesc, &samplerState), "Sampler could not be created.");
        Samplers.emplace_back(samplerState);
    }

    void CRenderStateManager::AddTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
    {
        Topologies.emplace_back(topology);
    }

    void CRenderStateManager::AddViewport(SVector2<F32> topLeftCoordinate, SVector2<F32> widthAndHeight, SVector2<F32> depth)
    {
        Viewports.emplace_back(D3D11_VIEWPORT(topLeftCoordinate.X, topLeftCoordinate.Y, widthAndHeight.X, widthAndHeight.Y, depth.X, depth.Y));
    }

    void CRenderStateManager::IASetTopology(ETopologies topology) const
    {
        Context->IASetPrimitiveTopology(Topologies[STATIC_U8(topology)]);
    }

    void CRenderStateManager::IASetInputLayout(EInputLayoutType layout) const
    {
        Context->IASetInputLayout(InputLayouts[STATIC_U8(layout)]);
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
			Context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

    	Context->IASetIndexBuffer(buffer.Buffer, DXGI_FORMAT_R32_UINT, 0);
    }

    void CRenderStateManager::VSSetShader(EVertexShaders shader) const
    {
        Context->VSSetShader(VertexShaders[STATIC_U8(shader)], nullptr, 0);
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
        Context->GSSetShader(GeometryShaders[STATIC_U8(shader)], nullptr, 0);
    }

    void CRenderStateManager::GSSetConstantBuffer(U8 slot, const CDataBuffer& buffer) const
    {
        Context->GSSetConstantBuffers(slot, 1, &buffer.Buffer);
    }

    void CRenderStateManager::PSSetSampler(U8 slot, ESamplers sampler) const
    {
        ID3D11SamplerState* samplerState = Samplers[STATIC_U8(sampler)];
        Context->PSSetSamplers(slot, 1, &samplerState);
    }

    void CRenderStateManager::PSSetShader(EPixelShaders shader) const
    {
        Context->PSSetShader(PixelShaders[STATIC_U8(shader)], nullptr, 0);
    }

    void CRenderStateManager::PSSetConstantBuffer(U8 slot, const CDataBuffer& buffer) const
    {
        Context->PSSetConstantBuffers(slot, 1, &buffer.Buffer);
    }

    void CRenderStateManager::PSSetResources(U8 startSlot, U8 numberOfResources, ID3D11ShaderResourceView* const* resources)
    {
        Context->PSSetShaderResources(startSlot, numberOfResources, resources);
    }

    void CRenderStateManager::RSSetViewports(U8 numberOfViewports, const D3D11_VIEWPORT* viewports)
    {
        Context->RSSetViewports(numberOfViewports, viewports);
    }

    void CRenderStateManager::RSSetRasterizerState(ERasterizerStates rasterizerState) const
    {
        Context->RSSetState(RasterizerStates[(size_t)rasterizerState]);
    }

    void CRenderStateManager::OMSetBlendState(EBlendStates blendState) const
    {
        std::array<F32, 4> blendFactors = { 0.5f, 0.5f, 0.5f, 0.5f };
        Context->OMSetBlendState(BlendStates[(U64)blendState], blendFactors.data(), 0xFFFFFFFFu);
    }

    void CRenderStateManager::OMSetDepthStencilState(EDepthStencilStates depthStencilState, U32 stencilRef) const
    {
        Context->OMSetDepthStencilState(DepthStencilStates[(U64)depthStencilState], stencilRef);
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

    bool CRenderStateManager::CreateBlendStates(ID3D11Device* device)
    {
        D3D11_BLEND_DESC alphaBlendDesc = { 0 };
        alphaBlendDesc.RenderTarget[0].BlendEnable = true;
        alphaBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        alphaBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        alphaBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        alphaBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        alphaBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        alphaBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
        alphaBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        alphaBlendDesc.IndependentBlendEnable = true;
        alphaBlendDesc.RenderTarget[1].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        alphaBlendDesc.RenderTarget[1].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        alphaBlendDesc.RenderTarget[1].BlendOp = D3D11_BLEND_OP_ADD;
        alphaBlendDesc.RenderTarget[1].SrcBlendAlpha = D3D11_BLEND_ONE;
        alphaBlendDesc.RenderTarget[1].DestBlendAlpha = D3D11_BLEND_ONE;
        alphaBlendDesc.RenderTarget[1].BlendOpAlpha = D3D11_BLEND_OP_MAX;
        alphaBlendDesc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        D3D11_BLEND_DESC additiveBlendDesc = { 0 };
        additiveBlendDesc.RenderTarget[0].BlendEnable = true;
        additiveBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        additiveBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
        additiveBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        additiveBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        additiveBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        additiveBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
        additiveBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        CD3D11_BLEND_DESC gbufferBlendDesc{ D3D11_DEFAULT };
        gbufferBlendDesc.IndependentBlendEnable = true;
        for (unsigned int i = 0; i < 4; ++i) // 4 targets in GBuffer
        {
            gbufferBlendDesc.RenderTarget[i].BlendEnable = TRUE;
            gbufferBlendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            gbufferBlendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            gbufferBlendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
            gbufferBlendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
            gbufferBlendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
            gbufferBlendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            gbufferBlendDesc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
        }
        gbufferBlendDesc.RenderTarget[4].BlendEnable = false;

        ID3D11BlendState* alphaBlendState;
        ENGINE_HR_MESSAGE(device->CreateBlendState(&alphaBlendDesc, &alphaBlendState), "Alpha Blend State could not be created.");

        ID3D11BlendState* additiveBlendState;
        ENGINE_HR_MESSAGE(device->CreateBlendState(&additiveBlendDesc, &additiveBlendState), "Additive Blend State could not be created");

        ID3D11BlendState* gbufferBlendState;
        ENGINE_HR_MESSAGE(device->CreateBlendState(&gbufferBlendDesc, &gbufferBlendState), "GBuffer Alpha Blend State could not be created");

        BlendStates[(U64)EBlendStates::Disable] = nullptr;
        BlendStates[(U64)EBlendStates::AlphaBlend] = alphaBlendState;
        BlendStates[(U64)EBlendStates::AdditiveBlend] = additiveBlendState;
        BlendStates[(U64)EBlendStates::GBufferAlphaBlend] = gbufferBlendState;

        return true;
    }

    bool CRenderStateManager::CreateDepthStencilStates(ID3D11Device* device)
    {
        D3D11_DEPTH_STENCIL_DESC onlyreadDepthDesc = { 0 };
        onlyreadDepthDesc.DepthEnable = true;
        onlyreadDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        onlyreadDepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
        onlyreadDepthDesc.StencilEnable = false;

        D3D11_DEPTH_STENCIL_DESC stencilWriteDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
        stencilWriteDesc.DepthEnable = true;
        stencilWriteDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        stencilWriteDesc.DepthFunc = D3D11_COMPARISON_LESS;
        stencilWriteDesc.StencilEnable = TRUE;
        stencilWriteDesc.StencilReadMask = 0xFF;
        stencilWriteDesc.StencilWriteMask = 0xFF;
        stencilWriteDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        stencilWriteDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        stencilWriteDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
        stencilWriteDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        D3D11_DEPTH_STENCIL_DESC stencilMaskDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
        stencilMaskDesc.DepthEnable = FALSE;
        stencilMaskDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        stencilMaskDesc.StencilEnable = TRUE;
        stencilMaskDesc.StencilReadMask = 0xFF;
        stencilMaskDesc.StencilWriteMask = 0xFF;
        stencilMaskDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        stencilMaskDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        stencilMaskDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;

        D3D11_DEPTH_STENCIL_DESC depthFirstDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
        depthFirstDesc.DepthEnable = TRUE;
        depthFirstDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        depthFirstDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        depthFirstDesc.StencilEnable = FALSE;

        ID3D11DepthStencilState* onlyreadDepthStencilState;
        ENGINE_HR_MESSAGE(device->CreateDepthStencilState(&onlyreadDepthDesc, &onlyreadDepthStencilState), "OnlyRead Depth Stencil State could not be created.");

        ID3D11DepthStencilState* writeDepthStencilState;
        ENGINE_HR_MESSAGE(device->CreateDepthStencilState(&stencilWriteDesc, &writeDepthStencilState), "Write Stencil State could not be created.");

        ID3D11DepthStencilState* maskDepthStencilState;
        ENGINE_HR_MESSAGE(device->CreateDepthStencilState(&stencilMaskDesc, &maskDepthStencilState), "Mask Stencil State could not be created.");

        ID3D11DepthStencilState* depthFirstStencilState;
        ENGINE_HR_MESSAGE(device->CreateDepthStencilState(&depthFirstDesc, &depthFirstStencilState), "Depth First Stencil State could not be created.");

        DepthStencilStates[(U64)EDepthStencilStates::Default] = nullptr;
        DepthStencilStates[(U64)EDepthStencilStates::OnlyRead] = onlyreadDepthStencilState;
        DepthStencilStates[(U64)EDepthStencilStates::StencilWrite] = writeDepthStencilState;
        DepthStencilStates[(U64)EDepthStencilStates::StencilMask] = maskDepthStencilState;
        DepthStencilStates[(U64)EDepthStencilStates::DepthFirst] = depthFirstStencilState;

        return true;
    }

    bool CRenderStateManager::CreateRasterizerStates(ID3D11Device* device)
    {
        D3D11_RASTERIZER_DESC wireframeRasterizerDesc = {};
        wireframeRasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
        wireframeRasterizerDesc.CullMode = D3D11_CULL_BACK;
        wireframeRasterizerDesc.DepthClipEnable = true;

        D3D11_RASTERIZER_DESC frontFaceDesc = {};
        frontFaceDesc.FillMode = D3D11_FILL_SOLID;
        frontFaceDesc.CullMode = D3D11_CULL_FRONT;
        frontFaceDesc.DepthClipEnable = true;

        D3D11_RASTERIZER_DESC noCullDesc = {};
        noCullDesc.FillMode = D3D11_FILL_SOLID;
        noCullDesc.CullMode = D3D11_CULL_NONE;
        noCullDesc.DepthClipEnable = true;

        ID3D11RasterizerState* wireframeRasterizerState;
        ENGINE_HR_MESSAGE(device->CreateRasterizerState(&wireframeRasterizerDesc, &wireframeRasterizerState), "Wireframe Rasterizer State could not be created.");

        ID3D11RasterizerState* frontFaceState;
        ENGINE_HR_MESSAGE(device->CreateRasterizerState(&frontFaceDesc, &frontFaceState), "Front face Rasterizer State could not be created.");

        ID3D11RasterizerState* noCullState;
        ENGINE_HR_MESSAGE(device->CreateRasterizerState(&noCullDesc, &noCullState), "No Face culling Rasterizer State could not be created.");

        RasterizerStates[(U64)ERasterizerStates::Default] = nullptr;
        RasterizerStates[(U64)ERasterizerStates::Wireframe] = wireframeRasterizerState;
        RasterizerStates[(U64)ERasterizerStates::FrontFaceCulling] = frontFaceState;
        RasterizerStates[(U64)ERasterizerStates::NoFaceCulling] = noCullState;

        return true;
    }
}
