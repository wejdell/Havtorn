cmake_minimum_required (VERSION 3.31)

set(CMAKE_CONFIGURATION_TYPES GameDebug GameRelease EditorDebug EditorDevelopment)

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

# ==================== FOLDERS ====================
set(ENGINE_FOLDER "Engine/")
set(SHADER_FOLDER "Engine/Graphics/Shaders/")
set(SHADER_INCL_FOLDER "Engine/Graphics/Shaders/Includes/")
set(EDITOR_FOLDER "Editor/")
set(GAME_FOLDER "Game/")
set(GUI_FOLDER "GUI/")
set(CORE_FOLDER "Core/")
set(PLATFORM_FOLDER "Platform/")
set(LAUNCHER_FOLDER "Launcher/")
set(EXTERNAL_FOLDER "../External/")
set(IMGUI_FOLDER "../External/imgui/")
set(IMGUIZMO_FOLDER "../External/ImGuizmo/")
set(IMGUI_NODE_FOLDER "../External/imgui-node-editor/")
set(WORKING_FOLDER "${CMAKE_BINARY_DIR}/../Bin/")
# ==================== FOLDERS ====================

#TODO: Define macro like HV_CONFIG and set that to one of the configs somehow? 
set(COMMON_COMPILE_DEFINITIONS 
HV_PLATFORM_WINDOWS 
$<$<CONFIG:EditorDebug>:HV_EDITOR_BUILD HV_CONFIG_EDITOR_DEBUG _DEBUG HV_DEBUG_INFO HV_ENABLE_DEBUG HV_ENABLE_ASSERTS _ITERATOR_DEBUG_LEVEL=2> 
$<$<CONFIG:GameDebug>:HV_GAME_BUILD HV_CONFIG_GAME_DEBUG _DEBUG HV_DEBUG_INFO HV_ENABLE_DEBUG HV_ENABLE_ASSERTS _ITERATOR_DEBUG_LEVEL=2> 
$<$<CONFIG:EditorDevelopment>:HV_EDITOR_BUILD HV_CONFIG_EDITOR_DEVELOPMENT NDEBUG HV_ENABLE_DEBUG _ITERATOR_DEBUG_LEVEL=0> 
$<$<CONFIG:GameRelease>:HV_GAME_BUILD HV_CONFIG_GAME_RELEASE NDEBUG _ITERATOR_DEBUG_LEVEL=0> 
_UNICODE 
UNICODE
)
#TODO: Look closely at Use Debug Libraries
set(COMMON_COMPILE_OPTIONS
$<$<CONFIG:EditorDebug>:/W4 /WX /MP /ZI /JMC /fp:fast "/MDd" /FC>
$<$<CONFIG:GameDebug>:/W4 /WX /MP /ZI /JMC /fp:fast "/MDd" /FC>
$<$<CONFIG:EditorDevelopment>:/W4 /WX /MP /fp:fast "/MD" /FC /GL /O2 /Oi>
$<$<CONFIG:GameRelease>:/W4 /WX /MP /fp:fast "/MD" /FC /GL /O2 /Oi>
)
set(COMMON_LINK_OPTIONS
/WX /SUBSYSTEM:WINDOWS
$<$<CONFIG:EditorDebug>:/WX /SUBSYSTEM:WINDOWS /DEBUG>
$<$<CONFIG:GameDebug>:/WX /SUBSYSTEM:WINDOWS /DEBUG>
$<$<CONFIG:EditorDevelopment>:/WX /SUBSYSTEM:WINDOWS "/INCREMENTAL:NO" /OPT:REF /OPT:ICF /LTCG:incremental>
$<$<CONFIG:GameRelease>:/WX /SUBSYSTEM:WINDOWS "/INCREMENTAL:NO" /OPT:REF /OPT:ICF /LTCG:incremental>
)
set(COMMON_TARGET_PROPERTIES
RUNTIME_OUTPUT_DIRECTORY_EDITORDEBUG ${WORKING_FOLDER}/$<CONFIG>
RUNTIME_OUTPUT_DIRECTORY_EDITORDEVELOPMENT ${WORKING_FOLDER}/$<CONFIG>
RUNTIME_OUTPUT_DIRECTORY_GAMEDEBUG ${WORKING_FOLDER}/$<CONFIG>
RUNTIME_OUTPUT_DIRECTORY_GAMERELEASE ${WORKING_FOLDER}/$<CONFIG>
)

