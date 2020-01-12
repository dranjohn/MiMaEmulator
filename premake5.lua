projectName = "MiMaEmulator"

workspace (projectName)
	architecture "x64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release"
	}

outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project (projectName)
	location (projectName)
	kind "StaticLib"

	language "C++"
	cppdialect "C++17"
	
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
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/fmt/include"
	}

	postbuildcommands
	{
		"{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputDir .. "/Sandbox"
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "on"
		

	filter "configurations:Debug"
		defines "MIMA_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "MIMA_RELEASE"
		optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"

	language "C++"
	cppdialect "C++17"
	
	targetdir ("bin/" .. outputDir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputDir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		projectName .. "/src",
		projectName .. "/vendor/spdlog/include",
		projectName .. "/vendor/fmt/include"
	}

	links
	{
		projectName
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "on"
		

	filter "configurations:Debug"
		defines "MIMA_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "MIMA_RELEASE"
		optimize "On"