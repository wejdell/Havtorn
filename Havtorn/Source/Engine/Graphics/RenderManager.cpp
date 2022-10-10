// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RenderManager.h"
#include "GraphicsUtilities.h"
#include "Core/GeneralUtilities.h"
#include "RenderCommand.h"

#include "Engine.h"

#include "ECS/ECSInclude.h"
#include "GraphicsStructs.h"
#include "GeometryPrimitives.h"
#include "FileSystem/FileHeaderDeclarations.h"

#include <algorithm>
#include <future>

#include "FileSystem/FileSystem.h"
#include "Threading/ThreadManager.h"
#include "TextureBank.h"

#include "ModelImporter.h"

#include <DirectXTex/DirectXTex.h>

namespace Havtorn
{
	U32 CRenderManager::NumberOfDrawCallsThisFrame = 0;

	CRenderManager::CRenderManager()
		: Framework(nullptr)
	    , Context(nullptr)
	    , FrameBuffer(nullptr)
	    , ObjectBuffer(nullptr)
		, DecalBuffer(nullptr)
	    , DirectionalLightBuffer(nullptr)
		, PointLightBuffer(nullptr)
		, SpotLightBuffer(nullptr)
		, ShadowmapBuffer(nullptr)
		, VolumetricLightBuffer(nullptr)
	    , PushToCommands(&RenderCommandsA)
	    , PopFromCommands(&RenderCommandsB)
	    , ClearColor(0.5f, 0.5f, 0.5f, 1.0f)
	    , RenderPassIndex(0)
	    , DoFullRender(true)
	    , UseAntiAliasing(true)
	    , UseBrokenScreenPass(false)
	{
	}

	CRenderManager::~CRenderManager()
	{
		Release();
	}

