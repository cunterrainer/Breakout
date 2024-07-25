project "raylib"
    language "C"
    kind "StaticLib"
    defines "PLATFORM_DESKTOP"
    warnings "off"

    files {
        "src/**.h",
        "src/rcore.c",
        "src/rmodels.c",
        "src/rshapes.c",
        "src/rtext.c",
        "src/rtextures.c",
        "src/utils.c",
        "src/rglfw.c",
        "src/raudio.c"
    }

    includedirs {
        "src/external/glfw/include"
    }

    filter { "system:macosx", "files:src/rglfw.c" }
        compileas "Objective-C"

    filter "toolset:gcc* or toolset:clang*"
        disablewarnings "implicit-function-declaration"

    filter { "configurations:Release", "toolset:gcc* or toolset:clang*" }
        buildoptions { "-ffunction-sections", "-fdata-sections" } -- places each function and data item in its own section

    filter "system:windows"
        defines "_CRT_SECURE_NO_WARNINGS"
    filter "system:macosx"
        defines "GL_SILENCE_DEPRECATION"