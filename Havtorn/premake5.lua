workspace "Havtorn"
	architecture "x86_64"
	location "Source"

	configurations
	{ "Debug", "Release" }
	startproject "Launcher"

outputdir = "%{cfg.buildcfg}_%{cfg.system}_%{cfg.architecture}"
engineProj = "Engine"
engineSource = "Source/" .. engineProj .. "/"
editorProj = "Editor"
editorSource = "Source/" .. editorProj .. "/"
imGuiProj = "ImGui"
imGuiSource = "Source/" .. imGuiProj .. "/"
gameProj = "Game"
gameSource = "Source/" .. gameProj .. "/"
externalLinkDir = "External/Lib/"

project "Engine"
	location ("Source/" .. engineProj)
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	architecture "x86_64"

	targetname "%{prj.name}_%{cfg.buildcfg}"

	targetdir ("Bin/" .. outputdir .. "/%{prj.name}") 
	objdir ("Temp/" .. outputdir .. "/%{prj.name}") 

	warnings "Extra"
	flags { "FatalWarnings", "ShadowedVariables", "MultiProcessorCompile" }

	pchheader "hvpch.h"
	pchsource ("Source/" .. engineProj .. "/hvpch.cpp")
	forceincludes { "hvpch.h" }

	files 
	{
		"Source/%{prj.name}/**.h",
		"Source/%{prj.name}/**.cpp",
		
		vpaths 
		{
			["*"] = "Source/"
		}
	}

	includedirs
	{
		"Source/%{prj.name}",
		"External",
		"External/FastNoise2/include",
		"External/rapidjson",
		"External/DirectXTex",
		"External/box2d/include/box2d",
		"External/box2dcpp/include/box2cpp",
		"External/PhysX/physx/include",
		"Source/ImGui"
	}

	libdirs 
	{
		"Lib",
		"External/Lib"
	}

	links
	{
		"ImGui"
	}

	floatingpoint "Fast"
	debugdir "Bin/"

	-- Begin Shader stuff
		filter{} -- Clear filter

		shaderSource = engineSource .. "Graphics/Shaders/"
		shaderTarget = "$(SolutionDir)../Bin/Shaders/"

		shadermodel("5.0")
		-- Warnings as errors
		shaderoptions({"/WX"})
		shaderobjectfileoutput(shaderTarget .. "%%(Filename).cso")

		files
		{
			shaderSource .. "*.hlsl",
			shaderSource .. "Includes/*.hlsli",
		}
	
		filter("files:**_PS.hlsl")
		removeflags("ExcludeFromBuild")
		shadertype("Pixel")
	
		filter("files:**_VS.hlsl")
		removeflags("ExcludeFromBuild")
		shadertype("Vertex")

		filter("files:**_GS.hlsl")
		removeflags("ExcludeFromBuild")
		shadertype("Geometry")
		
		filter{} -- Clear filter
	-- End Shader Stuff

	filter "system:Windows"
		staticruntime "On"
		systemversion "latest"
		vectorextensions "SSE4.1"

		defines 
		{
			"HV_PLATFORM_WINDOWS",
			"HV_BUILD_DLL"
		}

		prebuildcommands
		{
			"{MKDIR} ../../Bin/",
			"{MKDIR} ../../Bin/Shaders/"
		}

		postbuildcommands
		{
			"{COPY} %{cfg.buildtarget.relpath} ../../Bin/"
		}

	filter "configurations:Debug"
		defines "HV_DEBUG"
		buildoptions "/MDd"
		staticruntime "off"
		runtime "Debug"
		symbols "On"
		
		links
		{
			externalLinkDir .. "Debug/assimp-vc142-mtd",
			externalLinkDir .. "Debug/DirectXTex_Debug",
			externalLinkDir .. "Debug/box2dd",
			externalLinkDir .. "Debug/PhysX/PhysX_64",
			externalLinkDir .. "Debug/PhysX/PhysXCommon_64",
			externalLinkDir .. "Debug/PhysX/PhysXExtensions_static_64",
			externalLinkDir .. "Debug/PhysX/PhysXFoundation_64",
			externalLinkDir .. "Debug/PhysX/PhysXPvdSDK_static_64",
			externalLinkDir .. "Debug/PhysX/PhysXCharacterKinematic_static_64"
		}
		
		ignoredefaultlibraries
		{
			"LIBCMTD"
		}
		
		defines 
		{
			"HV_ENABLE_ASSERTS"
		}

	filter "configurations:Release"
		defines "HV_RELEASE"
		buildoptions "/MD"
		staticruntime "off"
		runtime "Release"
		optimize "On"
		flags { "LinkTimeOptimization" }
	
		links
		{
			externalLinkDir .. "Release/assimp-vc142-mt",
			externalLinkDir .. "Release/DirectXTex_Release",
			externalLinkDir .. "Release/box2d",
			externalLinkDir .. "Release/PhysX/PhysX_64",
			externalLinkDir .. "Release/PhysX/PhysXCommon_64",
			externalLinkDir .. "Release/PhysX/PhysXExtensions_static_64",
			externalLinkDir .. "Release/PhysX/PhysXFoundation_64",
			externalLinkDir .. "Release/PhysX/PhysXPvdSDK_static_64",
			externalLinkDir .. "Debug/PhysX/PhysXCharacterKinematic_static_64"
		}

		ignoredefaultlibraries
		{
			"LIBCMT"
		}



