workspace "Snowmeter"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "3rdparty/GLFW/include"
IncludeDir["Glad"] = "3rdparty/Glad/include"
IncludeDir["glm"] = "3rdparty/glm/"

include "3rdparty/GLFW"
include "3rdparty/Glad"

project "Snowmeter"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/"..outputdir.."/%{prj.name}")
	objdir ("bin-int/"..outputdir.."/%{prj.name}")

	files
	{
		-- "src/**.h",
		"src/**.cpp",
	}

	-- NOTE: Make sure to remove this after debugging, or obv can cause linking errors
	removefiles
	{
		"src/alsa.cpp",
		"src/blockingread.cpp"
		-- "src/audioinput.cpp",
		-- "src/pulseaudio.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.glm}",
	}

	links
	{
		"GLFW",
		"Glad",
		"Xrandr",
		"Xi",
		"X11",
		"dl",
		"pthread",
		"pulse",
		"m",
		"fftw3",
		"fftw3f",
		-- "pulse-simple",
		-- "pulseasync"
		-- "asound"
	}

	defines
	{
		"GLFW_INCLUDE_NONE"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
