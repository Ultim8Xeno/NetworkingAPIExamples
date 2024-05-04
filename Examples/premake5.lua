project "Examples"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetdir "%{wks.location}/bin/%{cfg.system}/%{cfg.buildcfg}/%{cfg.architecture}/%{prj.name}"
	objdir "%{wks.location}/bin-int/%{cfg.system}/%{cfg.buildcfg}/%{cfg.architecture}/%{prj.name}"

	files
	{
		"**.hpp",
		"**.cpp",
		"**.h",
		"**.c"
	}

	includedirs
	{
		"%{wks.location}/Networking/src",
		"%{wks.location}/Networking/dependencies/include",
		"src"
	}

	links
	{
		"Networking"
	}

	filter "configurations:Debug"
		defines "DEBUG"
		runtime "Debug"
		symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"