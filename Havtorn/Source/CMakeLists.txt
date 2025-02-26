cmake_minimum_required (VERSION 3.31)

project (Havtorn
  DESCRIPTION "Havtorn"
  LANGUAGES CXX
  VERSION 1.0.0)

# option(BUILD_TEST_TEMPLATE "Ignore warnings related to TODOs" OFF)

# option(ENABLE_CODE_ANALYSIS "Use Static Code Analysis on build" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(ENGINE_FOLDER "Engine/")
set(EDITOR_FOLDER "Editor/")
set(GAME_FOLDER "Game/")
set(GUI_FOLDER "GUI/")
#set(IMGUI_FOLDER "ImGui/")
set(LAUNCHER_FOLDER "Launcher/")
set(EXTERNAL_FOLDER "../External/")
set(WORKING_FOLDER "${CMAKE_BINARY_DIR}/../Bin/")

set(COMMON_COMPILE_DEFINITIONS HV_PLATFORM_WINDOWS HV_DEBUG HV_ENABLE_ASSERTS _UNICODE UNICODE)
set(COMMON_COMPILE_OPTIONS /W4 /WX /MP /ZI /JMC /fp:fast /FC)
set(COMMON_LINK_OPTIONS /WX /SUBSYSTEM:WINDOWS)