project "Game"
	location ("Source/" .. gameProj)
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	architecture "x86_64"

	targetname "%{prj.name}_%{cfg.buildcfg}"

	targetdir ("Bin/" .. outputdir .. "/%{prj.name}") 
	objdir ("Temp/" .. outputdir .. "/%{prj.name}") 

	warnings "Extra"
	flags { "FatalWarnings", "ShadowedVariables", "MultiProcessorCompile" }

	pchheader "hvpch.h"
	pchsource ("Source/" .. gameProj .. "/hvpch.cpp")
	forceincludes { "hvpch.h" }

	files 
	{
		"Source/%{prj.name}/**.h",
		"Source/%{prj.name}/**.cpp",
		
		vpaths 
		{
			["*"] = "Source/"
		}
	}

	includedirs
	{
		"Source/%{prj.name}",
		"External",
		"External/FastNoise2/include",
		"External/rapidjson",
		"External/DirectXTex",
		"External/box2d/include/box2d",
		"External/box2dcpp/include/box2cpp",
		"External/PhysX/physx/include",
		"Source/Engine",
		"Source/ImGui"		
	}

	libdirs 
	{
		"Lib/",
		"External/Lib"
	}

	links
	{
		"Engine",
		"ImGui"
	}

	floatingpoint "Fast"
	debugdir "Bin/"

	filter "system:Windows"
		staticruntime "On"
		systemversion "latest"
		vectorextensions "SSE4.1"

		defines 
		{
			"HV_PLATFORM_WINDOWS"
		}

		postbuildcommands
		{
		    "{copy} %{cfg.buildtarget.relpath} ../../bin/"
		}

	filter "configurations:Debug"
		defines "HV_DEBUG"
		buildoptions "/MDd"
		staticruntime "off"
		runtime "Debug"
		symbols "On"

		defines 
		{
			"HV_ENABLE_ASSERTS"
		}

	filter "configurations:Release"
		defines "HV_RELEASE"
		buildoptions "/MD"
		staticruntime "off"
		runtime "Release"
		optimize "On"
		flags { "LinkTimeOptimization" }
		




project "Editor"
	location ("Source/" .. editorProj)
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	architecture "x86_64"

	targetname "%{prj.name}_%{cfg.buildcfg}"

	targetdir ("Bin/" .. outputdir .. "/%{prj.name}") 
	objdir ("Temp/" .. outputdir .. "/%{prj.name}") 

	warnings "Extra"
	flags { "FatalWarnings", "ShadowedVariables", "MultiProcessorCompile" }

	files 
	{
		"Source/%{prj.name}/**.h",
		"Source/%{prj.name}/**.cpp",
		
		vpaths 
		{
			["*"] = "Source/"
		}
	}

	includedirs
	{
		"Source/%{prj.name}",
		"External/rapidjson",
		"External/DirectXTex",
		"External/box2d/include/box2d",
		"External/box2dcpp/include/box2cpp",
		"External/PhysX/physx/include",
		"Source/Engine",
		"Source/ImGui"
	}

	libdirs 
	{ 
		"Lib/",
		"External/Lib"
	}

	links
	{
		"Engine",
		"Game",
		"ImGui"
	}

	floatingpoint "Fast"
	debugdir "Bin/"

	filter "system:Windows"
		staticruntime "On"
		systemversion "latest"
		vectorextensions "SSE4.1"

		defines 
		{
			"HV_PLATFORM_WINDOWS"
		}

		postbuildcommands
		{
			"{COPY} %{cfg.buildtarget.relpath} ../../Bin/"
		}

	filter "configurations:Debug"
		defines "HV_DEBUG"
		buildoptions "/MDd"
		staticruntime "off"
		runtime "Debug"
		symbols "On"

		defines 
		{
			"HV_ENABLE_ASSERTS"
		}

	filter "configurations:Release"
		defines "HV_RELEASE"
		buildoptions "/MD"
		staticruntime "off"
		runtime "Release"
		optimize "On"
		flags { "LinkTimeOptimization" }
		
		
		
		
		
