workspace "RaytracingVulkan"
    configurations { "Debug", "Release"}
    location "../../build"
project "build"
    --dependson {"glad2","","imgui", "tinyobjloader","soloud"}
    architecture "x64"
    location "../../build"
    debugdir "../../bin/Debug"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir "../../bin/%{cfg.buildcfg}"
    -- ignoredefaultlibraries { "MSVCRT" }
    includedirs { 
        "../../deps/glfw/include"
    }

    files {
        "../../src/**.cc", 
        "../../src/**.cpp", 
        "../../include/**.h"
    }

    libdirs { 
        "../../deps/glfw/lib-vc2022",
        "../../deps/bin/Debug",
        "../../deps/bin/Release"
    }
    
    links {
        "glfw3.lib",
        "kernel32.lib",
        "user32.lib",
        "gdi32.lib",
        "winspool.lib",
        "comdlg32.lib",
        "advapi32.lib",
        "shell32.lib",
        "ole32.lib",
        "oleaut32.lib",
        "uuid.lib",
        "odbc32.lib",
        "odbccp32.lib"
    }
    filter {"configurations:Debug"}
        defines { "DEBUG" }
        symbols "On"

    filter {"configurations:Release"}
        defines { "NDEBUG" }
        optimize "Speed"

    filter {}