# ==================== CORE ====================
set(CORE_FILES
    ${CORE_FOLDER}MathTypes/AABB3D.h
    ${CORE_FOLDER}MathTypes/Curve.cpp
    ${CORE_FOLDER}MathTypes/Curve.h
    ${CORE_FOLDER}MathTypes/EngineMath.cpp
    ${CORE_FOLDER}MathTypes/EngineMath.h
    ${CORE_FOLDER}MathTypes/EngineMathSSE.h
    ${CORE_FOLDER}MathTypes/Frustum.cpp
    ${CORE_FOLDER}MathTypes/Frustum.h
    ${CORE_FOLDER}MathTypes/HavtornMath.h
    ${CORE_FOLDER}MathTypes/Intersection.h
    ${CORE_FOLDER}MathTypes/Line.h
    ${CORE_FOLDER}MathTypes/LineVolume.h
    ${CORE_FOLDER}MathTypes/MathUtilities.h
    ${CORE_FOLDER}MathTypes/Matrix.cpp
    ${CORE_FOLDER}MathTypes/Matrix.h
    ${CORE_FOLDER}MathTypes/Matrix3x3.h
    ${CORE_FOLDER}MathTypes/Plane.h
    ${CORE_FOLDER}MathTypes/PlaneVolume.h
    ${CORE_FOLDER}MathTypes/Quaternion.cpp
    ${CORE_FOLDER}MathTypes/Quaternion.h
    ${CORE_FOLDER}MathTypes/Ray.h
    ${CORE_FOLDER}MathTypes/Rotator.h
    ${CORE_FOLDER}MathTypes/Sphere.h
    ${CORE_FOLDER}MathTypes/Transform.cpp
    ${CORE_FOLDER}MathTypes/Transform.h
    ${CORE_FOLDER}MathTypes/Vector.cpp
    ${CORE_FOLDER}MathTypes/Vector.h
    ${CORE_FOLDER}BitSet.h
    ${CORE_FOLDER}Color.cpp
    ${CORE_FOLDER}Color.h
    ${CORE_FOLDER}CommandLine.cpp
    ${CORE_FOLDER}CommandLine.h
    ${CORE_FOLDER}Core.h
    ${CORE_FOLDER}CoreTypes.h
    ${CORE_FOLDER}Delegate.cpp
    ${CORE_FOLDER}Delegate.h
    ${CORE_FOLDER}EngineException.h
    ${CORE_FOLDER}EngineTypes.h
    ${CORE_FOLDER}FileSystem.cpp
    ${CORE_FOLDER}FileSystem.h
    ${CORE_FOLDER}GeneralUtilities.h
    ${CORE_FOLDER}HavtornDelegate.h
    ${CORE_FOLDER}HavtornString.cpp
    ${CORE_FOLDER}HavtornString.h
    ${CORE_FOLDER}Serializable.cpp
    ${CORE_FOLDER}Serializable.h
    ${CORE_FOLDER}hvpch.cpp
    ${CORE_FOLDER}hvpch.h
    "${CORE_FOLDER}Log.cpp"
    "${CORE_FOLDER}Log.h"
    ${CORE_FOLDER}magic_enum.h
)
set(RAPIDJSON_FILES
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/error/en.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/error/error.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/internal/biginteger.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/internal/clzll.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/internal/diyfp.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/internal/dtoa.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/internal/ieee754.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/internal/itoa.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/internal/meta.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/internal/pow10.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/internal/regex.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/internal/stack.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/internal/strfunc.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/internal/strtod.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/internal/swap.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/msinttypes/inttypes.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/msinttypes/stdint.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/allocators.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/cursorstreamwrapper.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/document.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/encodedstream.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/encodings.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/filereadstream.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/filewritestream.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/fwd.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/istreamwrapper.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/memorybuffer.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/memorystream.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/ostreamwrapper.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/pointer.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/prettywriter.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/rapidjson.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/reader.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/schema.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/stream.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/stringbuffer.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/uri.h"
    "${EXTERNAL_FOLDER}rapidjson/include/rapidjson/writer.h"
)
add_library(Core SHARED ${CORE_FILES} ${RAPIDJSON_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CORE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}"/../External/rapidjson/include" FILES ${RAPIDJSON_FILES})
target_include_directories(Core PRIVATE
    "${CORE_FOLDER}"
    "${EXTERNAL_FOLDER}rapidjson/include"
)
target_compile_definitions(Core PRIVATE ${COMMON_COMPILE_DEFINITIONS} HV_CORE_DLL)
target_compile_options(Core PRIVATE ${COMMON_COMPILE_OPTIONS})
target_link_options(Core PRIVATE ${COMMON_LINK_OPTIONS})
target_precompile_headers(Core PRIVATE ${CORE_FOLDER}hvpch.h)
set_property(TARGET Core PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${WORKING_FOLDER})
set_target_properties(Core PROPERTIES ${COMMON_TARGET_PROPERTIES})
# ==================== CORE ====================