project "ImGui"
	location ("Source/" .. imGuiProj)
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	architecture "x86_64"

	targetname "%{prj.name}_%{cfg.buildcfg}"

	targetdir ("Bin/" .. outputdir .. "/%{prj.name}") 
	objdir ("Temp/" .. outputdir .. "/%{prj.name}") 

	flags { "ShadowedVariables", "MultiProcessorCompile" }

	files 
	{
		"Source/%{prj.name}/**.h",
		"Source/%{prj.name}/**.cpp",
		
		vpaths 
		{
			["*"] = "Source/"
		}
	}

	includedirs
	{
		"Source/%{prj.name}",
		"External/imgui"
	}

	floatingpoint "Fast"
	debugdir "Bin/"

	filter "system:Windows"
		staticruntime "On"
		systemversion "latest"
		vectorextensions "SSE4.1"

		defines 
		{
			"HV_PLATFORM_WINDOWS",
			"HV_BUILD_DLL"
		}

		postbuildcommands
		{
			"{COPY} %{cfg.buildtarget.relpath} ../../Bin/"
		}

	filter "configurations:Debug"
		defines "HV_DEBUG"
		buildoptions "/MDd"
		staticruntime "off"
		runtime "Debug"
		symbols "On"

		defines 
		{
			"HV_ENABLE_ASSERTS"
		}

	filter "configurations:Release"
		defines "HV_RELEASE"
		buildoptions "/MD"
		staticruntime "off"
		runtime "Release"
		optimize "On"
		flags { "LinkTimeOptimization" }





project "Launcher"
	location "Source/Launcher"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++20"
	architecture "x86_64"

	targetname "%{prj.name}_%{cfg.buildcfg}"

	targetdir ("Bin/" .. outputdir .. "/%{prj.name}") 
	objdir ("Temp/" .. outputdir .. "/%{prj.name}") 

	warnings "Extra"
	flags { "FatalWarnings", "ShadowedVariables", "MultiProcessorCompile" }

	files
	{
		"Source/%{prj.name}/**.h",
		"Source/%{prj.name}/**.cpp",

		vpaths 
		{
			["*"] = "Source/"
		}
	}

	includedirs
	{
		"Source/%{prj.name}",
		"Source/ImGui",
		"Source/Editor",
		"Source/Engine",
		"Source/Game",
		"External/box2d/include/box2d",
		"External/box2dcpp/include/box2cpp",
		"External/PhysX/physx/include"
	}

	links
	{
		"Engine",
		"Game",
		"Editor",
		"ImGui",
	}

	floatingpoint "Fast"
	debugdir "Bin/"

	filter "system:Windows"
		staticruntime "On"
		systemversion "latest"
		vectorextensions "SSE4.1"

		defines 
		{
			"HV_PLATFORM_WINDOWS"
		}
		
		postbuildcommands
		{
			"{MKDIR} ../../Bin/",
			"{COPY} %{cfg.buildtarget.relpath} ../../Bin/"
		}

	filter "configurations:Debug"
		defines "HV_DEBUG"
		buildoptions "/MDd"
		staticruntime "off"
		runtime "Debug"
		symbols "On"

		defines 
		{
			"HV_ENABLE_ASSERTS"
		}

	filter "configurations:Release"
		defines "HV_RELEASE"
		buildoptions "/MD"
		staticruntime "off"
		runtime "Release"
		optimize "On"
		flags { "LinkTimeOptimization" }