# ==================== GUI ====================
set(GUI_FILES
   ${GUI_FOLDER}GUI.cpp
   ${GUI_FOLDER}GUI.h
   ${GUI_FOLDER}GUIProcess.cpp
   ${GUI_FOLDER}GUIProcess.h
)
add_library(GUI STATIC ${GUI_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${GUI_FILES})
target_include_directories(GUI PRIVATE
	"${GUI_FOLDER}"
	"${ENGINE_FOLDER}"
    "${EXTERNAL_FOLDER}imgui"
    "${EXTERNAL_FOLDER}ImGuizmo"
)
target_compile_definitions(GUI PRIVATE ${COMMON_COMPILE_DEFINITIONS} HV_BUILD_DLL)
target_compile_options(GUI PRIVATE /W3 /MP /ZI /JMC /fp:fast /FC)
set_property(TARGET GUI PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${WORKING_FOLDER})
set_target_properties(GUI PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${WORKING_FOLDER} RUNTIME_OUTPUT_DIRECTORY_RELEASE ${WORKING_FOLDER})
# ==================== GUI ====================

# ==================== ENGINE ====================
set(ENGINE_FILES
    ${ENGINE_FOLDER}Application/EngineProcess.cpp
    ${ENGINE_FOLDER}Application/EngineProcess.h
    #${ENGINE_FOLDER}Application/ImGuiCrossProjectSetup.h
    #${ENGINE_FOLDER}Application/ImGuiManager.cpp
    #${ENGINE_FOLDER}Application/ImGuiManager.h
    #${ENGINE_FOLDER}Application/ImGuiProcess.cpp
    #${ENGINE_FOLDER}Application/ImGuiProcess.h
    ${ENGINE_FOLDER}Application/WindowHandler.cpp
    ${ENGINE_FOLDER}Application/WindowHandler.h
    ${ENGINE_FOLDER}Assets/SequencerAsset.cpp
    ${ENGINE_FOLDER}Assets/SequencerAsset.h
    ${ENGINE_FOLDER}Core/MathTypes/AABB3D.h
    ${ENGINE_FOLDER}Core/MathTypes/Curve.cpp
    ${ENGINE_FOLDER}Core/MathTypes/Curve.h
    ${ENGINE_FOLDER}Core/MathTypes/EngineMath.cpp
    ${ENGINE_FOLDER}Core/MathTypes/EngineMath.h
    ${ENGINE_FOLDER}Core/MathTypes/EngineMathSSE.h
    ${ENGINE_FOLDER}Core/MathTypes/Frustum.cpp
    ${ENGINE_FOLDER}Core/MathTypes/Frustum.h
    ${ENGINE_FOLDER}Core/MathTypes/HavtornMath.h
    ${ENGINE_FOLDER}Core/MathTypes/Intersection.h
    ${ENGINE_FOLDER}Core/MathTypes/Line.h
    ${ENGINE_FOLDER}Core/MathTypes/LineVolume.h
    ${ENGINE_FOLDER}Core/MathTypes/MathUtilities.h
    ${ENGINE_FOLDER}Core/MathTypes/Matrix.cpp
    ${ENGINE_FOLDER}Core/MathTypes/Matrix.h
    ${ENGINE_FOLDER}Core/MathTypes/Matrix3x3.h
    ${ENGINE_FOLDER}Core/MathTypes/Plane.h
    ${ENGINE_FOLDER}Core/MathTypes/PlaneVolume.h
    ${ENGINE_FOLDER}Core/MathTypes/Quaternion.cpp
    ${ENGINE_FOLDER}Core/MathTypes/Quaternion.h
    ${ENGINE_FOLDER}Core/MathTypes/Ray.h
    ${ENGINE_FOLDER}Core/MathTypes/Rotator.h
    ${ENGINE_FOLDER}Core/MathTypes/Sphere.h
    ${ENGINE_FOLDER}Core/MathTypes/Transform.h
    ${ENGINE_FOLDER}Core/MathTypes/Vector.cpp
    ${ENGINE_FOLDER}Core/MathTypes/Vector.h
    ${ENGINE_FOLDER}Core/BitSet.h
    ${ENGINE_FOLDER}Core/Color.cpp
    ${ENGINE_FOLDER}Core/Color.h
    ${ENGINE_FOLDER}Core/Core.h
    ${ENGINE_FOLDER}Core/CoreTypes.h
    ${ENGINE_FOLDER}Core/Delegate.cpp
    ${ENGINE_FOLDER}Core/Delegate.h
    ${ENGINE_FOLDER}Core/EngineException.h
    ${ENGINE_FOLDER}Core/EngineTypes.h
    ${ENGINE_FOLDER}Core/GeneralUtilities.h
    ${ENGINE_FOLDER}Core/HavtornDelegate.h
    ${ENGINE_FOLDER}Core/HavtornString.cpp
    ${ENGINE_FOLDER}Core/HavtornString.h
    ${ENGINE_FOLDER}Core/ImGuiTypeUtil.cpp
    ${ENGINE_FOLDER}Core/ImGuiTypeUtil.h
    ${ENGINE_FOLDER}Core/RuntimeAssetDeclarations.h
    ${ENGINE_FOLDER}Core/Serializable.cpp
    ${ENGINE_FOLDER}Core/Serializable.h
    ${ENGINE_FOLDER}Core/WindowsInclude.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/CameraComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/CameraComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/CameraControllerComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/CameraControllerComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/DecalComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/DecalComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/DirectionalLightComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/DirectionalLightComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/EnvironmentLightComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/EnvironmentLightComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/MaterialComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/MaterialComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/Physics2DComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/Physics2DComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/Physics3DComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/Physics3DComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/Physics3DControllerComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/Physics3DControllerComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/PointLightComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/PointLightComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/SequencerComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/SequencerComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/SkeletalAnimationComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/SkeletalAnimationComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/SkeletalMeshComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/SkeletalMeshComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/SpotLightComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/SpotLightComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/SpriteAnimatorGraphComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/SpriteAnimatorGraphComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/SpriteComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/SpriteComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/StaticMeshComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/StaticMeshComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/Transform2DComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/Transform2DComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/TransformComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/TransformComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/VolumetricLightComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/VolumetricLightComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/Components/CameraComponent.h
    ${ENGINE_FOLDER}ECS/Components/CameraControllerComponent.h
    ${ENGINE_FOLDER}ECS/Components/DecalComponent.cpp
    ${ENGINE_FOLDER}ECS/Components/DecalComponent.h
    ${ENGINE_FOLDER}ECS/Components/DirectionalLightComponent.h
    ${ENGINE_FOLDER}ECS/Components/EnvironmentLightComponent.h
    ${ENGINE_FOLDER}ECS/Components/MaterialComponent.cpp
    ${ENGINE_FOLDER}ECS/Components/MaterialComponent.h
    ${ENGINE_FOLDER}ECS/Components/MetaDataComponent.h
    ${ENGINE_FOLDER}ECS/Components/Physics2DComponent.h
    ${ENGINE_FOLDER}ECS/Components/Physics3DComponent.h
    ${ENGINE_FOLDER}ECS/Components/Physics3DControllerComponent.h
    ${ENGINE_FOLDER}ECS/Components/PointLightComponent.h
    ${ENGINE_FOLDER}ECS/Components/SequencerComponent.h
    ${ENGINE_FOLDER}ECS/Components/SkeletalAnimationComponent.cpp
    ${ENGINE_FOLDER}ECS/Components/SkeletalAnimationComponent.h
    ${ENGINE_FOLDER}ECS/Components/SkeletalMeshComponent.cpp
    ${ENGINE_FOLDER}ECS/Components/SkeletalMeshComponent.h
    ${ENGINE_FOLDER}ECS/Components/SpotLightComponent.h
    ${ENGINE_FOLDER}ECS/Components/SpriteAnimatorGraphComponent.cpp
    ${ENGINE_FOLDER}ECS/Components/SpriteAnimatorGraphComponent.h
    ${ENGINE_FOLDER}ECS/Components/SpriteAnimatorGraphNode.cpp
    ${ENGINE_FOLDER}ECS/Components/SpriteAnimatorGraphNode.h
    ${ENGINE_FOLDER}ECS/Components/SpriteComponent.h
    ${ENGINE_FOLDER}ECS/Components/StaticMeshComponent.cpp
    ${ENGINE_FOLDER}ECS/Components/StaticMeshComponent.h
    ${ENGINE_FOLDER}ECS/Components/Transform2DComponent.h
    ${ENGINE_FOLDER}ECS/Components/TransformComponent.h
    ${ENGINE_FOLDER}ECS/Components/VolumetricLightComponent.h
    ${ENGINE_FOLDER}ECS/Systems/AnimatorGraphSystem.cpp
    ${ENGINE_FOLDER}ECS/Systems/AnimatorGraphSystem.h
    ${ENGINE_FOLDER}ECS/Systems/CameraSystem.cpp
    ${ENGINE_FOLDER}ECS/Systems/CameraSystem.h
    ${ENGINE_FOLDER}ECS/Systems/DebugShapeSystem.cpp
    ${ENGINE_FOLDER}ECS/Systems/DebugShapeSystem.h
    ${ENGINE_FOLDER}ECS/Systems/LightSystem.cpp
    ${ENGINE_FOLDER}ECS/Systems/LightSystem.h
    ${ENGINE_FOLDER}ECS/Systems/RenderSystem.cpp
    ${ENGINE_FOLDER}ECS/Systems/RenderSystem.h
    ${ENGINE_FOLDER}ECS/Systems/SequencerSystem.cpp
    ${ENGINE_FOLDER}ECS/Systems/SequencerSystem.h
    ${ENGINE_FOLDER}ECS/Systems/SpriteAnimatorGraphSystem.cpp
    ${ENGINE_FOLDER}ECS/Systems/SpriteAnimatorGraphSystem.h
    "${ENGINE_FOLDER}ECS/Component.cpp"
    "${ENGINE_FOLDER}ECS/Component.h"
    ${ENGINE_FOLDER}ECS/ComponentAlgo.h
    ${ENGINE_FOLDER}ECS/ComponentEditorContext.h
    ${ENGINE_FOLDER}ECS/ECSInclude.h
    ${ENGINE_FOLDER}ECS/Entity.cpp
    ${ENGINE_FOLDER}ECS/Entity.h
    ${ENGINE_FOLDER}ECS/GUIDManager.cpp
    ${ENGINE_FOLDER}ECS/GUIDManager.h
    "${ENGINE_FOLDER}ECS/System.cpp"
    "${ENGINE_FOLDER}ECS/System.h"
    ${ENGINE_FOLDER}FileSystem/FileHeaderDeclarations.h
    ${ENGINE_FOLDER}FileSystem/FileSystem.cpp
    ${ENGINE_FOLDER}FileSystem/FileSystem.h
    ${ENGINE_FOLDER}Graphics/Debug/DebugDrawUtility.cpp
    ${ENGINE_FOLDER}Graphics/Debug/DebugDrawUtility.h
    ${ENGINE_FOLDER}Graphics/Renderers/FullscreenRenderer.cpp
    ${ENGINE_FOLDER}Graphics/Renderers/FullscreenRenderer.h
    ${ENGINE_FOLDER}Graphics/RenderingPrimitives/DataBuffer.cpp
    ${ENGINE_FOLDER}Graphics/RenderingPrimitives/DataBuffer.h
    ${ENGINE_FOLDER}Graphics/RenderingPrimitives/FullscreenTexture.cpp
    ${ENGINE_FOLDER}Graphics/RenderingPrimitives/FullscreenTexture.h
    ${ENGINE_FOLDER}Graphics/RenderingPrimitives/GBuffer.cpp
    ${ENGINE_FOLDER}Graphics/RenderingPrimitives/GBuffer.h
    ${ENGINE_FOLDER}Graphics/FullscreenTextureFactory.cpp
    ${ENGINE_FOLDER}Graphics/FullscreenTextureFactory.h
    ${ENGINE_FOLDER}Graphics/GeometryPrimitives.h
    ${ENGINE_FOLDER}Graphics/GraphicsEnums.h
    ${ENGINE_FOLDER}Graphics/GraphicsFramework.cpp
    ${ENGINE_FOLDER}Graphics/GraphicsFramework.h
    ${ENGINE_FOLDER}Graphics/GraphicsMaterial.h
    ${ENGINE_FOLDER}Graphics/GraphicsStructs.h
    ${ENGINE_FOLDER}Graphics/GraphicsUtilities.cpp
    ${ENGINE_FOLDER}Graphics/GraphicsUtilities.h
    ${ENGINE_FOLDER}Graphics/RenderCommand.h
    ${ENGINE_FOLDER}Graphics/RenderManager.cpp
    ${ENGINE_FOLDER}Graphics/RenderManager.h
    ${ENGINE_FOLDER}Graphics/RenderStateManager.cpp
    ${ENGINE_FOLDER}Graphics/RenderStateManager.h
    ${ENGINE_FOLDER}Graphics/TextureBank.cpp
    ${ENGINE_FOLDER}Graphics/TextureBank.h
    ${ENGINE_FOLDER}HexPhys/HexPhys.cpp
    ${ENGINE_FOLDER}HexPhys/HexPhys.h
    "${ENGINE_FOLDER}Input/Input.cpp"
    "${ENGINE_FOLDER}Input/Input.h"
    "${ENGINE_FOLDER}Input/InputMapper.cpp"
    "${ENGINE_FOLDER}Input/InputMapper.h"
    "${ENGINE_FOLDER}Input/InputTypes.h"
    ${ENGINE_FOLDER}Scene/AssetRegistry.cpp
    ${ENGINE_FOLDER}Scene/AssetRegistry.h
    ${ENGINE_FOLDER}Scene/Scene.cpp
    ${ENGINE_FOLDER}Scene/Scene.h
    ${ENGINE_FOLDER}Scene/World.cpp
    ${ENGINE_FOLDER}Scene/World.h
    ${ENGINE_FOLDER}SequencerKeyframes/SequencerKeyframe.cpp
    ${ENGINE_FOLDER}SequencerKeyframes/SequencerKeyframe.h
    ${ENGINE_FOLDER}SequencerKeyframes/SequencerSpriteKeyframe.cpp
    ${ENGINE_FOLDER}SequencerKeyframes/SequencerSpriteKeyframe.h
    ${ENGINE_FOLDER}SequencerKeyframes/SequencerTransformKeyframe.cpp
    ${ENGINE_FOLDER}SequencerKeyframes/SequencerTransformKeyframe.h
    ${ENGINE_FOLDER}Threading/ThreadManager.cpp
    ${ENGINE_FOLDER}Threading/ThreadManager.h
    ${ENGINE_FOLDER}Engine.cpp
    ${ENGINE_FOLDER}Engine.h
    ${ENGINE_FOLDER}Havtorn.h
    ${ENGINE_FOLDER}hvpch.cpp
    ${ENGINE_FOLDER}hvpch.h
    "${ENGINE_FOLDER}Log.cpp"
    "${ENGINE_FOLDER}Log.h"
    ${ENGINE_FOLDER}ModelImporter.cpp
    ${ENGINE_FOLDER}ModelImporter.h
    ${ENGINE_FOLDER}Timer.cpp
    ${ENGINE_FOLDER}Timer.h
)

# shaders
set(SHADER_FOLDER "Engine/Graphics/Shaders/")

set(SHADER_INCLUDES 
    ${SHADER_FOLDER}Includes/DecalShaderStructs.hlsli
    ${SHADER_FOLDER}Includes/DeferredSamplingFunctions.hlsli
    ${SHADER_FOLDER}Includes/DeferredShaderStructs.hlsli
    ${SHADER_FOLDER}Includes/DetailNormalHelpers.hlsli
    ${SHADER_FOLDER}Includes/FullscreenShaderStructs.hlsli
    ${SHADER_FOLDER}Includes/LineShaderStructs.hlsli
    ${SHADER_FOLDER}Includes/MathHelpers.hlsli
    ${SHADER_FOLDER}Includes/PBRAmbientLighting.hlsli
    ${SHADER_FOLDER}Includes/PBRDirectLighting.hlsli
    ${SHADER_FOLDER}Includes/PointLightShaderStructs.hlsli
    ${SHADER_FOLDER}Includes/ShadowSampling.hlsli
    ${SHADER_FOLDER}Includes/SpotLightShaderStructs.hlsli
    ${SHADER_FOLDER}Includes/SpriteShaderStructs.hlsli
    ${SHADER_FOLDER}Includes/VolumetricLightShaderStructs.hlsli
)
set(VERTEX_SHADERS 
    ${SHADER_FOLDER}Decal_VS.hlsl
    ${SHADER_FOLDER}DeferredInstancedAnimation_VS.hlsl
    ${SHADER_FOLDER}DeferredInstancedAnimationEditor_VS.hlsl
    ${SHADER_FOLDER}DeferredInstancedMesh_VS.hlsl
    ${SHADER_FOLDER}DeferredInstancedMeshEditor_VS.hlsl
    ${SHADER_FOLDER}DeferredStaticMesh_VS.hlsl
    ${SHADER_FOLDER}DeferredVertexPaint_VS.hlsl
    ${SHADER_FOLDER}EditorPreview_VS.hlsl
    ${SHADER_FOLDER}FullscreenVertexShader_VS.hlsl
    ${SHADER_FOLDER}Line_VS.hlsl
    ${SHADER_FOLDER}PointLight_VS.hlsl
    ${SHADER_FOLDER}SpriteInstanced_VS.hlsl
    ${SHADER_FOLDER}SpriteInstancedEditor_VS.hlsl
)
set(GEOMETRY_SHADERS 
    ${SHADER_FOLDER}Line_GS.hlsl
    ${SHADER_FOLDER}SpriteScreenSpace_GS.hlsl
    ${SHADER_FOLDER}SpriteWorldSpace_GS.hlsl
    ${SHADER_FOLDER}SpriteWorldSpaceEditor_GS.hlsl
)
set(PIXEL_SHADERS 
    ${SHADER_FOLDER}Decal_Albedo_PS.hlsl
    ${SHADER_FOLDER}Decal_Material_PS.hlsl
    ${SHADER_FOLDER}Decal_Normal_PS.hlsl
    ${SHADER_FOLDER}DeferredLightDirectionalAndEnvironment_PS.hlsl
    ${SHADER_FOLDER}DeferredLightDirectionalVolumetric_PS.hlsl
    ${SHADER_FOLDER}DeferredLightPoint_PS.hlsl
    ${SHADER_FOLDER}DeferredLightPointVolumetric_PS.hlsl
    ${SHADER_FOLDER}DeferredLightSpot_PS.hlsl
    ${SHADER_FOLDER}DeferredLightSpotVolumetric_PS.hlsl
    ${SHADER_FOLDER}DeferredRenderPassAlbedo_PS.hlsl
    ${SHADER_FOLDER}DeferredRenderPassAO_PS.hlsl
    ${SHADER_FOLDER}DeferredRenderPassEmissive_PS.hlsl
    ${SHADER_FOLDER}DeferredRenderPassFullscreenGammaCorrection_PS.hlsl
    ${SHADER_FOLDER}DeferredRenderPassGBuffer_PS.hlsl
    ${SHADER_FOLDER}DeferredRenderPassMetalness_PS.hlsl
    ${SHADER_FOLDER}DeferredRenderPassNormal_PS.hlsl
    ${SHADER_FOLDER}DeferredRenderPassRoughness_PS.hlsl
    ${SHADER_FOLDER}DeferredVertexPaint_PS.hlsl
    ${SHADER_FOLDER}EditorPreview_PS.hlsl
    ${SHADER_FOLDER}FullscreenBilateralBlurHorizontal_PS.hlsl
    ${SHADER_FOLDER}FullscreenBilateralBlurVertical_PS.hlsl
    ${SHADER_FOLDER}FullscreenBloom_PS.hlsl
    ${SHADER_FOLDER}FullscreenCopy_PS.hlsl
    ${SHADER_FOLDER}FullscreenCopyDepth_PS.hlsl
    ${SHADER_FOLDER}FullscreenCopyGBuffer_PS.hlsl
    ${SHADER_FOLDER}FullscreenDepthAwareUpsample_PS.hlsl
    ${SHADER_FOLDER}FullscreenDepthDownSample_PS.hlsl
    ${SHADER_FOLDER}FullscreenDifference_PS.hlsl
    ${SHADER_FOLDER}FullscreenEditorData_PS.hlsl
    ${SHADER_FOLDER}FullscreenFXAA_PS.hlsl
    ${SHADER_FOLDER}FullscreenGammaCorrection_PS.hlsl
    ${SHADER_FOLDER}FullscreenGaussianBlurHorizontal_PS.hlsl
    ${SHADER_FOLDER}FullscreenGaussianBlurVertical_PS.hlsl
    ${SHADER_FOLDER}FullscreenLuminance_PS.hlsl
    ${SHADER_FOLDER}FullscreenMultiply_PS.hlsl
    ${SHADER_FOLDER}FullscreenSSAO_PS.hlsl
    ${SHADER_FOLDER}FullscreenSSAOBlur_PS.hlsl
    ${SHADER_FOLDER}FullscreenTonemap_PS.hlsl
    ${SHADER_FOLDER}FullscreenVignette_PS.hlsl
    ${SHADER_FOLDER}GBuffer_PS.hlsl
    ${SHADER_FOLDER}GBufferEditor_PS.hlsl
    ${SHADER_FOLDER}Line_PS.hlsl
    ${SHADER_FOLDER}SpriteScreenSpace_PS.hlsl
    ${SHADER_FOLDER}SpriteWorldSpace_PS.hlsl
    ${SHADER_FOLDER}SpriteWorldSpaceEditor_PS.hlsl
    ${SHADER_FOLDER}SpriteWorldSpaceEditorWidget_PS.hlsl
)
set_source_files_properties(${VERTEX_SHADERS} PROPERTIES VS_SHADER_TYPE Vertex)
set_source_files_properties(${GEOMETRY_SHADERS} PROPERTIES VS_SHADER_TYPE Geometry)
set_source_files_properties(${PIXEL_SHADERS} PROPERTIES VS_SHADER_TYPE Pixel)
set(HLSL_SHADER_FILES
    ${VERTEX_SHADERS}
    ${GEOMETRY_SHADERS}
    ${PIXEL_SHADERS}
)
set_source_files_properties(${HLSL_SHADER_FILES} PROPERTIES VS_SHADER_MODEL 5.0)
foreach(FILE ${HLSL_SHADER_FILES})
  get_filename_component(FILE_NAME ${FILE} NAME_WE)
  set_property(SOURCE ${FILE} PROPERTY VS_SHADER_OBJECT_FILE_NAME ${WORKING_FOLDER}Shaders/${FILE_NAME}.cso)
endforeach(FILE)
# !shaders

add_library(Engine SHARED ${ENGINE_FILES} ${SHADER_INCLUDES} ${HLSL_SHADER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SHADER_INCLUDES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HLSL_SHADER_FILES})
target_link_libraries(Engine PRIVATE
    assimp-vc143-mtd
    DirectXTex
    box2dd
    PhysX/PhysX_64
    PhysX/PhysXCommon_64
    PhysX/PhysXExtensions_static_64
    PhysX/PhysXFoundation_64
    PhysX/PhysXPvdSDK_static_64
    PhysX/PhysXCharacterKinematic_static_64
    GUI
)
target_include_directories(Engine PRIVATE
    "${ENGINE_FOLDER}"
    "${EXTERNAL_FOLDER}"
    "${EXTERNAL_FOLDER}assimp/include"
    "${EXTERNAL_FOLDER}FastNoise2/include"
    "${EXTERNAL_FOLDER}rapidjson"
    "${EXTERNAL_FOLDER}DirectXTex"
    "${EXTERNAL_FOLDER}box2d/include/box2d"
    "${EXTERNAL_FOLDER}box2dcpp/include/box2cpp"
    "${EXTERNAL_FOLDER}PhysX/physx/include"
    "${EXTERNAL_FOLDER}imgui"
    "${EXTERNAL_FOLDER}ImGuizmo"
    "GUI"
)
target_link_directories(Engine PRIVATE
    "${EXTERNAL_FOLDER}Lib"
)
target_compile_definitions(Engine PRIVATE ${COMMON_COMPILE_DEFINITIONS} HV_BUILD_DLL)
target_compile_options(Engine PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_options(Engine PRIVATE "/FI${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Engine.dir/$<CONFIG>/cmake_pch.hxx")
target_precompile_headers(Engine PRIVATE ${ENGINE_FOLDER}hvpch.h)
target_link_options(Engine PRIVATE ${COMMON_LINK_OPTIONS} /NODEFAULTLIB:LIBCMTD.lib)
#target_link_options(Engine PRIVATE ${COMMON_LINK_OPTIONS} /NODEFAULTLIB:LIBCMTD.lib /IMPLIB:${CMAKE_CURRENT_BINARY_DIR}/Engine.dir/Engine/Engine_$<CONFIG>.lib)
set_property(TARGET Engine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${WORKING_FOLDER})
set_target_properties(Engine PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${WORKING_FOLDER} RUNTIME_OUTPUT_DIRECTORY_RELEASE ${WORKING_FOLDER})
add_dependencies(Engine GUI)
# ==================== ENGINE ====================


# ==================== GAME ====================
set(GAME_FILES
    ${GAME_FOLDER}Ghosty/GhostyComponent.h
    ${GAME_FOLDER}Ghosty/GhostyComponentEditorContext.cpp
    ${GAME_FOLDER}Ghosty/GhostyComponentEditorContext.h
    ${GAME_FOLDER}Ghosty/GhostySystem.cpp
    ${GAME_FOLDER}Ghosty/GhostySystem.h
    ${GAME_FOLDER}GameManager.cpp
    ${GAME_FOLDER}GameManager.h
    ${GAME_FOLDER}GameProcess.cpp
    ${GAME_FOLDER}GameProcess.h
    ${GAME_FOLDER}GameScene.cpp
    ${GAME_FOLDER}GameScene.h
    ${GAME_FOLDER}hvpch.cpp
    ${GAME_FOLDER}hvpch.h
)
add_library(Game SHARED ${GAME_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${GAME_FILES})
target_include_directories(Game PRIVATE
    "${GAME_FOLDER}"
    "${EXTERNAL_FOLDER}"
    "${EXTERNAL_FOLDER}FastNoise2/include"
    "${EXTERNAL_FOLDER}rapidjson"
    "${EXTERNAL_FOLDER}DirectXTex"
    "${EXTERNAL_FOLDER}box2d/include/box2d"
    "${EXTERNAL_FOLDER}box2dcpp/include/box2cpp"
    "${EXTERNAL_FOLDER}PhysX/physx/include"
    "Engine"
    "GUI"
)
target_link_libraries(Game PRIVATE
    Engine
    GUI
)
target_compile_definitions(Game PRIVATE ${COMMON_COMPILE_DEFINITIONS})
target_compile_options(Game PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_options(Game PRIVATE "/FI${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Game.dir/$<CONFIG>/cmake_pch.hxx")
target_precompile_headers(Game PRIVATE ${GAME_FOLDER}hvpch.h)
target_link_options(Game PRIVATE ${COMMON_LINK_OPTIONS})
set_property(TARGET Game PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${WORKING_FOLDER})
set_target_properties(Game PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${WORKING_FOLDER} RUNTIME_OUTPUT_DIRECTORY_RELEASE ${WORKING_FOLDER})
add_dependencies(Game Engine GUI)
# ==================== GAME ====================


# ==================== EDITOR ====================
set(EDITOR_FILES
    ${EDITOR_FOLDER}Systems/PickingSystem.cpp
    ${EDITOR_FOLDER}Systems/PickingSystem.h
    ${EDITOR_FOLDER}Toggleables/EditMenu.cpp
    ${EDITOR_FOLDER}Toggleables/EditMenu.h
    ${EDITOR_FOLDER}Toggleables/FileMenu.cpp
    ${EDITOR_FOLDER}Toggleables/FileMenu.h
    ${EDITOR_FOLDER}Toggleables/HelpMenu.cpp
    ${EDITOR_FOLDER}Toggleables/HelpMenu.h
    ${EDITOR_FOLDER}Toggleables/ViewMenu.cpp
    ${EDITOR_FOLDER}Toggleables/ViewMenu.h
    ${EDITOR_FOLDER}Toggleables/WindowMenu.cpp
    ${EDITOR_FOLDER}Toggleables/WindowMenu.h
    ${EDITOR_FOLDER}Utils/ImGuiUtils.h
    ${EDITOR_FOLDER}Windows/AssetBrowserWindow.cpp
    ${EDITOR_FOLDER}Windows/AssetBrowserWindow.h
    ${EDITOR_FOLDER}Windows/DockSpaceWindow.cpp
    ${EDITOR_FOLDER}Windows/DockSpaceWindow.h
    ${EDITOR_FOLDER}Windows/HierarchyWindow.cpp
    ${EDITOR_FOLDER}Windows/HierarchyWindow.h
    ${EDITOR_FOLDER}Windows/InspectorWindow.cpp
    ${EDITOR_FOLDER}Windows/InspectorWindow.h
    ${EDITOR_FOLDER}Windows/OutputLogWindow.cpp
    ${EDITOR_FOLDER}Windows/OutputLogWindow.h
    ${EDITOR_FOLDER}Windows/SequencerWindow.cpp
    ${EDITOR_FOLDER}Windows/SequencerWindow.h
    ${EDITOR_FOLDER}Windows/SpriteAnimatorGraphNodeWindow.cpp
    ${EDITOR_FOLDER}Windows/SpriteAnimatorGraphNodeWindow.h
    ${EDITOR_FOLDER}Windows/ViewportWindow.cpp
    ${EDITOR_FOLDER}Windows/ViewportWindow.h
    ${EDITOR_FOLDER}EditorManager.cpp
    ${EDITOR_FOLDER}EditorManager.h
    ${EDITOR_FOLDER}EditorProcess.cpp
    ${EDITOR_FOLDER}EditorProcess.h
    ${EDITOR_FOLDER}EditorResourceManager.cpp
    ${EDITOR_FOLDER}EditorResourceManager.h
    ${EDITOR_FOLDER}EditorToggleable.cpp
    ${EDITOR_FOLDER}EditorToggleable.h
    ${EDITOR_FOLDER}EditorToggleables.h
    ${EDITOR_FOLDER}EditorWindow.cpp
    ${EDITOR_FOLDER}EditorWindow.h
    ${EDITOR_FOLDER}EditorWindows.h
)
add_library(Editor SHARED ${EDITOR_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EDITOR_FILES})
target_include_directories(Editor PRIVATE
    "${EDITOR_FOLDER}"
    "${EXTERNAL_FOLDER}rapidjson"
    "${EXTERNAL_FOLDER}DirectXTex"
    "${EXTERNAL_FOLDER}box2d/include/box2d"
    "${EXTERNAL_FOLDER}box2dcpp/include/box2cpp"
    "${EXTERNAL_FOLDER}PhysX/physx/include"
    "Engine"
    "GUI"
)
target_link_libraries(Editor PRIVATE
    Engine
    Game
    GUI
)
target_compile_definitions(Editor PRIVATE ${COMMON_COMPILE_DEFINITIONS})
target_compile_options(Editor PRIVATE ${COMMON_COMPILE_OPTIONS})
target_link_options(Editor PRIVATE ${COMMON_LINK_OPTIONS})
set_property(TARGET Editor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${WORKING_FOLDER})
set_target_properties(Editor PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${WORKING_FOLDER} RUNTIME_OUTPUT_DIRECTORY_RELEASE ${WORKING_FOLDER})
add_dependencies(Editor Engine Game GUI)
# ==================== EDITOR ====================


# ==================== LAUNCHER ====================
set(LAUNCHER_FILES
    ${LAUNCHER_FOLDER}Application/Application.cpp
    ${LAUNCHER_FOLDER}Application/Application.h
    ${LAUNCHER_FOLDER}Application/Process.h
    ${LAUNCHER_FOLDER}Main.cpp
)
add_executable(Launcher WIN32 ${LAUNCHER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LAUNCHER_FILES})
target_include_directories(Launcher PRIVATE
	"${LAUNCHER_FOLDER}"
    "${EXTERNAL_FOLDER}box2d/include/box2d"
	"${EXTERNAL_FOLDER}box2dcpp/include/box2cpp"
	"${EXTERNAL_FOLDER}PhysX/physx/include"
	"GUI"
	"Editor"
	"Engine"
	"Game"
)
target_link_libraries(Launcher PRIVATE
    Engine
    Game
    Editor
    GUI
)
target_compile_definitions(Launcher PRIVATE ${COMMON_COMPILE_DEFINITIONS})
target_compile_options(Launcher PRIVATE ${COMMON_COMPILE_OPTIONS})
target_link_options(Launcher PRIVATE ${COMMON_LINK_OPTIONS})
set_property(TARGET Launcher PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${WORKING_FOLDER})
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Launcher)
set_target_properties(Launcher PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${WORKING_FOLDER} RUNTIME_OUTPUT_DIRECTORY_RELEASE ${WORKING_FOLDER})
add_dependencies(Launcher Engine Editor Game GUI)
# ==================== LAUNCHER ====================
