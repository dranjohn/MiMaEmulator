-- Workspace setup for the MiMaEmulator
projectName = "MiMaEmulator"

workspace (projectName)
	architecture "x64"
	startproject "Sandbox"

	configurations {
		"Debug",
		"Release"
	}
	
outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
newestCppDialect = "C++17"

-- MiMaEmulator main module
project (projectName)
	location (projectName)
	kind "StaticLib"

	language "C++"
	cppdialect (newestCppDialect)
	
	targetdir ("bin/" .. outputDir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputDir .. "/%{prj.name}")

	pchheader "mimapch.h"
	pchsource "MiMaEmulator/src/mimapch.cpp"

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/fmt/include"
	}

	postbuildcommands {
		"{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputDir .. "/Sandbox",
		"{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputDir .. "/ConsoleInterface"
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


-- Sandbox for testing
project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"

	language "C++"
	cppdialect (newestCppDialect)
	
	targetdir ("bin/" .. outputDir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputDir .. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
		"%{prj.name}/src",
		projectName .. "/src"
	}

	links {
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


--Command Line Interface as a user interface to the MiMaEmulator
project "ConsoleInterface"
	location "ConsoleInterface"
	kind "ConsoleApp"

	language "C++"
	cppdialect (newestCppDialect)
	
	targetdir ("bin/" .. outputDir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputDir .. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
		"%{prj.name}/src",
		projectName .. "/src",
		"%{prj.name}/vendor/fmt/include"
	}

	links {
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