# ==================== PLATFORM ====================
set(PLATFORM_FILES
    ${PLATFORM_FOLDER}PlatformProcess.cpp
    ${PLATFORM_FOLDER}PlatformProcess.h
    ${PLATFORM_FOLDER}PlatformManager.cpp
    ${PLATFORM_FOLDER}PlatformManager.h
    ${PLATFORM_FOLDER}PlatformUtilities.cpp
    ${PLATFORM_FOLDER}PlatformUtilities.h
    ${PLATFORM_FOLDER}WindowsInclude.h
)
add_library(Platform SHARED ${PLATFORM_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PLATFORM_FILES})
target_link_libraries(Platform PRIVATE
    Core
)
target_include_directories(Platform PRIVATE
	"${PLATFORM_FOLDER}"
    "${CORE_FOLDER}"
    "${EXTERNAL_FOLDER}rapidjson/include"
)
target_link_directories(Platform PRIVATE
    ${CMAKE_BINARY_DIR}
)
target_compile_definitions(Platform PRIVATE ${COMMON_COMPILE_DEFINITIONS} HV_PLATFORM_DLL)
target_compile_options(Platform PRIVATE ${COMMON_COMPILE_OPTIONS})
target_link_options(Platform PRIVATE ${COMMON_LINK_OPTIONS})
set_property(TARGET Platform PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${WORKING_FOLDER})
set_target_properties(Platform PROPERTIES ${COMMON_TARGET_PROPERTIES})
add_dependencies(Platform Core)
# ==================== PLATFORM ====================

