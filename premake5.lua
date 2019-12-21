projectName = "MiMaEmulator"

workspace (projectName)
	architecture "x64"
	startproject (projectName)

	configurations
	{
		"Debug",
		"Release"
	}

outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project (projectName)
	location (projectName)
	kind "ConsoleApp"

	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	
	targetdir ("bin/" .. outputDir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputDir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include"
	}

	filter "system:windows"
		systemversion "latest"
		

	filter "configurations:Debug"
		defines "MIMA_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "MIMA_RELEASE"
		optimize "On"

	