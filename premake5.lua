workspace "joystick"
    language "C++"
    cppdialect "C++17"

    location "build"

    configurations { "debug", "release" }

    filter { "configurations:debug" }
        targetdir "build/%{prj.name}/debug"
        objdir    "build/%{prj.name}/debug"
        symbols "On"
        optimize "Off"
        buildoptions { "-std=c++2a" }

    filter { "configurations:release" }
        targetdir "build/%{prj.name}/release"
        objdir    "build/%{prj.name}/release"
        optimize "Full"
        buildoptions { "-std=c++2a", "-Werror", "-Wextra", "-Wall", "-Wpedantic" }

    filter { }

project "test"
    kind "ConsoleApp"
    files { "src/main.cpp" }
    links "pthread"

project "module"
    kind "SharedLib"
    files { "src/module.cpp" }
    includedirs { "ext/lunar-sol2", "ext/lunar-sol2/sol2/include" }
    links "pthread"

    targetprefix "gmcl_"
    targetname "joystick"
    targetsuffix "_linux64"
    targetextension ".dll"