# ==================== GUI ====================
set(GUI_FILES
   ${GUI_FOLDER}GUI.cpp
   ${GUI_FOLDER}GUI.h
   ${GUI_FOLDER}GUIProcess.cpp
   ${GUI_FOLDER}GUIProcess.h
   ${GUI_FOLDER}FuzzySearch.cpp
   ${GUI_FOLDER}FuzzySearch.h
)
set(IMGUI_FILES
   ${IMGUI_FOLDER}imconfig.h
   ${IMGUI_FOLDER}imgui.cpp
   ${IMGUI_FOLDER}imgui.h
   ${IMGUI_FOLDER}imgui_demo.cpp
   ${IMGUI_FOLDER}imgui_draw.cpp
   ${IMGUI_FOLDER}imgui_internal.h
   ${IMGUI_FOLDER}imgui_tables.cpp
   ${IMGUI_FOLDER}imgui_widgets.cpp
   ${IMGUI_FOLDER}imstb_rectpack.h
   ${IMGUI_FOLDER}imstb_textedit.h
   ${IMGUI_FOLDER}imstb_truetype.h
   ${IMGUI_FOLDER}backends/imgui_impl_dx11.cpp
   ${IMGUI_FOLDER}backends/imgui_impl_dx11.h
   ${IMGUI_FOLDER}backends/imgui_impl_win32.cpp
   ${IMGUI_FOLDER}backends/imgui_impl_win32.h
   ${IMGUI_FOLDER}misc/debuggers/imgui.natvis
   ${IMGUI_FOLDER}misc/debuggers/imgui.natstepfilter
   ${IMGUI_FOLDER}misc/cpp/imgui_stdlib.cpp
   ${IMGUI_FOLDER}misc/cpp/imgui_stdlib.h
   ${IMGUIZMO_FOLDER}GraphEditor.cpp
   ${IMGUIZMO_FOLDER}GraphEditor.h
   ${IMGUIZMO_FOLDER}ImCurveEdit.cpp
   ${IMGUIZMO_FOLDER}ImCurveEdit.h
   ${IMGUIZMO_FOLDER}ImCurveEdit.cpp
   ${IMGUIZMO_FOLDER}ImCurveEdit.h
   ${IMGUIZMO_FOLDER}ImGradient.cpp
   ${IMGUIZMO_FOLDER}ImGradient.h
   ${IMGUIZMO_FOLDER}ImGuizmo.cpp
   ${IMGUIZMO_FOLDER}ImGuizmo.h
   ${IMGUIZMO_FOLDER}ImSequencer.cpp
   ${IMGUIZMO_FOLDER}ImSequencer.h
   ${IMGUIZMO_FOLDER}ImZoomSlider.h
   ${IMGUI_NODE_FOLDER}crude_json.cpp
   ${IMGUI_NODE_FOLDER}crude_json.h
   ${IMGUI_NODE_FOLDER}imgui_bezier_math.h
   ${IMGUI_NODE_FOLDER}imgui_canvas.cpp
   ${IMGUI_NODE_FOLDER}imgui_canvas.h
   ${IMGUI_NODE_FOLDER}imgui_extra_math.h
   ${IMGUI_NODE_FOLDER}imgui_node_editor.cpp
   ${IMGUI_NODE_FOLDER}imgui_node_editor.h
   ${IMGUI_NODE_FOLDER}imgui_node_editor.h
   ${IMGUI_NODE_FOLDER}imgui_node_editor_api.cpp
   ${IMGUI_NODE_FOLDER}imgui_node_editor_internal.h
   ${IMGUI_NODE_FOLDER}utilities/builders.cpp
   ${IMGUI_NODE_FOLDER}utilities/builders.h
   ${IMGUI_NODE_FOLDER}utilities/drawing.cpp
   ${IMGUI_NODE_FOLDER}utilities/drawing.h
   ${IMGUI_NODE_FOLDER}utilities/widgets.cpp
   ${IMGUI_NODE_FOLDER}utilities/widgets.h
)
add_library(GUI SHARED ${GUI_FILES} ${IMGUI_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${GUI_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}"/../External" FILES ${IMGUI_FILES})
target_link_libraries(GUI PRIVATE
    Core
    Platform
    $<$<CONFIG:EditorDebug>:DirectXTexd>
    $<$<CONFIG:GameDebug>:DirectXTexd>
    $<$<CONFIG:EditorDevelopment>:DirectXTex>
    $<$<CONFIG:GameRelease>:DirectXTex>
)
target_include_directories(GUI PRIVATE
	"${GUI_FOLDER}"
    "${PLATFORM_FOLDER}"
	"${CORE_FOLDER}"
    "${EXTERNAL_FOLDER}imgui"
    "${EXTERNAL_FOLDER}ImGuizmo"
    "${EXTERNAL_FOLDER}imgui-node-editor"
    "${EXTERNAL_FOLDER}DirectXTex"
)
target_link_directories(GUI PRIVATE
    "${EXTERNAL_FOLDER}Lib/Debug"
    "${EXTERNAL_FOLDER}Lib/Release"
    ${CMAKE_BINARY_DIR}
)
target_compile_definitions(GUI PRIVATE ${COMMON_COMPILE_DEFINITIONS} HV_GUI_DLL)
target_compile_options(GUI PRIVATE 
$<$<CONFIG:EditorDebug>:/W3 /MP /ZI /JMC /fp:fast "/MDd" /FC>
$<$<CONFIG:GameDebug>:/W3 /MP /ZI /JMC /fp:fast "/MDd" /FC>
$<$<CONFIG:EditorDevelopment>:/W3 /MP /fp:fast "/MD" /FC /GL /O2 /Oi>
$<$<CONFIG:GameRelease>:/W3 /MP /fp:fast "/MD" /FC /GL /O2 /Oi>
)
target_link_options(GUI PRIVATE ${COMMON_LINK_OPTIONS})
set_property(TARGET GUI PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${WORKING_FOLDER})
set_target_properties(GUI PROPERTIES ${COMMON_TARGET_PROPERTIES})
add_dependencies(GUI Core Platform)
# ==================== GUI ====================

# ==================== ENGINE ====================
set(ENGINE_FILES
	Engine/ECS/Systems/UISystem.cpp
	Engine/ECS/Systems/UISystem.h
	Engine/ECS/ComponentEditorContexts/UICanvasComponentEditorContext.cpp
	Engine/ECS/ComponentEditorContexts/UICanvasComponentEditorContext.h
	Engine/ECS/Components/UICanvasComponent.cpp
	Engine/ECS/Components/UICanvasComponent.h
    ${ENGINE_FOLDER}Application/EngineProcess.cpp
    ${ENGINE_FOLDER}Application/EngineProcess.h
    ${ENGINE_FOLDER}Assets/AssetRegistry.cpp
    ${ENGINE_FOLDER}Assets/AssetRegistry.h
    ${ENGINE_FOLDER}Assets/FileHeaderDeclarations.h
    ${ENGINE_FOLDER}Assets/RuntimeAssetDeclarations.h
    ${ENGINE_FOLDER}Assets/SequencerAsset.cpp
    ${ENGINE_FOLDER}Assets/SequencerAsset.h
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
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/ScriptComponentEditorContext.cpp
    ${ENGINE_FOLDER}ECS/ComponentEditorContexts/ScriptComponentEditorContext.h
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
    ${ENGINE_FOLDER}ECS/Components/EnvironmentLightComponent.cpp
    ${ENGINE_FOLDER}ECS/Components/EnvironmentLightComponent.h
    ${ENGINE_FOLDER}ECS/Components/MaterialComponent.cpp
    ${ENGINE_FOLDER}ECS/Components/MaterialComponent.h
    ${ENGINE_FOLDER}ECS/Components/MetaDataComponent.h
    ${ENGINE_FOLDER}ECS/Components/Physics2DComponent.h
    ${ENGINE_FOLDER}ECS/Components/Physics3DComponent.h
    ${ENGINE_FOLDER}ECS/Components/Physics3DControllerComponent.h
    ${ENGINE_FOLDER}ECS/Components/PointLightComponent.h
    ${ENGINE_FOLDER}ECS/Components/SequencerComponent.h
    ${ENGINE_FOLDER}ECS/Components/ScriptComponent.cpp
    ${ENGINE_FOLDER}ECS/Components/ScriptComponent.h
    ${ENGINE_FOLDER}ECS/Components/SkeletalAnimationComponent.cpp
    ${ENGINE_FOLDER}ECS/Components/SkeletalAnimationComponent.h
    ${ENGINE_FOLDER}ECS/Components/SkeletalMeshComponent.cpp
    ${ENGINE_FOLDER}ECS/Components/SkeletalMeshComponent.h
    ${ENGINE_FOLDER}ECS/Components/SpotLightComponent.h
    ${ENGINE_FOLDER}ECS/Components/SpriteAnimatorGraphComponent.cpp
    ${ENGINE_FOLDER}ECS/Components/SpriteAnimatorGraphComponent.h
    ${ENGINE_FOLDER}ECS/Components/SpriteAnimatorGraphNode.cpp
    ${ENGINE_FOLDER}ECS/Components/SpriteAnimatorGraphNode.h
    ${ENGINE_FOLDER}ECS/Components/SpriteComponent.cpp
    ${ENGINE_FOLDER}ECS/Components/SpriteComponent.h
    ${ENGINE_FOLDER}ECS/Components/StaticMeshComponent.cpp
    ${ENGINE_FOLDER}ECS/Components/StaticMeshComponent.h
    ${ENGINE_FOLDER}ECS/Components/Transform2DComponent.h
    ${ENGINE_FOLDER}ECS/Components/TransformComponent.cpp
    ${ENGINE_FOLDER}ECS/Components/TransformComponent.h
    ${ENGINE_FOLDER}ECS/Components/VolumetricLightComponent.h
    ${ENGINE_FOLDER}ECS/Systems/AnimatorGraphSystem.cpp
    ${ENGINE_FOLDER}ECS/Systems/AnimatorGraphSystem.h
    ${ENGINE_FOLDER}ECS/Systems/CameraSystem.cpp
    ${ENGINE_FOLDER}ECS/Systems/CameraSystem.h
    ${ENGINE_FOLDER}ECS/Systems/LightSystem.cpp
    ${ENGINE_FOLDER}ECS/Systems/LightSystem.h
    ${ENGINE_FOLDER}ECS/Systems/RenderSystem.cpp
    ${ENGINE_FOLDER}ECS/Systems/RenderSystem.h
    ${ENGINE_FOLDER}ECS/Systems/ScriptSystem.cpp
    ${ENGINE_FOLDER}ECS/Systems/ScriptSystem.h
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
    ${ENGINE_FOLDER}FileSystem/FileWatcher.cpp
    ${ENGINE_FOLDER}FileSystem/FileWatcher.h
    ${ENGINE_FOLDER}Graphics/Debug/DebugDrawUtility.cpp
    ${ENGINE_FOLDER}Graphics/Debug/DebugDrawUtility.h
    ${ENGINE_FOLDER}Graphics/Renderers/FullscreenRenderer.cpp
    ${ENGINE_FOLDER}Graphics/Renderers/FullscreenRenderer.h
    ${ENGINE_FOLDER}Graphics/RenderingPrimitives/DataBuffer.cpp
    ${ENGINE_FOLDER}Graphics/RenderingPrimitives/DataBuffer.h
    ${ENGINE_FOLDER}Graphics/RenderingPrimitives/GBuffer.cpp
    ${ENGINE_FOLDER}Graphics/RenderingPrimitives/GBuffer.h
    ${ENGINE_FOLDER}Graphics/RenderingPrimitives/RenderTexture.cpp
    ${ENGINE_FOLDER}Graphics/RenderingPrimitives/RenderTexture.h
    ${ENGINE_FOLDER}Graphics/GeometryPrimitives.h
    ${ENGINE_FOLDER}Graphics/GraphicsEnums.h
    ${ENGINE_FOLDER}Graphics/GraphicsFramework.cpp
    ${ENGINE_FOLDER}Graphics/GraphicsFramework.h
    ${ENGINE_FOLDER}Graphics/GraphicsMaterial.cpp
    ${ENGINE_FOLDER}Graphics/GraphicsMaterial.h
    ${ENGINE_FOLDER}Graphics/GraphicsStructs.h
    ${ENGINE_FOLDER}Graphics/GraphicsUtilities.cpp
    ${ENGINE_FOLDER}Graphics/GraphicsUtilities.h
    ${ENGINE_FOLDER}Graphics/RenderCommand.h
    ${ENGINE_FOLDER}Graphics/RenderManager.cpp
    ${ENGINE_FOLDER}Graphics/RenderManager.h
    ${ENGINE_FOLDER}Graphics/RenderStateManager.cpp
    ${ENGINE_FOLDER}Graphics/RenderStateManager.h
    ${ENGINE_FOLDER}Graphics/RenderTextureFactory.cpp
    ${ENGINE_FOLDER}Graphics/RenderTextureFactory.h
    ${ENGINE_FOLDER}Graphics/TextureBank.cpp
    ${ENGINE_FOLDER}Graphics/TextureBank.h
    ${ENGINE_FOLDER}HexPhys/HexPhys.cpp
    ${ENGINE_FOLDER}HexPhys/HexPhys.h
    ${ENGINE_FOLDER}HexRune/CoreNodes/CoreNodes.cpp
    ${ENGINE_FOLDER}HexRune/CoreNodes/CoreNodes.h
    ${ENGINE_FOLDER}HexRune/ECSNodes/ECSNodes.cpp
    ${ENGINE_FOLDER}HexRune/ECSNodes/ECSNodes.h
    ${ENGINE_FOLDER}HexRune/NodeEditorContexts/CoreNodeEditorContexts.cpp
    ${ENGINE_FOLDER}HexRune/NodeEditorContexts/CoreNodeEditorContexts.h
    ${ENGINE_FOLDER}HexRune/NodeEditorContexts/ECSNodeEditorContexts.cpp
    ${ENGINE_FOLDER}HexRune/NodeEditorContexts/ECSNodeEditorContexts.h
    ${ENGINE_FOLDER}HexRune/HexRune.cpp
    ${ENGINE_FOLDER}HexRune/HexRune.h
    ${ENGINE_FOLDER}HexRune/NodeEditorContext.h
    ${ENGINE_FOLDER}HexRune/Pin.cpp
    ${ENGINE_FOLDER}HexRune/Pin.h
    "${ENGINE_FOLDER}Input/Input.cpp"
    "${ENGINE_FOLDER}Input/Input.h"
    "${ENGINE_FOLDER}Input/InputMapper.cpp"
    "${ENGINE_FOLDER}Input/InputMapper.h"
    "${ENGINE_FOLDER}Input/InputTypes.h"
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
    ${ENGINE_FOLDER}ModelImporter.cpp
    ${ENGINE_FOLDER}ModelImporter.h
    ${ENGINE_FOLDER}Timer.cpp
    ${ENGINE_FOLDER}Timer.h
)

# ==================== SHADERS ====================
set(SHADER_INCLUDES
    ${SHADER_INCL_FOLDER}DecalShaderStructs.hlsli
    ${SHADER_INCL_FOLDER}DeferredSamplingFunctions.hlsli
    ${SHADER_INCL_FOLDER}DeferredShaderStructs.hlsli
    ${SHADER_INCL_FOLDER}DetailNormalHelpers.hlsli
    ${SHADER_INCL_FOLDER}FullscreenShaderStructs.hlsli
    ${SHADER_INCL_FOLDER}LineShaderStructs.hlsli
    ${SHADER_INCL_FOLDER}MathHelpers.hlsli
    ${SHADER_INCL_FOLDER}PBRAmbientLighting.hlsli
    ${SHADER_INCL_FOLDER}PBRDirectLighting.hlsli
    ${SHADER_INCL_FOLDER}PointLightShaderStructs.hlsli
    ${SHADER_INCL_FOLDER}ShadowSampling.hlsli
    ${SHADER_INCL_FOLDER}SpotLightShaderStructs.hlsli
    ${SHADER_INCL_FOLDER}SpriteShaderStructs.hlsli
    ${SHADER_INCL_FOLDER}VolumetricLightShaderStructs.hlsli
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
    ${SHADER_FOLDER}EditorPreviewSkeletal_VS.hlsl
    ${SHADER_FOLDER}FullscreenVertexShader_VS.hlsl
    ${SHADER_FOLDER}Line_VS.hlsl
    ${SHADER_FOLDER}Skybox_VS.hlsl
    ${SHADER_FOLDER}PointLight_VS.hlsl
    ${SHADER_FOLDER}SpriteInstanced_VS.hlsl
    ${SHADER_FOLDER}SpriteInstancedEditor_VS.hlsl
)
set(GEOMETRY_SHADERS
	Engine/Graphics/Shaders/LineScreenSpace_GS.hlsl
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
    ${SHADER_FOLDER}Skybox_PS.hlsl
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
# ==================== SHADERS ====================

add_library(Engine SHARED ${ENGINE_FILES} ${SHADER_INCLUDES} ${HLSL_SHADER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SHADER_INCLUDES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HLSL_SHADER_FILES})
target_link_libraries(Engine PRIVATE
    $<$<CONFIG:EditorDebug>:assimp-vc143-mtd DirectXTexd d3dcompiler freetyped box2dd PhysX/PhysX_64d PhysX/PhysXCommon_64d PhysX/PhysXExtensions_static_64d PhysX/PhysXFoundation_64d PhysX/PhysXPvdSDK_static_64d PhysX/PhysXCharacterKinematic_static_64d>
    $<$<CONFIG:GameDebug>:assimp-vc143-mtd DirectXTexd d3dcompiler freetyped box2dd PhysX/PhysX_64d PhysX/PhysXCommon_64d PhysX/PhysXExtensions_static_64d PhysX/PhysXFoundation_64d PhysX/PhysXPvdSDK_static_64d PhysX/PhysXCharacterKinematic_static_64d>
    $<$<CONFIG:EditorDevelopment>:assimp-vc143-mt DirectXTex d3dcompiler freetype box2d PhysX/PhysX_64 PhysX/PhysXCommon_64 PhysX/PhysXExtensions_static_64 PhysX/PhysXFoundation_64 PhysX/PhysXPvdSDK_static_64 PhysX/PhysXCharacterKinematic_static_64>
    $<$<CONFIG:GameRelease>:assimp-vc143-mt DirectXTex d3dcompiler freetype box2d PhysX/PhysX_64 PhysX/PhysXCommon_64 PhysX/PhysXExtensions_static_64 PhysX/PhysXFoundation_64 PhysX/PhysXPvdSDK_static_64 PhysX/PhysXCharacterKinematic_static_64>
    Core
    Platform
    GUI
)
target_include_directories(Engine PRIVATE
    "${ENGINE_FOLDER}"
    "${EXTERNAL_FOLDER}"
    "${EXTERNAL_FOLDER}assimp/include"
    "${EXTERNAL_FOLDER}FastNoise2/include"
    "${EXTERNAL_FOLDER}rapidjson/include"
    "${EXTERNAL_FOLDER}freetype/include"
    "${EXTERNAL_FOLDER}DirectXTex"
    "${EXTERNAL_FOLDER}box2d/include/box2d"
    "${EXTERNAL_FOLDER}box2dcpp/include/box2cpp"
    "${EXTERNAL_FOLDER}PhysX/physx/include"
    "Core"
    "Platform"
    "GUI"
)
target_link_directories(Engine PRIVATE
    "${EXTERNAL_FOLDER}Lib/Debug"
    "${EXTERNAL_FOLDER}Lib/Release" 
    ${CMAKE_BINARY_DIR}
)
target_compile_definitions(Engine PRIVATE ${COMMON_COMPILE_DEFINITIONS} HV_ENGINE_DLL)
target_compile_options(Engine PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_options(Engine PRIVATE "/FI${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Engine.dir/$<CONFIG>/cmake_pch.hxx")
target_precompile_headers(Engine PRIVATE ${ENGINE_FOLDER}hvpch.h)
target_link_options(Engine PRIVATE ${COMMON_LINK_OPTIONS} 
$<$<CONFIG:EditorDebug>:/NODEFAULTLIB:LIBCMTD.lib>
$<$<CONFIG:GameDebug>:/NODEFAULTLIB:LIBCMTD.lib>
$<$<CONFIG:EditorDevelopment>:/NODEFAULTLIB:LIBCMT /NODEFAULTLIB:MSVCRTD>
$<$<CONFIG:GameRelease>:/NODEFAULTLIB:LIBCMT /NODEFAULTLIB:MSVCRTD>
)
#target_link_options(Engine PRIVATE ${COMMON_LINK_OPTIONS} /NODEFAULTLIB:LIBCMTD.lib /IMPLIB:${CMAKE_CURRENT_BINARY_DIR}/Engine.dir/Engine/Engine_$<CONFIG>.lib)
set_property(TARGET Engine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${WORKING_FOLDER})
set_target_properties(Engine PROPERTIES ${COMMON_TARGET_PROPERTIES})
add_dependencies(Engine Core Platform GUI)
# ==================== ENGINE ====================


# ==================== GAME ====================
set(GAME_FILES
	Game/HexRune/NodeEditorContext/SetPositionNodeEditorContext.cpp
	Game/HexRune/NodeEditorContext/SetPositionNodeEditorContext.h
	Game/HexRune/SetPositionNode.cpp
	Game/HexRune/SetPositionNode.h
	${GAME_FOLDER}NodeInclude.h
	${GAME_FOLDER}HexRune/NodeEditorContext/TimerNodeEditorContext.cpp
	${GAME_FOLDER}HexRune/NodeEditorContext/TimerNodeEditorContext.h
	${GAME_FOLDER}HexRune/TimerNode.cpp
	${GAME_FOLDER}HexRune/TimerNode.h
	${GAME_FOLDER}HexRune/NodeEditorContext/InterpolatePositionNodeEditorContext.cpp
	${GAME_FOLDER}HexRune/NodeEditorContext/InterpolatePositionNodeEditorContext.h
	${GAME_FOLDER}HexRune/InterpolatePosition.cpp
	${GAME_FOLDER}HexRune/InterpolatePosition.h
	${GAME_FOLDER}Ghosty/SGhostyNodeEditorContext.h
	${GAME_FOLDER}Ghosty/SGhostyNodeEditorContext.cpp
	${GAME_FOLDER}GhostyNode.h
	${GAME_FOLDER}GhostyNode.cpp
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
    ${GAME_FOLDER}GameScript.cpp
    ${GAME_FOLDER}GameScript.h
    ${GAME_FOLDER}hvpch.cpp
    ${GAME_FOLDER}hvpch.h
)
add_library(Game SHARED ${GAME_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${GAME_FILES})
target_include_directories(Game PRIVATE
    "${GAME_FOLDER}"
    "${EXTERNAL_FOLDER}"
    "${EXTERNAL_FOLDER}FastNoise2/include"
    "${EXTERNAL_FOLDER}rapidjson/include"
    "${EXTERNAL_FOLDER}DirectXTex"
    "${EXTERNAL_FOLDER}box2d/include/box2d"
    "${EXTERNAL_FOLDER}box2dcpp/include/box2cpp"
    "${EXTERNAL_FOLDER}PhysX/physx/include"
    "Core"
    "Platform"
    "GUI"
    "Engine"
)
target_link_libraries(Game PRIVATE
    Core
    Platform
    GUI
    Engine
)
target_link_directories(Game PRIVATE
    ${CMAKE_BINARY_DIR}
)
target_compile_definitions(Game PRIVATE ${COMMON_COMPILE_DEFINITIONS} HV_GAME_DLL)
target_compile_options(Game PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_options(Game PRIVATE "/FI${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Game.dir/$<CONFIG>/cmake_pch.hxx")
target_precompile_headers(Game PRIVATE ${GAME_FOLDER}hvpch.h)
target_link_options(Game PRIVATE ${COMMON_LINK_OPTIONS})
set_property(TARGET Game PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${WORKING_FOLDER})
set_target_properties(Game PROPERTIES ${COMMON_TARGET_PROPERTIES})
add_dependencies(Game Core Platform GUI Engine)
# ==================== GAME ====================


# ==================== EDITOR ====================
set(EDITOR_FILES
    ${EDITOR_FOLDER}AuthoringTools/MaterialTool.cpp
    ${EDITOR_FOLDER}AuthoringTools/MaterialTool.h
    ${EDITOR_FOLDER}AuthoringTools/ScriptTool.cpp
    ${EDITOR_FOLDER}AuthoringTools/ScriptTool.h
    ${EDITOR_FOLDER}Systems/EditorRenderSystem.cpp
    ${EDITOR_FOLDER}Systems/EditorRenderSystem.h
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
    ${EDITOR_FOLDER}hvpch.cpp
    ${EDITOR_FOLDER}hvpch.h
)
add_library(Editor SHARED ${EDITOR_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EDITOR_FILES})
target_include_directories(Editor PRIVATE
    "${EDITOR_FOLDER}"
    "${EXTERNAL_FOLDER}rapidjson/include"
    "${EXTERNAL_FOLDER}DirectXTex"
    "${EXTERNAL_FOLDER}box2d/include/box2d"
    "${EXTERNAL_FOLDER}box2dcpp/include/box2cpp"
    "${EXTERNAL_FOLDER}PhysX/physx/include"
    "Core"
    "Platform"
    "GUI"
    "Engine"
)
target_link_libraries(Editor PRIVATE
    Core
    Platform
    GUI
    Engine
    Game
)
target_link_directories(Editor PRIVATE
    ${CMAKE_BINARY_DIR}
)
target_compile_definitions(Editor PRIVATE ${COMMON_COMPILE_DEFINITIONS} HV_EDITOR_DLL)
target_compile_options(Editor PRIVATE ${COMMON_COMPILE_OPTIONS})
target_precompile_headers(Editor PRIVATE ${EDITOR_FOLDER}hvpch.h)
target_link_options(Editor PRIVATE ${COMMON_LINK_OPTIONS})
set_property(TARGET Editor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${WORKING_FOLDER})
set_target_properties(Editor PROPERTIES ${COMMON_TARGET_PROPERTIES})
add_dependencies(Editor Core Platform GUI Engine Game)
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
    "${EXTERNAL_FOLDER}rapidjson/include"
	"${EXTERNAL_FOLDER}box2dcpp/include/box2cpp"
	"${EXTERNAL_FOLDER}PhysX/physx/include"
    "Core"
    "Platform"
	"GUI"
	"Engine"
	"Game"
	"Editor"
)
target_link_libraries(Launcher PRIVATE
    Core    
    Platform
    GUI
    Engine
    Game
    Editor
)
target_link_directories(Launcher PRIVATE
    ${CMAKE_BINARY_DIR}
)
target_compile_definitions(Launcher PRIVATE ${COMMON_COMPILE_DEFINITIONS})
target_compile_options(Launcher PRIVATE ${COMMON_COMPILE_OPTIONS})
target_link_options(Launcher PRIVATE ${COMMON_LINK_OPTIONS})
set_property(TARGET Launcher PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${WORKING_FOLDER})
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Launcher)
set_target_properties(Launcher PROPERTIES ${COMMON_TARGET_PROPERTIES} RUNTIME_OUTPUT_NAME $<CONFIG>)
#add_custom_command(TARGET Launcher POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Launcher> ${WORKING_FOLDER})
add_dependencies(Launcher Core Platform GUI Engine Game Editor)
# ==================== LAUNCHER ====================