	bool CRenderManager::Init(CGraphicsFramework* framework, CWindowHandler* windowHandler)
	{
		Framework = framework;
		Context = Framework->GetContext();

		D3D11_BUFFER_DESC bufferDescription = { 0 };
		bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
		bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		bufferDescription.ByteWidth = sizeof(SFrameBufferData);
		ENGINE_HR_BOOL_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &FrameBuffer), "Frame Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SObjectBufferData);
		ENGINE_HR_BOOL_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &ObjectBuffer), "Object Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SColorObjectBufferData);
		ENGINE_HR_BOOL_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &ColorObjectBuffer), "Color Object Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SDecalBufferData);
		ENGINE_HR_BOOL_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &DecalBuffer), "Decal Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SDirectionalLightBufferData);
		ENGINE_HR_BOOL_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &DirectionalLightBuffer), "Directional Light Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SPointLightBufferData);
		ENGINE_HR_BOOL_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &PointLightBuffer), "Point Light Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SSpotLightBufferData);
		ENGINE_HR_BOOL_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &SpotLightBuffer), "Spot Light Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SShadowmapBufferData) * 6;
		ENGINE_HR_BOOL_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &ShadowmapBuffer), "Shadowmap Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SVolumetricLightBufferData);
		ENGINE_HR_BOOL_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &VolumetricLightBuffer), "Volumetric Light Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SEmissiveBufferData);
		ENGINE_HR_BOOL_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &EmissiveBuffer), "Emissive Buffer could not be created.");

		//Instance Buffer
		D3D11_BUFFER_DESC instanceBufferDesc;
		instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		instanceBufferDesc.ByteWidth = sizeof(SMatrix) * InstancedMeshNumberLimit;
		instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		instanceBufferDesc.MiscFlags = 0;
		instanceBufferDesc.StructureByteStride = 0;

		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&instanceBufferDesc, nullptr, &InstancedTransformBuffer), "Instanced Transform Buffer could not be created.");

		ENGINE_ERROR_BOOL_MESSAGE(FullscreenRenderer.Init(framework), "Failed to Init Fullscreen Renderer.");
		ENGINE_ERROR_BOOL_MESSAGE(FullscreenTextureFactory.Init(framework), "Failed to Init Fullscreen Texture Factory.");
		ENGINE_ERROR_BOOL_MESSAGE(RenderStateManager.Init(framework), "Failed to Init Render State Manager.");

		ID3D11Texture2D* backbufferTexture = framework->GetBackbufferTexture();
		ENGINE_ERROR_BOOL_MESSAGE(backbufferTexture, "Backbuffer Texture is null.");

		Backbuffer = FullscreenTextureFactory.CreateTexture(backbufferTexture);
		InitRenderTextures(windowHandler);

		// Load default resources
		InitVertexShadersAndInputLayouts();
		InitPixelShaders();
		InitSamplers();
		InitVertexBuffers();
		InitIndexBuffers();
		InitTopologies();
		InitMeshVertexStrides();
		InitMeshVertexOffset();

		InitEditorResources();
		LoadDemoSceneResources();

		return true;
	}

	bool CRenderManager::ReInit(CGraphicsFramework* framework, CWindowHandler* windowHandler)
	{
		ENGINE_ERROR_BOOL_MESSAGE(RenderStateManager.Init(framework), "Failed to Init Render State Manager.");

		ID3D11Texture2D* backbufferTexture = framework->GetBackbufferTexture();
		ENGINE_ERROR_BOOL_MESSAGE(backbufferTexture, "Backbuffer Texture is null.");

		Backbuffer = FullscreenTextureFactory.CreateTexture(backbufferTexture);
		InitRenderTextures(windowHandler);

		return true;
	}

	void CRenderManager::InitRenderTextures(CWindowHandler* windowHandler)
	{
		RenderedScene = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT);
		IntermediateDepth = FullscreenTextureFactory.CreateDepth(windowHandler->GetResolution(), DXGI_FORMAT_R24G8_TYPELESS);

		ShadowAtlasResolution = {8192.0f, 8192.0f};
		InitShadowmapAtlas(ShadowAtlasResolution);

		DepthCopy = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution(), DXGI_FORMAT_R32_FLOAT);
		DownsampledDepth = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution() / 2.0f, DXGI_FORMAT_R32_FLOAT);

		IntermediateTexture = FullscreenTextureFactory.CreateTexture(ShadowAtlasResolution, DXGI_FORMAT_R16G16B16A16_FLOAT);

		HalfSizeTexture = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution() / 2.0f, DXGI_FORMAT_R16G16B16A16_FLOAT);
		QuarterSizeTexture = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution() / 4.0f, DXGI_FORMAT_R16G16B16A16_FLOAT);
		BlurTexture1 = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT);
		BlurTexture2 = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT);
		VignetteTexture = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT);

		VolumetricAccumulationBuffer = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution() / 2.0f, DXGI_FORMAT_R16G16B16A16_FLOAT);
		VolumetricBlurTexture = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution() / 2.0f, DXGI_FORMAT_R16G16B16A16_FLOAT);

		SSAOBuffer = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution() / 2.0f, DXGI_FORMAT_R16G16B16A16_FLOAT);
		SSAOBlurTexture = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution() / 2.0f, DXGI_FORMAT_R16G16B16A16_FLOAT);

		TonemappedTexture = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT);
		AntiAliasedTexture = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT);
		GBuffer = FullscreenTextureFactory.CreateGBuffer(windowHandler->GetResolution());
	}

	void CRenderManager::InitShadowmapAtlas(SVector2<F32> atlasResolution)
	{
		ShadowAtlasDepth = FullscreenTextureFactory.CreateDepth(atlasResolution, DXGI_FORMAT_R32_TYPELESS);

		// LOD 1
		U16 mapsInLod = 8;
		SVector2<F32> topLeftCoordinate = SVector2<F32>::Zero;
		SVector2<F32> widthAndHeight = atlasResolution * (1.0f / (mapsInLod / 2));
		const SVector2<F32> depth = { 0.0f, 1.0f };
		InitShadowmapLOD(topLeftCoordinate, widthAndHeight, depth, atlasResolution, mapsInLod, 0);

		// LOD 2
		mapsInLod = 16;
		topLeftCoordinate = { 0.0f, atlasResolution.Y * 0.5f };
		widthAndHeight = atlasResolution * (1.0f / (mapsInLod / 2));
		InitShadowmapLOD(topLeftCoordinate, widthAndHeight, depth, atlasResolution, mapsInLod, 8);

		// LOD 3
		mapsInLod = 32;
		topLeftCoordinate = {0.0f, atlasResolution.Y * 0.75f };
		widthAndHeight = atlasResolution * (1.0f / (mapsInLod / 2));
		InitShadowmapLOD(topLeftCoordinate, widthAndHeight, depth, atlasResolution, mapsInLod, 24);

		// LOD 4
		mapsInLod = 128;
		topLeftCoordinate = { 0.0f, atlasResolution.Y * 0.875f };
		widthAndHeight = atlasResolution * (1.0f / (mapsInLod / 4));
		InitShadowmapLOD(topLeftCoordinate, widthAndHeight, depth, atlasResolution, mapsInLod, 56);
	}

	void CRenderManager::InitShadowmapLOD(SVector2<F32> topLeftCoordinate, const SVector2<F32>& widthAndHeight, const SVector2<F32>& depth, const SVector2<F32>& atlasResolution, U16 mapsInLod, U16 startIndex)
	{
		const float startingYCoordinate = topLeftCoordinate.Y;
		const U16 mapsPerRow = static_cast<U16>(atlasResolution.X / widthAndHeight.X);
		for (U16 i = startIndex; i < startIndex + mapsInLod; i++)
		{
			const U16 relativeIndex = i - startIndex;
			topLeftCoordinate.X = static_cast<U16>(relativeIndex % mapsPerRow) * widthAndHeight.X;
			topLeftCoordinate.Y = startingYCoordinate + static_cast<U16>(relativeIndex / mapsPerRow) * widthAndHeight.Y;
			AddViewport(topLeftCoordinate, widthAndHeight, depth);
		}
	}

	void CRenderManager::InitVertexShadersAndInputLayouts()
	{
		AddShader("Shaders/FullscreenVertexShader_VS.cso", EShaderType::Vertex);
		
		std::string vsData = AddShader("Shaders/DeferredStaticMesh_VS.cso", EShaderType::Vertex);
		AddInputLayout(vsData, EInputLayoutType::Pos3Nor3Tan3Bit3UV2);
		
		vsData = AddShader("Shaders/DeferredInstancedMesh_VS.cso", EShaderType::Vertex);
		AddInputLayout(vsData, EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans);
		
		AddShader("Shaders/Decal_VS.cso", EShaderType::Vertex);

		vsData = AddShader("Shaders/PointLight_VS.cso", EShaderType::Vertex);
		AddInputLayout(vsData, EInputLayoutType::Pos4);

		AddShader("Shaders/EditorPreview_VS.cso", EShaderType::Vertex);
		
		AddShader("Shaders/Line_VS.cso", EShaderType::Vertex);
	}

	void CRenderManager::InitPixelShaders()
	{
		AddShader("Shaders/GBuffer_PS.cso", EShaderType::Pixel);

		AddShader("Shaders/Decal_Albedo_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/Decal_Material_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/Decal_Normal_PS.cso", EShaderType::Pixel);
		
		AddShader("Shaders/DeferredLightDirectionalAndEnvironment_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/DeferredLightPoint_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/DeferredLightSpot_PS.cso", EShaderType::Pixel);
		
		AddShader("Shaders/DeferredLightDirectionalVolumetric_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/DeferredLightPointVolumetric_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/DeferredLightSpotVolumetric_PS.cso", EShaderType::Pixel);
		
		AddShader("Shaders/EditorPreview_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/Line_PS.cso", EShaderType::Pixel);
	}

	void CRenderManager::InitSamplers()
	{
		AddSampler(ESamplerType::Wrap);
		AddSampler(ESamplerType::Border);
	}

	void CRenderManager::InitVertexBuffers()
	{
		AddVertexBuffer<SStaticMeshVertex>(GeometryPrimitives::DecalProjector);
		AddVertexBuffer<SPositionVertex>(GeometryPrimitives::PointLightCube);
		AddVertexBuffer<SPositionVertex>(GeometryPrimitives::Line);
	}

	void CRenderManager::InitIndexBuffers()
	{
		AddIndexBuffer(GeometryPrimitives::DecalProjectorIndices);
		AddIndexBuffer(GeometryPrimitives::PointLightCubeIndices);
	}

	void CRenderManager::InitTopologies()
	{
		AddTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		AddTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}

	void CRenderManager::InitMeshVertexStrides()
	{
		AddMeshVertexStride(sizeof(SStaticMeshVertex));
		AddMeshVertexStride(sizeof(SPositionVertex));
	}

	void CRenderManager::InitMeshVertexOffset()
	{
		AddMeshVertexOffset(0);
	}

	void CRenderManager::InitEditorResources()
	{}

	void CRenderManager::LoadDemoSceneResources()
	{
	}

	void CRenderManager::Render()
	{
		while(CThreadManager::RunRenderThread)
		{
			std::unique_lock<std::mutex> uniqueLock(CThreadManager::RenderMutex);
			CThreadManager::RenderCondition.wait(uniqueLock, [] 
				{ return CThreadManager::RenderThreadStatus == ERenderThreadStatus::ReadyToRender; });

			ShouldBlurVolumetricBuffer = false;
			CRenderManager::NumberOfDrawCallsThisFrame = 0;
			RenderStateManager.SetAllDefault();

			Backbuffer.ClearTexture();
			ShadowAtlasDepth.ClearDepth();
			SSAOBuffer.ClearTexture();

			RenderedScene.ClearTexture();
			IntermediateTexture.ClearTexture();
			IntermediateDepth.ClearDepth();
			VolumetricAccumulationBuffer.ClearTexture();
			GBuffer.ClearTextures(ClearColor);

			ShadowAtlasDepth.SetAsDepthTarget(&IntermediateTexture);

			const U16 commandsInHeap = static_cast<U16>(PopFromCommands->size());
			for (U16 i = 0; i < commandsInHeap; ++i)
			{
				SRenderCommand currentCommand = PopFromCommands->top();
				switch (currentCommand.Type)
				{
				case ERenderCommandType::ShadowAtlasPrePassDirectional:
				{
					const auto transformComp = currentCommand.GetComponent(TransformComponent);
					const auto staticMeshComp = currentCommand.GetComponent(StaticMeshComponent);
					const auto directionalLightComp = currentCommand.GetComponent(DirectionalLightComponent);

					FrameBufferData.ToCameraFromWorld = directionalLightComp->ShadowmapView.ShadowViewMatrix;
					FrameBufferData.ToWorldFromCamera = directionalLightComp->ShadowmapView.ShadowViewMatrix.FastInverse();
					FrameBufferData.ToProjectionFromCamera = directionalLightComp->ShadowmapView.ShadowProjectionMatrix;
					FrameBufferData.ToCameraFromProjection = directionalLightComp->ShadowmapView.ShadowProjectionMatrix.Inverse();
					FrameBufferData.CameraPosition = directionalLightComp->ShadowmapView.ShadowPosition;
					BindBuffer(FrameBuffer, FrameBufferData, "Frame Buffer");
					Context->RSSetViewports(1, &Viewports[directionalLightComp->ShadowmapView.ShadowmapViewportIndex]);

					// =============

					ObjectBufferData.ToWorldFromObject = transformComp->Transform.GetMatrix();
					BindBuffer(ObjectBuffer, ObjectBufferData, "Object Buffer");

					const std::vector<SMatrix>& matrices = RendererStaticMeshInstanceTransforms[staticMeshComp->Name];
					BindBuffer(InstancedTransformBuffer, matrices, "Instanced Transform Buffer");

					Context->VSSetConstantBuffers(1, 1, &ObjectBuffer);
					Context->IASetPrimitiveTopology(Topologies[staticMeshComp->TopologyIndex]);
					Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans)]);

					Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::StaticMeshInstanced)], nullptr, 0);
					Context->PSSetShader(nullptr, nullptr, 0);

					for (U8 drawCallIndex = 0; drawCallIndex < static_cast<U8>(staticMeshComp->DrawCallData.size()); drawCallIndex++)
					{
						const SDrawCallData& drawData = staticMeshComp->DrawCallData[drawCallIndex];
						ID3D11Buffer* bufferPointers[2] = { VertexBuffers[drawData.VertexBufferIndex], InstancedTransformBuffer };
						const U32 strides[2] = { MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SMatrix) };
						const U32 offsets[2] = { MeshVertexOffsets[drawData.VertexOffsetIndex], 0 };
						Context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
						Context->IASetIndexBuffer(IndexBuffers[drawData.IndexBufferIndex], DXGI_FORMAT_R32_UINT, 0);
						Context->DrawIndexedInstanced(drawData.IndexCount, static_cast<U32>(matrices.size()), 0, 0, 0);
						CRenderManager::NumberOfDrawCallsThisFrame++;
					}

				}
				break;

				case ERenderCommandType::ShadowAtlasPrePassPoint:
				{
					const auto transformComp = currentCommand.GetComponent(TransformComponent);
					const auto staticMeshComp = currentCommand.GetComponent(StaticMeshComponent);
					const auto pointLightComp = currentCommand.GetComponent(PointLightComponent);

					// TODO.NR: Not needed for instanced rendering?
					ObjectBufferData.ToWorldFromObject = transformComp->Transform.GetMatrix();
					BindBuffer(ObjectBuffer, ObjectBufferData, "Object Buffer");

					const std::vector<SMatrix>& matrices = RendererStaticMeshInstanceTransforms[staticMeshComp->Name];
					BindBuffer(InstancedTransformBuffer, matrices, "Instanced Transform Buffer");

					Context->VSSetConstantBuffers(1, 1, &ObjectBuffer);
					Context->IASetPrimitiveTopology(Topologies[staticMeshComp->TopologyIndex]);
					Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans)]);

					Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::StaticMeshInstanced)], nullptr, 0);
					Context->PSSetShader(nullptr, nullptr, 0);

					for (const auto& shadowmapView : pointLightComp->ShadowmapViews)
					{
						FrameBufferData.ToCameraFromWorld = shadowmapView.ShadowViewMatrix;
						FrameBufferData.ToWorldFromCamera = shadowmapView.ShadowViewMatrix.FastInverse();
						FrameBufferData.ToProjectionFromCamera = shadowmapView.ShadowProjectionMatrix;
						FrameBufferData.ToCameraFromProjection = shadowmapView.ShadowProjectionMatrix.Inverse();
						FrameBufferData.CameraPosition = shadowmapView.ShadowPosition;
						BindBuffer(FrameBuffer, FrameBufferData, "Frame Buffer");

						Context->VSSetConstantBuffers(0, 1, &FrameBuffer);

						Context->RSSetViewports(1, &Viewports[shadowmapView.ShadowmapViewportIndex]);

						// =============

						for (U8 drawCallIndex = 0; drawCallIndex < static_cast<U8>(staticMeshComp->DrawCallData.size()); drawCallIndex++)
						{
							const SDrawCallData& drawData = staticMeshComp->DrawCallData[drawCallIndex];
							ID3D11Buffer* bufferPointers[2] = { VertexBuffers[drawData.VertexBufferIndex], InstancedTransformBuffer };
							const U32 strides[2] = { MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SMatrix) };
							const U32 offsets[2] = { MeshVertexOffsets[drawData.VertexOffsetIndex], 0 };
							Context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
							Context->IASetIndexBuffer(IndexBuffers[drawData.IndexBufferIndex], DXGI_FORMAT_R32_UINT, 0);
							Context->DrawIndexedInstanced(drawData.IndexCount, static_cast<U32>(matrices.size()), 0, 0, 0);
							CRenderManager::NumberOfDrawCallsThisFrame++;
						}
					}
				}
				break;

				case ERenderCommandType::ShadowAtlasPrePassSpot:
				{
					const auto transformComp = currentCommand.GetComponent(TransformComponent);
					const auto staticMeshComp = currentCommand.GetComponent(StaticMeshComponent);
					const auto spotLightComp = currentCommand.GetComponent(SpotLightComponent);

					FrameBufferData.ToCameraFromWorld = spotLightComp->ShadowmapView.ShadowViewMatrix;
					FrameBufferData.ToWorldFromCamera = spotLightComp->ShadowmapView.ShadowViewMatrix.FastInverse();
					FrameBufferData.ToProjectionFromCamera = spotLightComp->ShadowmapView.ShadowProjectionMatrix;
					FrameBufferData.ToCameraFromProjection = spotLightComp->ShadowmapView.ShadowProjectionMatrix.Inverse();
					FrameBufferData.CameraPosition = spotLightComp->ShadowmapView.ShadowPosition;
					BindBuffer(FrameBuffer, FrameBufferData, "Frame Buffer");

					ObjectBufferData.ToWorldFromObject = transformComp->Transform.GetMatrix();
					BindBuffer(ObjectBuffer, ObjectBufferData, "Object Buffer");

					Context->VSSetConstantBuffers(0, 1, &FrameBuffer);
					Context->RSSetViewports(1, &Viewports[spotLightComp->ShadowmapView.ShadowmapViewportIndex]);
				
					// =============
					
					const std::vector<SMatrix>& matrices = RendererStaticMeshInstanceTransforms[staticMeshComp->Name];
					BindBuffer(InstancedTransformBuffer, matrices, "Instanced Transform Buffer");

					Context->VSSetConstantBuffers(1, 1, &ObjectBuffer);
					Context->IASetPrimitiveTopology(Topologies[staticMeshComp->TopologyIndex]);
					Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans)]);

					Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::StaticMeshInstanced)], nullptr, 0);
					Context->PSSetShader(nullptr, nullptr, 0);

					for (U8 drawCallIndex = 0; drawCallIndex < static_cast<U8>(staticMeshComp->DrawCallData.size()); drawCallIndex++)
					{
						const SDrawCallData& drawData = staticMeshComp->DrawCallData[drawCallIndex];
						ID3D11Buffer* bufferPointers[2] = { VertexBuffers[drawData.VertexBufferIndex], InstancedTransformBuffer };
						const U32 strides[2] = { MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SMatrix) };
						const U32 offsets[2] = { MeshVertexOffsets[drawData.VertexOffsetIndex], 0 };
						Context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
						Context->IASetIndexBuffer(IndexBuffers[drawData.IndexBufferIndex], DXGI_FORMAT_R32_UINT, 0);
						Context->DrawIndexedInstanced(drawData.IndexCount, static_cast<U32>(matrices.size()), 0, 0, 0);
						CRenderManager::NumberOfDrawCallsThisFrame++;
					}
				}
				break;

				case ERenderCommandType::CameraDataStorage:
				{
					GBuffer.SetAsActiveTarget(&IntermediateDepth);

					const auto transformComp = currentCommand.GetComponent(TransformComponent);
					const auto cameraComp = currentCommand.GetComponent(CameraComponent);

					FrameBufferData.ToCameraFromWorld = transformComp->Transform.GetMatrix().FastInverse();
					FrameBufferData.ToWorldFromCamera = transformComp->Transform.GetMatrix();
					FrameBufferData.ToProjectionFromCamera = cameraComp->ProjectionMatrix;
					FrameBufferData.ToCameraFromProjection = cameraComp->ProjectionMatrix.Inverse();
					FrameBufferData.CameraPosition = transformComp->Transform.GetMatrix().GetTranslation4();
					BindBuffer(FrameBuffer, FrameBufferData, "Frame Buffer");

					Context->VSSetConstantBuffers(0, 1, &FrameBuffer);
					Context->PSSetConstantBuffers(0, 1, &FrameBuffer);
				}
				break;

				case ERenderCommandType::GBufferData:
				{
					const auto transformComp = currentCommand.GetComponent(TransformComponent);
					const auto staticMeshComp = currentCommand.GetComponent(StaticMeshComponent);
					const auto materialComp = currentCommand.GetComponent(MaterialComponent);

					ObjectBufferData.ToWorldFromObject = transformComp->Transform.GetMatrix();
					BindBuffer(ObjectBuffer, ObjectBufferData, "Object Buffer");

					Context->VSSetConstantBuffers(1, 1, &ObjectBuffer);
					Context->IASetPrimitiveTopology(Topologies[staticMeshComp->TopologyIndex]);
					Context->IASetInputLayout(InputLayouts[staticMeshComp->InputLayoutIndex]);

					Context->VSSetShader(VertexShaders[staticMeshComp->VertexShaderIndex], nullptr, 0);
					Context->PSSetShader(PixelShaders[staticMeshComp->PixelShaderIndex], nullptr, 0);

					ID3D11SamplerState* sampler = Samplers[staticMeshComp->SamplerIndex];
					Context->PSSetSamplers(0, 1, &sampler);

					auto textureBank = GEngine::GetTextureBank();
					for (U8 drawCallIndex = 0; drawCallIndex < static_cast<U8>(staticMeshComp->DrawCallData.size()); drawCallIndex++)
					{
						// Load Textures
						std::vector<ID3D11ShaderResourceView*> resourceViewPointers;
						resourceViewPointers.resize(TexturesPerMaterial);
						for (U8 textureIndex = 0, pointerTracker = 0; textureIndex < TexturesPerMaterial; textureIndex++, pointerTracker++)
						{
							U8 materialIndex = UMath::Min(drawCallIndex, static_cast<U8>(staticMeshComp->NumberOfMaterials - 1));
							resourceViewPointers[pointerTracker] = textureBank->GetTexture(materialComp->MaterialReferences[textureIndex + materialIndex * TexturesPerMaterial]);
						}
						Context->PSSetShaderResources(5, TexturesPerMaterial, resourceViewPointers.data());

						const SDrawCallData& drawData = staticMeshComp->DrawCallData[drawCallIndex];
						ID3D11Buffer* vertexBuffer = VertexBuffers[drawData.VertexBufferIndex];
						Context->IASetVertexBuffers(0, 1, &vertexBuffer, &MeshVertexStrides[drawData.VertexStrideIndex], &MeshVertexOffsets[drawData.VertexOffsetIndex]);
						Context->IASetIndexBuffer(IndexBuffers[drawData.IndexBufferIndex], DXGI_FORMAT_R32_UINT, 0);
						Context->DrawIndexed(drawData.IndexCount, 0, 0);
						CRenderManager::NumberOfDrawCallsThisFrame++;
					}
				}
				break;

				case ERenderCommandType::GBufferDataInstanced:
				{
					const auto transformComp = currentCommand.GetComponent(TransformComponent);
					const auto staticMeshComp = currentCommand.GetComponent(StaticMeshComponent);
					const auto materialComp = currentCommand.GetComponent(MaterialComponent);

					ObjectBufferData.ToWorldFromObject = transformComp->Transform.GetMatrix();
					BindBuffer(ObjectBuffer, ObjectBufferData, "Object Buffer");
					
					const std::vector<SMatrix>& matrices = RendererStaticMeshInstanceTransforms[staticMeshComp->Name];
					BindBuffer(InstancedTransformBuffer, matrices, "Instanced Transform Buffer");

					Context->VSSetConstantBuffers(1, 1, &ObjectBuffer);
					Context->IASetPrimitiveTopology(Topologies[staticMeshComp->TopologyIndex]);
					Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans)]);

					Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::StaticMeshInstanced)], nullptr, 0);
					Context->PSSetShader(PixelShaders[staticMeshComp->PixelShaderIndex], nullptr, 0);

					ID3D11SamplerState* sampler = Samplers[staticMeshComp->SamplerIndex];
					Context->PSSetSamplers(0, 1, &sampler);

					auto textureBank = GEngine::GetTextureBank();
					for (U8 drawCallIndex = 0; drawCallIndex < static_cast<U8>(staticMeshComp->DrawCallData.size()); drawCallIndex++)
					{
						// Load Textures
						std::vector<ID3D11ShaderResourceView*> resourceViewPointers;
						resourceViewPointers.resize(TexturesPerMaterial);
						for (U8 textureIndex = 0, pointerTracker = 0; textureIndex < TexturesPerMaterial; textureIndex++, pointerTracker++)
						{
							U8 materialIndex = UMath::Min(drawCallIndex, static_cast<U8>(staticMeshComp->NumberOfMaterials - 1));
							resourceViewPointers[pointerTracker] = textureBank->GetTexture(materialComp->MaterialReferences[textureIndex + materialIndex * TexturesPerMaterial]);
						}
						Context->PSSetShaderResources(5, TexturesPerMaterial, resourceViewPointers.data());

						const SDrawCallData& drawData = staticMeshComp->DrawCallData[drawCallIndex];
						ID3D11Buffer* bufferPointers[2] = { VertexBuffers[drawData.VertexBufferIndex], InstancedTransformBuffer };
						const U32 strides[2] = { MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SMatrix) };
						const U32 offsets[2] = { MeshVertexOffsets[drawData.VertexOffsetIndex], 0 };
						Context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
						Context->IASetIndexBuffer(IndexBuffers[drawData.IndexBufferIndex], DXGI_FORMAT_R32_UINT, 0);
						Context->DrawIndexedInstanced(drawData.IndexCount, static_cast<U32>(matrices.size()), 0, 0, 0);
						CRenderManager::NumberOfDrawCallsThisFrame++;
					}
				}
				break;

				case ERenderCommandType::DecalDepthCopy:
				{
					DepthCopy.SetAsActiveTarget();
					IntermediateDepth.SetAsResourceOnSlot(0);
					FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::CopyDepth);
				}
				break;

				case ERenderCommandType::DeferredDecal:
				{
					RenderStateManager.SetDepthStencilState(CRenderStateManager::EDepthStencilStates::OnlyRead);
					RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AlphaBlend);
					GBuffer.SetAsActiveTarget(&IntermediateDepth);
					DepthCopy.SetAsResourceOnSlot(21);

					const auto transformComp = currentCommand.GetComponent(TransformComponent);
					const auto decalComp = currentCommand.GetComponent(DecalComponent);

					DecalBufferData.ToWorld = transformComp->Transform.GetMatrix();
					DecalBufferData.ToObjectSpace = transformComp->Transform.GetMatrix().Inverse();

					BindBuffer(DecalBuffer, DecalBufferData, "Decal Buffer");

					Context->VSSetConstantBuffers(1, 1, &DecalBuffer);
					Context->PSSetConstantBuffers(1, 1, &DecalBuffer);

					Context->IASetPrimitiveTopology(Topologies[0]);
					Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos3Nor3Tan3Bit3UV2)]);
					Context->IASetVertexBuffers(0, 1, &VertexBuffers[0], &MeshVertexStrides[0], &MeshVertexOffsets[0]);
					Context->IASetIndexBuffer(IndexBuffers[0], DXGI_FORMAT_R32_UINT, 0);

					Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::Decal)], nullptr, 0);

					auto sampler = Samplers[static_cast<U8>(ESamplers::DefaultWrap)];
					Context->PSSetSamplers(0, 1, &sampler);

					auto textureBank = GEngine::GetTextureBank();
					if (decalComp->ShouldRenderAlbedo)
					{
						auto shaderResource = textureBank->GetTexture(decalComp->TextureReferences[0]);
						Context->PSSetShaderResources(5, 1, &shaderResource);
						Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::DecalAlbedo)], nullptr, 0);
						Context->DrawIndexed(36, 0, 0);
						CRenderManager::NumberOfDrawCallsThisFrame++;
					}

					if (decalComp->ShouldRenderMaterial)
					{
						auto shaderResource = textureBank->GetTexture(decalComp->TextureReferences[1]);
						Context->PSSetShaderResources(6, 1, &shaderResource);
						Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::DecalMaterial)], nullptr, 0);
						Context->DrawIndexed(36, 0, 0);
						CRenderManager::NumberOfDrawCallsThisFrame++;
					}

					if (decalComp->ShouldRenderNormal)
					{
						auto shaderResource = textureBank->GetTexture(decalComp->TextureReferences[2]);
						Context->PSSetShaderResources(7, 1, &shaderResource);
						Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::DecalNormal)], nullptr, 0);
						Context->DrawIndexed(36, 0, 0);
						CRenderManager::NumberOfDrawCallsThisFrame++;
					}
				}
				break;

				case ERenderCommandType::PreLightingPass:
				{
					// === SSAO ===
					SSAOBuffer.SetAsActiveTarget();
					GBuffer.SetAsResourceOnSlot(CGBuffer::EGBufferTextures::Normal, 2);
					IntermediateDepth.SetAsResourceOnSlot(21);
					FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::SSAO);
					RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::Disable);

					SSAOBlurTexture.SetAsActiveTarget();
					SSAOBuffer.SetAsResourceOnSlot(0);
					FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::SSAOBlur);
					// === !SSAO ===

					Context->PSSetSamplers(0, 1, &Samplers[static_cast<U8>(ESamplers::DefaultWrap)]);
					Context->PSSetSamplers(1, 1, &Samplers[static_cast<U8>(ESamplers::DefaultBorder)]);

					Context->GSSetShader(nullptr, nullptr, 0);
				}
				break;

				case ERenderCommandType::DeferredLightingDirectional:
				{
					RenderedScene.SetAsActiveTarget();
					GBuffer.SetAllAsResources(1);
					IntermediateDepth.SetAsResourceOnSlot(21);
					RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);

					// add Alpha blend PS shader

					ShadowAtlasDepth.SetAsResourceOnSlot(22);
					SSAOBlurTexture.SetAsResourceOnSlot(23);

					const auto environmentLightComp = currentCommand.GetComponent(EnvironmentLightComponent);
					const auto directionalLightComp = currentCommand.GetComponent(DirectionalLightComponent);

					auto cubemapTexture = GEngine::GetTextureBank()->GetTexture(environmentLightComp->AmbientCubemapReference);
					Context->PSSetShaderResources(0, 1, &cubemapTexture);

					// Update lightbufferdata and fill lightbuffer
					DirectionalLightBufferData.DirectionalLightDirection = directionalLightComp->Direction;
					DirectionalLightBufferData.DirectionalLightColor = directionalLightComp->Color;
					BindBuffer(DirectionalLightBuffer, DirectionalLightBufferData, "Light Buffer");
					Context->PSSetConstantBuffers(2, 1, &DirectionalLightBuffer);

					ShadowmapBufferData.ToShadowmapView = directionalLightComp->ShadowmapView.ShadowViewMatrix;
					ShadowmapBufferData.ToShadowmapProjection = directionalLightComp->ShadowmapView.ShadowProjectionMatrix;
					ShadowmapBufferData.ShadowmapPosition = directionalLightComp->ShadowmapView.ShadowPosition;

					const auto& viewport = Viewports[directionalLightComp->ShadowmapView.ShadowmapViewportIndex];
					ShadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
					ShadowmapBufferData.ShadowAtlasResolution = ShadowAtlasResolution;
					ShadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
					ShadowmapBufferData.ShadowTestTolerance = 0.001f;
					
					BindBuffer(ShadowmapBuffer, ShadowmapBufferData, "Shadowmap Buffer");
					Context->PSSetConstantBuffers(5, 1, &ShadowmapBuffer);

					// Emissive Post Processing 
					EmissiveBufferData.EmissiveStrength = FullscreenRenderer.PostProcessingBufferData.EmissiveStrength;
					BindBuffer(EmissiveBuffer, EmissiveBufferData, "Emissive Buffer");
					Context->PSSetConstantBuffers(7, 1, &EmissiveBuffer);

					Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					Context->IASetInputLayout(nullptr);
					Context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
					Context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

					Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::Fullscreen)], nullptr, 0);
					Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::DeferredDirectional)], nullptr, 0);

					Context->Draw(3, 0);
					CRenderManager::NumberOfDrawCallsThisFrame++;
				}
				break;

				case ERenderCommandType::DeferredLightingPoint: 
				{
					RenderedScene.SetAsActiveTarget();
					GBuffer.SetAllAsResources(1);
					IntermediateDepth.SetAsResourceOnSlot(21);
					RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);

					ShadowAtlasDepth.SetAsResourceOnSlot(22);
					RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::FrontFaceCulling);

					// Update lightbufferdata and fill lightbuffer
					const auto pointLightComp = currentCommand.GetComponent(PointLightComponent);
					const auto transformComponent = currentCommand.GetComponent(TransformComponent);
					SVector position = transformComponent->Transform.GetMatrix().GetTranslation();
					PointLightBufferData.ToWorldFromObject = transformComponent->Transform.GetMatrix();
					PointLightBufferData.ColorAndIntensity = pointLightComp->ColorAndIntensity;
					PointLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, pointLightComp->Range };
					BindBuffer(PointLightBuffer, PointLightBufferData, "Point Light Buffer");
					Context->VSSetConstantBuffers(3, 1, &PointLightBuffer);
					Context->PSSetConstantBuffers(3, 1, &PointLightBuffer);

					SShadowmapBufferData shadowmapBufferData[6];
					for (U8 shadowmapViewIndex = 0; shadowmapViewIndex < 6; shadowmapViewIndex++)
					{
						shadowmapBufferData[shadowmapViewIndex].ToShadowmapView = pointLightComp->ShadowmapViews[shadowmapViewIndex].ShadowViewMatrix;
						shadowmapBufferData[shadowmapViewIndex].ToShadowmapProjection = pointLightComp->ShadowmapViews[shadowmapViewIndex].ShadowProjectionMatrix;
						shadowmapBufferData[shadowmapViewIndex].ShadowmapPosition = pointLightComp->ShadowmapViews[shadowmapViewIndex].ShadowPosition;

						const auto& viewport = Viewports[pointLightComp->ShadowmapViews[shadowmapViewIndex].ShadowmapViewportIndex];
						shadowmapBufferData[shadowmapViewIndex].ShadowmapResolution = { viewport.Width, viewport.Height };
						shadowmapBufferData[shadowmapViewIndex].ShadowAtlasResolution = ShadowAtlasResolution;
						shadowmapBufferData[shadowmapViewIndex].ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
						shadowmapBufferData[shadowmapViewIndex].ShadowTestTolerance = 0.00001f;
					}

					BindBuffer(ShadowmapBuffer, shadowmapBufferData, "Shadowmap Buffer");
					Context->PSSetConstantBuffers(5, 1, &ShadowmapBuffer);

					Context->IASetPrimitiveTopology(Topologies[static_cast<U8>(ETopologies::TriangleList)]);
					Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos4)]);
					Context->IASetVertexBuffers(0, 1, &VertexBuffers[1], &MeshVertexStrides[1], &MeshVertexOffsets[0]);
					Context->IASetIndexBuffer(IndexBuffers[1], DXGI_FORMAT_R32_UINT, 0);

					Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::PointAndSpotLight)], nullptr, 0);
					Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::DeferredPoint)], nullptr, 0);

					Context->DrawIndexed(36, 0, 0);
					CRenderManager::NumberOfDrawCallsThisFrame++;
					RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::Default);
				}
				break;

				case ERenderCommandType::DeferredLightingSpot:
				{
					RenderedScene.SetAsActiveTarget();
					GBuffer.SetAllAsResources(1);
					IntermediateDepth.SetAsResourceOnSlot(21);
					RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);

					ShadowAtlasDepth.SetAsResourceOnSlot(22);
					RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::FrontFaceCulling);

					// Update lightbufferdata and fill lightbuffer
					const auto spotLightComp = currentCommand.GetComponent(SpotLightComponent);
					const auto transformComponent = currentCommand.GetComponent(TransformComponent);
					SVector position = transformComponent->Transform.GetMatrix().GetTranslation();

					PointLightBufferData.ToWorldFromObject = transformComponent->Transform.GetMatrix();
					PointLightBufferData.ColorAndIntensity = spotLightComp->ColorAndIntensity;
					PointLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, spotLightComp->Range };
					
					BindBuffer(PointLightBuffer, PointLightBufferData, "Spotlight Vertex Shader Buffer");
					Context->VSSetConstantBuffers(3, 1, &PointLightBuffer);
					
					SpotLightBufferData.ColorAndIntensity = spotLightComp->ColorAndIntensity;
					SpotLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, spotLightComp->Range };
					SpotLightBufferData.Direction = spotLightComp->Direction;
					SpotLightBufferData.DirectionNormal1 = spotLightComp->DirectionNormal1;
					SpotLightBufferData.DirectionNormal2 = spotLightComp->DirectionNormal2;
					SpotLightBufferData.OuterAngle = spotLightComp->OuterAngle;
					SpotLightBufferData.InnerAngle = spotLightComp->InnerAngle;
					
					BindBuffer(SpotLightBuffer, SpotLightBufferData, "Spotlight Pixel Shader Buffer");
					Context->PSSetConstantBuffers(3, 1, &SpotLightBuffer);

					SShadowmapBufferData shadowmapBufferData;
					shadowmapBufferData.ToShadowmapView = spotLightComp->ShadowmapView.ShadowViewMatrix;
					shadowmapBufferData.ToShadowmapProjection = spotLightComp->ShadowmapView.ShadowProjectionMatrix;
					shadowmapBufferData.ShadowmapPosition = spotLightComp->ShadowmapView.ShadowPosition;

					const auto& viewport = Viewports[spotLightComp->ShadowmapView.ShadowmapViewportIndex];
					shadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
					shadowmapBufferData.ShadowAtlasResolution = ShadowAtlasResolution;
					shadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
					shadowmapBufferData.ShadowTestTolerance = 0.00001f;

					BindBuffer(ShadowmapBuffer, shadowmapBufferData, "Shadowmap Buffer");
					Context->PSSetConstantBuffers(5, 1, &ShadowmapBuffer);

					Context->IASetPrimitiveTopology(Topologies[static_cast<U8>(ETopologies::TriangleList)]);
					Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos4)]);
					Context->IASetVertexBuffers(0, 1, &VertexBuffers[1], &MeshVertexStrides[1], &MeshVertexOffsets[0]);
					Context->IASetIndexBuffer(IndexBuffers[1], DXGI_FORMAT_R32_UINT, 0);

					// Use Point Light Vertex Shader
					Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::PointAndSpotLight)], nullptr, 0);
					Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::DeferredSpot)], nullptr, 0);

					Context->DrawIndexed(36, 0, 0);
					CRenderManager::NumberOfDrawCallsThisFrame++;
					RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::Default);
				}
				break;

				case ERenderCommandType::VolumetricLightingDirectional:
				{					
					VolumetricAccumulationBuffer.SetAsActiveTarget();
					RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
					RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::Default);
					IntermediateDepth.SetAsResourceOnSlot(21);
					ShadowAtlasDepth.SetAsResourceOnSlot(22);
				
					const auto directionalLightComp = currentCommand.GetComponent(DirectionalLightComponent);
					const auto volumetricLightComp = currentCommand.GetComponent(VolumetricLightComponent);

					// Lightbuffer
					DirectionalLightBufferData.DirectionalLightDirection = directionalLightComp->Direction;
					DirectionalLightBufferData.DirectionalLightColor = directionalLightComp->Color;
					BindBuffer(DirectionalLightBuffer, DirectionalLightBufferData, "Light Buffer");
					Context->PSSetConstantBuffers(1, 1, &DirectionalLightBuffer);

					// Volumetric buffer
					VolumetricLightBufferData.NumberOfSamplesReciprocal = (1.0f / volumetricLightComp->NumberOfSamples);
					VolumetricLightBufferData.LightPower = volumetricLightComp->LightPower;
					VolumetricLightBufferData.ScatteringProbability = volumetricLightComp->ScatteringProbability;
					VolumetricLightBufferData.HenyeyGreensteinGValue = volumetricLightComp->HenyeyGreensteinGValue;

					BindBuffer(VolumetricLightBuffer, VolumetricLightBufferData, "Volumetric Light Buffer");
					Context->PSSetConstantBuffers(4, 1, &VolumetricLightBuffer);

					// Shadowbuffer
					ShadowmapBufferData.ToShadowmapView = directionalLightComp->ShadowmapView.ShadowViewMatrix;
					ShadowmapBufferData.ToShadowmapProjection = directionalLightComp->ShadowmapView.ShadowProjectionMatrix;
					ShadowmapBufferData.ShadowmapPosition = directionalLightComp->ShadowmapView.ShadowPosition;

					const auto& viewport = Viewports[directionalLightComp->ShadowmapView.ShadowmapViewportIndex];
					ShadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
					ShadowmapBufferData.ShadowAtlasResolution = ShadowAtlasResolution;
					ShadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
					ShadowmapBufferData.ShadowTestTolerance = 0.001f;

					BindBuffer(ShadowmapBuffer, ShadowmapBufferData, "Shadowmap Buffer");
					Context->PSSetConstantBuffers(5, 1, &ShadowmapBuffer);

					Context->IASetPrimitiveTopology(Topologies[static_cast<U8>(ETopologies::TriangleList)]);
					Context->IASetInputLayout(nullptr);
					Context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
					Context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

					Context->VSSetShader(VertexShaders[static_cast<U16>(EVertexShaders::Fullscreen)], nullptr, 0);
					Context->PSSetShader(PixelShaders[static_cast<U16>(EPixelShaders::VolumetricDirectional)], nullptr, 0);

					Context->Draw(3, 0);
					CRenderManager::NumberOfDrawCallsThisFrame++;

					ShouldBlurVolumetricBuffer = true;
				}
				break;

				case ERenderCommandType::VolumetricLightingPoint:
				{
					VolumetricAccumulationBuffer.SetAsActiveTarget();
					RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
					RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::FrontFaceCulling);
					IntermediateDepth.SetAsResourceOnSlot(21);
					ShadowAtlasDepth.SetAsResourceOnSlot(22);

					const auto transformComp = currentCommand.GetComponent(TransformComponent);
					const auto pointLightComp = currentCommand.GetComponent(PointLightComponent);
					const auto volumetricLightComp = currentCommand.GetComponent(VolumetricLightComponent);

					// Light Buffer
					SVector position = transformComp->Transform.GetMatrix().GetTranslation();
					PointLightBufferData.ToWorldFromObject = transformComp->Transform.GetMatrix();
					PointLightBufferData.ColorAndIntensity = pointLightComp->ColorAndIntensity;
					PointLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, pointLightComp->Range };
					BindBuffer(PointLightBuffer, PointLightBufferData, "Point Light Buffer");
					Context->VSSetConstantBuffers(3, 1, &PointLightBuffer);
					Context->PSSetConstantBuffers(3, 1, &PointLightBuffer);

					// Volumetric buffer
					VolumetricLightBufferData.NumberOfSamplesReciprocal = (1.0f / volumetricLightComp->NumberOfSamples);
					VolumetricLightBufferData.LightPower = volumetricLightComp->LightPower;
					VolumetricLightBufferData.ScatteringProbability = volumetricLightComp->ScatteringProbability;
					VolumetricLightBufferData.HenyeyGreensteinGValue = volumetricLightComp->HenyeyGreensteinGValue;

					BindBuffer(VolumetricLightBuffer, VolumetricLightBufferData, "Volumetric Light Buffer");
					Context->PSSetConstantBuffers(4, 1, &VolumetricLightBuffer);

					// Shadow Buffer
					SShadowmapBufferData shadowmapBufferData[6];
					for (U8 shadowmapViewIndex = 0; shadowmapViewIndex < 6; shadowmapViewIndex++)
					{
						shadowmapBufferData[shadowmapViewIndex].ToShadowmapView = pointLightComp->ShadowmapViews[shadowmapViewIndex].ShadowViewMatrix;
						shadowmapBufferData[shadowmapViewIndex].ToShadowmapProjection = pointLightComp->ShadowmapViews[shadowmapViewIndex].ShadowProjectionMatrix;
						shadowmapBufferData[shadowmapViewIndex].ShadowmapPosition = pointLightComp->ShadowmapViews[shadowmapViewIndex].ShadowPosition;

						const auto& viewport = Viewports[pointLightComp->ShadowmapViews[shadowmapViewIndex].ShadowmapViewportIndex];
						shadowmapBufferData[shadowmapViewIndex].ShadowmapResolution = { viewport.Width, viewport.Height };
						shadowmapBufferData[shadowmapViewIndex].ShadowAtlasResolution = ShadowAtlasResolution;
						shadowmapBufferData[shadowmapViewIndex].ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
						shadowmapBufferData[shadowmapViewIndex].ShadowTestTolerance = 0.00001f;
					}

					BindBuffer(ShadowmapBuffer, shadowmapBufferData, "Shadowmap Buffer");
					Context->PSSetConstantBuffers(5, 1, &ShadowmapBuffer);

					Context->IASetPrimitiveTopology(Topologies[static_cast<U8>(ETopologies::TriangleList)]);
					Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos4)]);
					Context->IASetVertexBuffers(0, 1, &VertexBuffers[1], &MeshVertexStrides[1], &MeshVertexOffsets[0]);
					Context->IASetIndexBuffer(IndexBuffers[1], DXGI_FORMAT_R32_UINT, 0);

					Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::PointAndSpotLight)], nullptr, 0);
					Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::VolumetricPoint)], nullptr, 0);

					Context->DrawIndexed(36, 0, 0);
					CRenderManager::NumberOfDrawCallsThisFrame++;
					RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::Default);

					ShouldBlurVolumetricBuffer = true;
				}
				break;

				case ERenderCommandType::VolumetricLightingSpot:
				{
					VolumetricAccumulationBuffer.SetAsActiveTarget();
					RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
					RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::FrontFaceCulling);
					IntermediateDepth.SetAsResourceOnSlot(21);
					ShadowAtlasDepth.SetAsResourceOnSlot(22);

					const auto transformComp = currentCommand.GetComponent(TransformComponent);
					const auto spotLightComp = currentCommand.GetComponent(SpotLightComponent);
					const auto volumetricLightComp = currentCommand.GetComponent(VolumetricLightComponent);

					// Light Buffer
					SVector position = transformComp->Transform.GetMatrix().GetTranslation();
					PointLightBufferData.ToWorldFromObject = transformComp->Transform.GetMatrix();
					PointLightBufferData.ColorAndIntensity = spotLightComp->ColorAndIntensity;
					PointLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, spotLightComp->Range };

					BindBuffer(PointLightBuffer, PointLightBufferData, "Spotlight Vertex Shader Buffer");
					Context->VSSetConstantBuffers(3, 1, &PointLightBuffer);

					SpotLightBufferData.ColorAndIntensity = spotLightComp->ColorAndIntensity;
					SpotLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, spotLightComp->Range };
					SpotLightBufferData.Direction = spotLightComp->Direction;
					SpotLightBufferData.DirectionNormal1 = spotLightComp->DirectionNormal1;
					SpotLightBufferData.DirectionNormal2 = spotLightComp->DirectionNormal2;
					SpotLightBufferData.OuterAngle = spotLightComp->OuterAngle;
					SpotLightBufferData.InnerAngle = spotLightComp->InnerAngle;

					BindBuffer(SpotLightBuffer, SpotLightBufferData, "Spotlight Pixel Shader Buffer");
					Context->PSSetConstantBuffers(3, 1, &SpotLightBuffer);

					// Volumetric buffer
					VolumetricLightBufferData.NumberOfSamplesReciprocal = (1.0f / volumetricLightComp->NumberOfSamples);
					VolumetricLightBufferData.LightPower = volumetricLightComp->LightPower;
					VolumetricLightBufferData.ScatteringProbability = volumetricLightComp->ScatteringProbability;
					VolumetricLightBufferData.HenyeyGreensteinGValue = volumetricLightComp->HenyeyGreensteinGValue;

					BindBuffer(VolumetricLightBuffer, VolumetricLightBufferData, "Volumetric Light Buffer");
					Context->PSSetConstantBuffers(4, 1, &VolumetricLightBuffer);

					// Shadow Buffer
					SShadowmapBufferData shadowmapBufferData;
					shadowmapBufferData.ToShadowmapView = spotLightComp->ShadowmapView.ShadowViewMatrix;
					shadowmapBufferData.ToShadowmapProjection = spotLightComp->ShadowmapView.ShadowProjectionMatrix;
					shadowmapBufferData.ShadowmapPosition = spotLightComp->ShadowmapView.ShadowPosition;

					const auto& viewport = Viewports[spotLightComp->ShadowmapView.ShadowmapViewportIndex];
					shadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
					shadowmapBufferData.ShadowAtlasResolution = ShadowAtlasResolution;
					shadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
					shadowmapBufferData.ShadowTestTolerance = 0.00001f;

					BindBuffer(ShadowmapBuffer, shadowmapBufferData, "Shadowmap Buffer");
					Context->PSSetConstantBuffers(5, 1, &ShadowmapBuffer);

					Context->IASetPrimitiveTopology(Topologies[static_cast<U8>(ETopologies::TriangleList)]);
					Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos4)]);
					Context->IASetVertexBuffers(0, 1, &VertexBuffers[1], &MeshVertexStrides[1], &MeshVertexOffsets[0]);
					Context->IASetIndexBuffer(IndexBuffers[1], DXGI_FORMAT_R32_UINT, 0);

					Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::PointAndSpotLight)], nullptr, 0);
					Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::VolumetricSpot)], nullptr, 0);

					Context->DrawIndexed(36, 0, 0);
					CRenderManager::NumberOfDrawCallsThisFrame++;
					RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::Default);

					ShouldBlurVolumetricBuffer = true;
				}
				break;

				case ERenderCommandType::VolumetricBufferBlurPass:
				{
					if (!ShouldBlurVolumetricBuffer)
						break;

					VolumetricBlur();
				}
				break;

				case ERenderCommandType::DebugShape: 
				{
					RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AlphaBlend);
					RenderedScene.SetAsActiveTarget();
				
					SDebugShapeComponent* shape = currentCommand.GetComponent(DebugShapeComponent);
					STransformComponent* transform = currentCommand.GetComponent(TransformComponent);
					ColorObjectBufferData.ToWorldFromObject = transform->Transform.GetMatrix();
					ColorObjectBufferData.Color = shape->Color;

					BindBuffer(ColorObjectBuffer, ColorObjectBufferData, "Object Buffer");

					Context->IASetPrimitiveTopology(Topologies[static_cast<U8>(ETopologies::LineList)]);
					Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos4)]);
					
					Context->IASetVertexBuffers(0, 1, &VertexBuffers[shape->VertexBufferIndex], &MeshVertexStrides[1], &MeshVertexOffsets[0]);
					// if indexed in the future past
					//Context->IASetIndexBuffer(lineData.myIndexBuffer, DXGI_FORMAT_R32_UINT, 0); 

					Context->VSSetConstantBuffers(1, 1, &ColorObjectBuffer);
					Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::Line)], nullptr, 0);

					Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::Line)], nullptr, 0);

					Context->Draw(shape->VertexCount, 0);
					NumberOfDrawCallsThisFrame++;
				}
				break;

				default:
					break;
				}
				PopFromCommands->pop();
			}

			RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::Disable);
			RenderStateManager.SetDepthStencilState(CRenderStateManager::EDepthStencilStates::Default);
	
			// Bloom
			RenderBloom();

			// Tonemapping
			TonemappedTexture.SetAsActiveTarget();
			RenderedScene.SetAsResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Tonemap);
	
			// Anti-aliasing
			AntiAliasedTexture.SetAsActiveTarget();
			TonemappedTexture.SetAsResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::FXAA);

			// Gamma correction
			RenderedScene.SetAsActiveTarget();
			AntiAliasedTexture.SetAsResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::GammaCorrection);

			//DebugShadowAtlas();

			// RenderedScene should be complete as that is the texture we send to the viewport
			Backbuffer.SetAsActiveTarget();
			RenderedScene.SetAsResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

			//StaticMeshInstanceTransforms.clear();

			CThreadManager::RenderThreadStatus = ERenderThreadStatus::PostRender;
			uniqueLock.unlock();
			CThreadManager::RenderCondition.notify_one();
		}

		//#pragma region Deferred Render Passes
		//		switch (RenderPassIndex)
		//		{
		//		case 1:
		//			myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::DeferredAlbedo);
		//			break;
		//		case 2:
		//			myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::DeferredNormals);
		//			break;
		//		case 3:
		//			myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::DeferredRoughness);
		//			break;
		//		case 4:
		//			myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::DeferredMetalness);
		//			break;
		//		case 5:
		//			myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::DeferredAmbientOcclusion);
		//			break;
		//		case 6:
		//			myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::DeferredEmissive);
		//			break;
		//		default:
		//			break;
		//		}
		//#pragma endregion
		//
		//		// Skybox
		//		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::Disable);
		//		myDeferredLightingTexture.SetAsActiveTarget(&myIntermediateDepth);
		//
		//		RenderStateManager.SetDepthStencilState(CRenderStateManager::EDepthStencilStates::DepthFirst);
		//		RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::FrontFaceCulling);
		//		myDeferredRenderer.RenderSkybox(maincamera, environmentlight);
		//		RenderStateManager.SetDepthStencilState(CRenderStateManager::EDepthStencilStates::Default);
		//		RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::Default);
		//
		//		// Render Lines
		//		const std::vector<CLineInstance*>& lineInstances = aScene.CullLineInstances();
		//		const std::vector<SLineTime>& lines = aScene.CullLines();
		//		ForwardRenderer.RenderLines(maincamera, lines);
		//		ForwardRenderer.RenderLineInstances(maincamera, lineInstances);
		//
		//		//VFX
		//		myDeferredLightingTexture.SetAsActiveTarget(&myIntermediateDepth);
		//		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
		//		RenderStateManager.SetDepthStencilState(CRenderStateManager::EDepthStencilStates::OnlyRead);
		//		RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::NoFaceCulling);
		//		myVFXRenderer.Render(maincamera, gameObjects);
		//		RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::Default);
		//
		//		myParticleRenderer.Render(maincamera, gameObjects);
		//
		//		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::Disable);
		//		//RenderStateManager.SetDepthStencilState(CRenderStateManager::EDepthStencilStates::Default);
		//
		//		if (RenderPassIndex == 7)
		//		{
		//			myVolumetricAccumulationBuffer.SetAsResourceOnSlot(0);
		//		}
		//
		//		if (RenderPassIndex == 8)
		//		{
		//			mySSAOBlurTexture.SetAsResourceOnSlot(0);
		//			//mySSAOBuffer.SetAsResourceOnSlot(0);
		//		}
		//
		//		if (RenderPassIndex < 2)
		//		{
		//			myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::GammaCorrection);
		//		}
		//		else
		//		{
		//			myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::GammaCorrectionRenderPass);
		//		}
		//
		//		// Vignette
		//		Backbuffer.SetAsActiveTarget();
		//		myVignetteTexture.SetAsResourceOnSlot(0);
		//		myVignetteOverlayTexture.SetAsResourceOnSlot(1);
		//		myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::Vignette);
		//
		//		//Backbuffer.SetAsActiveTarget();
		//
		//		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AlphaBlend);
		//		RenderStateManager.SetDepthStencilState(CRenderStateManager::EDepthStencilStates::OnlyRead);
		//
		//		std::vector<CSpriteInstance*> sprites;
		//		std::vector<CSpriteInstance*> animatedUIFrames;
		//		std::vector<CTextInstance*> textsToRender;
		//		std::vector<CAnimatedUIElement*> animatedUIElements;
		//
		//		const CCanvas* canvas = aScene.Canvas();
		//		if (canvas)
		//		{
		//			canvas->EmplaceSprites(sprites);
		//			animatedUIElements = canvas->EmplaceAnimatedUI(animatedUIFrames);
		//			canvas->EmplaceTexts(textsToRender);
		//		}
		//
		//		// Sprites
		//		CMainSingleton::PopupTextService().EmplaceSprites(sprites);
		//		CMainSingleton::DialogueSystem().EmplaceSprites(sprites);
		//		CEngine::GetActiveScene().MainCamera()->EmplaceSprites(animatedUIFrames);
		//		mySpriteRenderer.Render(sprites);
		//		mySpriteRenderer.Render(animatedUIElements);
		//		mySpriteRenderer.Render(animatedUIFrames);
		//
		//		// Text
		//		CMainSingleton::PopupTextService().EmplaceTexts(textsToRender);
		//		CMainSingleton::DialogueSystem().EmplaceTexts(textsToRender);
		//		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::Disable);
		//		RenderStateManager.SetDepthStencilState(CRenderStateManager::EDepthStencilStates::Default);
		//		myTextRenderer.Render(textsToRender);
	}

	void CRenderManager::Release()
	{
		Clear(ClearColor);
		GEngine::Instance->Framework->GetContext()->OMSetRenderTargets(0, 0, 0);
		GEngine::Instance->Framework->GetContext()->OMGetDepthStencilState(0, 0);
		GEngine::Instance->Framework->GetContext()->ClearState();

		//Backbuffer.ReleaseTexture();
		//myIntermediateTexture.ReleaseTexture();
		//myIntermediateDepth.ReleaseDepth();
		//myLuminanceTexture.ReleaseTexture();
		//myHalfSizeTexture.ReleaseTexture();
		//myQuarterSizeTexture.ReleaseTexture();
		//myBlurTexture1.ReleaseTexture();
		//myBlurTexture2.ReleaseTexture();
		//myVignetteTexture.ReleaseTexture();
		//myVignetteOverlayTexture.ReleaseTexture();
		//myDeferredLightingTexture.ReleaseTexture();

		//myEnvironmentShadowDepth.ReleaseDepth();
		//myBoxLightShadowDepth.ReleaseDepth();
		//myDepthCopy.ReleaseDepth();
		//myDownsampledDepth.ReleaseDepth();

		//myVolumetricAccumulationBuffer.ReleaseTexture();
		//myVolumetricBlurTexture.ReleaseTexture();
		//myTonemappedTexture.ReleaseTexture();
		//myAntiAliasedTexture.ReleaseTexture();

		//myGBuffer.ReleaseResources();
		//myGBufferCopy.ReleaseResources();
	}

	std::string CRenderManager::ConvertToHVA(const std::string& filePath, const std::string& destination, EAssetType assetType) const
	{
		std::string hvaPath;
		switch (assetType)
		{
		case EAssetType::StaticMesh:
			{
				hvaPath = CModelImporter::ImportFBX(filePath);
			}
			break;
		case EAssetType::Texture:
			{
				std::string textureFileData;
				GEngine::GetFileSystem()->Deserialize(filePath, textureFileData);

				ETextureFormat format = {};
				if (const std::string extension = filePath.substr(filePath.size() - 4); extension == ".dds")
					format = ETextureFormat::DDS;
				else if (extension == ".tga")
					format = ETextureFormat::TGA;

				STextureFileHeader asset;
				asset.AssetType = EAssetType::Texture;

				asset.MaterialName = destination + filePath.substr(filePath.find_last_of('\\'), filePath.find_first_of('.') - filePath.find_last_of('\\'));// destination.substr(0, destination.find_last_of("."));
				asset.MaterialNameLength = static_cast<U32>(asset.MaterialName.length());
				asset.OriginalFormat = format;
				asset.Suffix = filePath[filePath.find_last_of(".") - 1];
				asset.DataSize = static_cast<U32>(textureFileData.length() * sizeof(char));
				asset.Data = std::move(textureFileData);

				const auto data = new char[asset.GetSize()];

				asset.Serialize(data);
				GEngine::GetFileSystem()->Serialize(asset.MaterialName + ".hva", &data[0], asset.GetSize());
				delete[] data;

				hvaPath = asset.MaterialName + ".hva";
			}
			break;
		case EAssetType::SkeletalMesh: 
			break;
		case EAssetType::Animation: 
			break;
		case EAssetType::AudioOneShot: 
			break;
		case EAssetType::AudioCollection: 
			break;
		case EAssetType::VisualFX: 
			break;
		}

		return hvaPath;
	}

	void CRenderManager::LoadStaticMeshComponent(const std::string& filePath, SStaticMeshComponent* outStaticMeshComponent)
	{
		SStaticMeshAsset asset;
		if (!LoadedStaticMeshes.contains(filePath))
		{
			// Asset Loading
			const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(filePath);
			char* data = new char[fileSize];

			GEngine::GetFileSystem()->Deserialize(filePath, data, static_cast<U32>(fileSize));

			SStaticModelFileHeader assetFile;
			assetFile.Deserialize(data);
			asset = SStaticMeshAsset(assetFile);

			for (U16 i = 0; i < assetFile.NumberOfMeshes; i++)
			{
				asset.DrawCallData[i].VertexBufferIndex = AddVertexBuffer(assetFile.Meshes[i].Vertices);
				asset.DrawCallData[i].IndexBufferIndex = AddIndexBuffer(assetFile.Meshes[i].Indices);
				asset.DrawCallData[i].VertexStrideIndex = /*AddMeshVertexStride(static_cast<U32>(sizeof(SStaticMeshVertex)))*/0;
				asset.DrawCallData[i].VertexOffsetIndex = /*AddMeshVertexOffset(0)*/0;
			}

			// NR: Mesh name will be much easier to handle
			LoadedStaticMeshes.emplace(UGeneralUtils::ExtractFileNameFromPath(filePath), asset);
			delete[] data;
		}
		else
		{
			asset = LoadedStaticMeshes.at(filePath);
		}

		outStaticMeshComponent->Name = UGeneralUtils::ExtractFileNameFromPath(filePath);
		outStaticMeshComponent->NumberOfMaterials = asset.NumberOfMaterials;

		// Geometry
		outStaticMeshComponent->VertexShaderIndex = static_cast<U8>(EVertexShaders::StaticMesh);
		outStaticMeshComponent->PixelShaderIndex = static_cast<U8>(EPixelShaders::GBuffer);
		outStaticMeshComponent->InputLayoutIndex = static_cast<U8>(EInputLayoutType::Pos3Nor3Tan3Bit3UV2);
		outStaticMeshComponent->SamplerIndex = static_cast<U8>(ESamplers::DefaultWrap);
		outStaticMeshComponent->TopologyIndex = static_cast<U8>(ETopologies::TriangleList);
		outStaticMeshComponent->DrawCallData = asset.DrawCallData;
	}

	void CRenderManager::LoadMaterialComponent(const std::vector<std::string>& materialNames, SMaterialComponent* outMaterialComponent)
	{
		outMaterialComponent->MaterialReferences.clear();
		for (const std::string& materialName : materialNames)
		{
			std::vector<U16> references = AddMaterial(materialName, MaterialConfiguration);
			
			for (U16 reference : references)
				outMaterialComponent->MaterialReferences.emplace_back(reference);
		}
	}

	void CRenderManager::LoadDecalComponent(const std::vector<std::string>& textureNames, SDecalComponent* outDecalComponent)
	{
		outDecalComponent->TextureReferences.clear();
		auto textureBank = GEngine::GetTextureBank();

		for (const std::string& textureName: textureNames)
		{
			outDecalComponent->TextureReferences.emplace_back(static_cast<U16>(textureBank->GetTextureIndex("Assets/Textures/" + textureName + ".hva")));
		}
	}

	void CRenderManager::LoadEnvironmentLightComponent(const std::string& ambientCubemapTextureName, SEnvironmentLightComponent* outEnvironmentLightComponent)
	{
		auto textureBank = GEngine::GetTextureBank();
		outEnvironmentLightComponent->AmbientCubemapReference = static_cast<U16>(textureBank->GetTextureIndex("Assets/Textures/Cubemaps/" + ambientCubemapTextureName + ".hva"));
	}

	EMaterialConfiguration CRenderManager::GetMaterialConfiguration() const
	{
		return MaterialConfiguration;
	}

	SVector2<F32> CRenderManager::GetShadowAtlasResolution() const
	{
		return ShadowAtlasResolution;
	}

	bool CRenderManager::TryLoadStaticMeshComponent(const std::string& fileName, SStaticMeshComponent* outStaticMeshComponent) const
	{
		SStaticMeshAsset asset;

		if (!LoadedStaticMeshes.contains(fileName))
			return false;
		else
			asset = LoadedStaticMeshes.at(fileName);

		outStaticMeshComponent->Name = fileName;
		outStaticMeshComponent->NumberOfMaterials = asset.NumberOfMaterials;

		// Geometry
		outStaticMeshComponent->VertexShaderIndex = static_cast<U8>(EVertexShaders::StaticMesh);
		outStaticMeshComponent->PixelShaderIndex = static_cast<U8>(EPixelShaders::GBuffer);
		outStaticMeshComponent->InputLayoutIndex = static_cast<U8>(EInputLayoutType::Pos3Nor3Tan3Bit3UV2);
		outStaticMeshComponent->SamplerIndex = static_cast<U8>(ESamplers::DefaultWrap);
		outStaticMeshComponent->TopologyIndex = static_cast<U8>(ETopologies::TriangleList);
		outStaticMeshComponent->DrawCallData = asset.DrawCallData;

		return true;
	}

	void* CRenderManager::RenderStaticMeshAssetTexture(const std::string& filePath)
	{
		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.Width = static_cast<U16>(256.0f);
		textureDesc.Height = static_cast<U16>(256.0f);
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		D3D11_TEXTURE2D_DESC depthStencilDesc = { 0 };
		depthStencilDesc.Width = static_cast<U16>(256.0f);
		depthStencilDesc.Height = static_cast<U16>(256.0f);
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		depthStencilViewDesc.Flags = 0;

		ID3D11Texture2D* depthStencilBuffer;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer), "Texture could not be created.");
		
		ID3D11DepthStencilView* depthStencilView;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView), "Depth could not be created.");

		ID3D11Texture2D* texture;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateTexture2D(&textureDesc, nullptr, &texture), "Could not create Fullscreen Texture2D");

		ID3D11ShaderResourceView* shaderResource;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateShaderResourceView(texture, nullptr, &shaderResource), "Could not create Fullscreen Shader Resource View.");

		ID3D11RenderTargetView* renderTarget;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateRenderTargetView(texture, nullptr, &renderTarget), "Could not create Fullcreen Render Target View.");

		D3D11_VIEWPORT* viewport;
		D3D11_TEXTURE2D_DESC textureDescription;
		texture->GetDesc(&textureDescription);
		viewport = new D3D11_VIEWPORT({ 0.0f, 0.0f, static_cast<F32>(textureDescription.Width), static_cast<F32>(textureDescription.Height), 0.0f, 1.0f });

		// TODO.NR: Figure out why the depth doesn't work
		Context->OMSetRenderTargets(1, &renderTarget, nullptr);
		Context->RSSetViewports(1, viewport);

		STransform camTransform;
		camTransform.Translate(SVector4::Right * 1.5f);
		camTransform.Translate(SVector4::Backward * 2.5f);
		camTransform.Translate(SVector4::Up * 2.0f);
		camTransform.Rotate({UMath::DegToRad(-30.0f), UMath::DegToRad(30.0f), 0.0f});
		SMatrix camProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), (16.0f / 9.0f), 0.1f, 10.0f);

		FrameBufferData.ToCameraFromWorld = camTransform.GetMatrix().FastInverse();
		FrameBufferData.ToWorldFromCamera = camTransform.GetMatrix();
		FrameBufferData.ToProjectionFromCamera = camProjection;
		FrameBufferData.ToCameraFromProjection = camProjection.Inverse();
		FrameBufferData.CameraPosition = camTransform.GetMatrix().GetTranslation4();
		BindBuffer(FrameBuffer, FrameBufferData, "Frame Buffer");

		Context->VSSetConstantBuffers(0, 1, &FrameBuffer);
		Context->PSSetConstantBuffers(0, 1, &FrameBuffer);

		ObjectBufferData.ToWorldFromObject = SMatrix();
		BindBuffer(ObjectBuffer, ObjectBufferData, "Object Buffer");

		Ref<SEntity> tempEntity = std::make_shared<SEntity>(0, "Temp");
		SStaticMeshComponent* staticMeshComp = new SStaticMeshComponent(tempEntity, EComponentType::StaticMeshComponent);
		LoadStaticMeshComponent(filePath, staticMeshComp);

		Context->VSSetConstantBuffers(1, 1, &ObjectBuffer);
		Context->IASetPrimitiveTopology(Topologies[staticMeshComp->TopologyIndex]);
		Context->IASetInputLayout(InputLayouts[staticMeshComp->InputLayoutIndex]);

		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::EditorPreview)], nullptr, 0);
		Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::EditorPreview)], nullptr, 0);

		ID3D11SamplerState* sampler = Samplers[staticMeshComp->SamplerIndex];
		Context->PSSetSamplers(0, 1, &sampler);

		for (U8 drawCallIndex = 0; drawCallIndex < static_cast<U8>(staticMeshComp->DrawCallData.size()); drawCallIndex++)
		{
			const SDrawCallData& drawData = staticMeshComp->DrawCallData[drawCallIndex];
			ID3D11Buffer* vertexBuffer = VertexBuffers[drawData.VertexBufferIndex];
			Context->IASetVertexBuffers(0, 1, &vertexBuffer, &MeshVertexStrides[drawData.VertexStrideIndex], &MeshVertexOffsets[drawData.VertexStrideIndex]);
			Context->IASetIndexBuffer(IndexBuffers[drawData.IndexBufferIndex], DXGI_FORMAT_R32_UINT, 0);
			Context->DrawIndexed(drawData.IndexCount, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}

		delete staticMeshComp;
		delete viewport;
		renderTarget->Release();
		texture->Release();
		depthStencilView->Release();
		depthStencilBuffer->Release();

		return std::move((void*)shaderResource);
	}

	void* CRenderManager::GetTextureAssetTexture(const std::string& filePath)
	{
		// Asset Loading
		const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(filePath);
		char* data = new char[fileSize];

		GEngine::GetFileSystem()->Deserialize(filePath, data, static_cast<U32>(fileSize));

		STextureFileHeader assetFile;
		assetFile.Deserialize(data);
		STextureAsset asset = STextureAsset(assetFile, Framework->GetDevice());

		return asset.ShaderResourceView;
	}

	bool CRenderManager::IsStaticMeshInInstancedRenderList(const std::string& meshName)
	{
		return SystemStaticMeshInstanceTransforms.contains(meshName);
	}

	void CRenderManager::AddStaticMeshToInstancedRenderList(const std::string& meshName, const SMatrix& transformMatrix)
	{
		if (!SystemStaticMeshInstanceTransforms.contains(meshName))
			SystemStaticMeshInstanceTransforms.emplace(meshName, std::vector<SMatrix>());

		SystemStaticMeshInstanceTransforms[meshName].emplace_back(transformMatrix);
	}

	void CRenderManager::SwapStaticMeshInstancedRenderLists()
	{
		std::swap(SystemStaticMeshInstanceTransforms, RendererStaticMeshInstanceTransforms);
	}

	void CRenderManager::ClearSystemStaticMeshInstanceTransforms()
	{
		return SystemStaticMeshInstanceTransforms.clear();
	}

	const CFullscreenTexture& CRenderManager::GetRenderedSceneTexture() const
	{
		return RenderedScene;
	}

	void CRenderManager::PushRenderCommand(SRenderCommand& command)
	{
		PushToCommands->push(command);
	}

	void CRenderManager::SwapRenderCommandBuffers()
	{
		std::swap(PushToCommands, PopFromCommands);
	}

	void CRenderManager::Clear(SVector4 /*clearColor*/)
	{
		//Backbuffer.ClearTexture(clearColor);
		//myIntermediateDepth.ClearDepth();
	}

	void CRenderManager::ToggleRenderPass(bool shouldToggleForwards)
	{
		if (!shouldToggleForwards)
		{
			--RenderPassIndex;
			if (RenderPassIndex < 0) {
				RenderPassIndex = 8;
			}
			return;
		}

		++RenderPassIndex;
		if (RenderPassIndex > 8)
		{
			RenderPassIndex = 0;
		}
	}

	void CRenderManager::VolumetricBlur()
	{
		// Downsampling and Blur
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::Disable);
		DownsampledDepth.SetAsActiveTarget();
		IntermediateDepth.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::DownsampleDepth);

		// Blur
		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralHorizontal);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralVertical);

		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralHorizontal);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralVertical);

		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralHorizontal);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralVertical);

		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralHorizontal);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralVertical);

		// Upsampling
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
		RenderedScene.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsResourceOnSlot(0);
		DownsampledDepth.SetAsResourceOnSlot(1);
		IntermediateDepth.SetAsResourceOnSlot(2);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::DepthAwareUpsampling);
	}

	void CRenderManager::RenderBloom()
	{
		HalfSizeTexture.SetAsActiveTarget();
		RenderedScene.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		QuarterSizeTexture.SetAsActiveTarget();
		HalfSizeTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		BlurTexture1.SetAsActiveTarget();
		QuarterSizeTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		BlurTexture2.SetAsActiveTarget();
		BlurTexture1.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::GaussianHorizontal);

		BlurTexture1.SetAsActiveTarget();
		BlurTexture2.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::GaussianVertical);

		BlurTexture2.SetAsActiveTarget();
		BlurTexture1.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::GaussianHorizontal);

		BlurTexture1.SetAsActiveTarget();
		BlurTexture2.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::GaussianVertical);

		QuarterSizeTexture.SetAsActiveTarget();
		BlurTexture1.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		HalfSizeTexture.SetAsActiveTarget();
		QuarterSizeTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		VignetteTexture.SetAsActiveTarget();
		RenderedScene.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		RenderedScene.SetAsActiveTarget();
		VignetteTexture.SetAsResourceOnSlot(0);
		HalfSizeTexture.SetAsResourceOnSlot(1);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Bloom);
	}

	void CRenderManager::DebugShadowAtlas()
	{
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = 256.0f;
		viewport.Height = 256.0f;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		Context->RSSetViewports(1, &viewport);
		ShadowAtlasDepth.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::CopyDepth);
	}

	U16 CRenderManager::AddIndexBuffer(const std::vector<U32>& indices)
	{
		D3D11_BUFFER_DESC indexBufferDesc = { 0 };
		indexBufferDesc.ByteWidth = sizeof(U32) * static_cast<U32>(indices.size());
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubresourceData = { 0 };
		indexSubresourceData.pSysMem = indices.data();

		ID3D11Buffer* indexBuffer;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &indexBuffer), "Index Buffer could not be created.");
		IndexBuffers.emplace_back(indexBuffer);

		return static_cast<U16>(IndexBuffers.size() - 1);
	}

	U16 CRenderManager::AddMeshVertexOffset(U32 offset)
	{
		MeshVertexOffsets.emplace_back(offset);
		return static_cast<U16>(MeshVertexOffsets.size() - 1);
	}

	U16 CRenderManager::AddMeshVertexStride(U32 stride)
	{
		MeshVertexStrides.emplace_back(stride);
		return static_cast<U16>(MeshVertexStrides.size() - 1);
	}

	std::string CRenderManager::AddShader(const std::string& fileName, const EShaderType shaderType)
	{
		std::string outShaderData;

		switch (shaderType)
		{
		case EShaderType::Vertex:
		{
			ID3D11VertexShader* vertexShader;
			UGraphicsUtils::CreateVertexShader(fileName, Framework, &vertexShader, outShaderData);
			VertexShaders.emplace_back(vertexShader);
		}
		break;
		case EShaderType::Compute:
		case EShaderType::Geometry:
			break;
		case EShaderType::Pixel:
		{
			ID3D11PixelShader* pixelShader;
			UGraphicsUtils::CreatePixelShader(fileName, Framework, &pixelShader);
			PixelShaders.emplace_back(pixelShader);
		}
		break;
		}

		return outShaderData;
	}

	void CRenderManager::AddInputLayout(const std::string& vsData, EInputLayoutType layoutType)
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

		case EInputLayoutType::Pos4:
			layout =
			{
				{"POSITION"	,	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
			};
			break;
		}
		ID3D11InputLayout* inputLayout;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateInputLayout(layout.data(), static_cast<U32>(layout.size()), vsData.data(), vsData.size(), &inputLayout), "Input Layout could not be created.")
			InputLayouts.emplace_back(inputLayout);
	}

	void CRenderManager::AddSampler(ESamplerType samplerType)
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

	void CRenderManager::AddTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
	{
		Topologies.emplace_back(topology);
	}

	void CRenderManager::AddViewport(SVector2<F32> topLeftCoordinate, SVector2<F32> widthAndHeight, SVector2<F32> depth)
	{
		Viewports.emplace_back(D3D11_VIEWPORT(topLeftCoordinate.X, topLeftCoordinate.Y, widthAndHeight.X, widthAndHeight.Y, depth.X, depth.Y));
	}

	std::vector<U16> CRenderManager::AddMaterial(const std::string& materialName, EMaterialConfiguration configuration)
	{
		std::vector<U16> references;
		
		switch (configuration)
		{
		case EMaterialConfiguration::AlbedoMaterialNormal_Packed:
		{
			const std::string texturesFolder = "Assets/Textures/";
			auto textureBank = GEngine::GetTextureBank();

			references.emplace_back(static_cast<U16>(textureBank->GetTextureIndex(texturesFolder + materialName + "_c.hva")));
			references.emplace_back(static_cast<U16>(textureBank->GetTextureIndex(texturesFolder + materialName + "_m.hva")));
			references.emplace_back(static_cast<U16>(textureBank->GetTextureIndex(texturesFolder + materialName + "_n.hva")));
		}
			break;
		}

		return references;
	}

	bool SRenderCommandComparer::operator()(const SRenderCommand& a, const SRenderCommand& b) const
	{
		return 	static_cast<U16>(a.Type) > static_cast<U16>(b.Type);
	}
}
