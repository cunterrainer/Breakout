project "Breakout"
    language "C"
    cdialect "C11"
    flags "FatalWarnings"

    filter { "toolset:gcc* or toolset:clang*", "platforms:x86", "system:windows" }
        linkoptions "res/icon/iconx86.res"

    filter { "toolset:gcc* or toolset:clang*", "platforms:x64", "system:windows" }
        linkoptions "res/icon/icon.res"

    -- gcc* clang* msc*
    filter "toolset:msc*"
        warnings "High" -- High
        externalwarnings "Default" -- Default
        files "res/icon/icon.rc"
        buildoptions { "/sdl" }
        disablewarnings "4244" -- float to int without cast

    filter { "toolset:gcc* or toolset:clang*" }
        enablewarnings {
            "cast-align",
            "cast-qual",
            "disabled-optimization",
            "format=2",
            "init-self",
            --"missing-declarations",
            "missing-include-dirs",
            "redundant-decls",
            "shadow",
            "sign-conversion",
            "strict-overflow=5",
            "switch-default",
            "undef",
            "uninitialized",
            "unreachable-code",
            "unused",
            "alloca",
            "conversion",
            "deprecated",
            "format-security",
            "null-dereference",
            "stack-protector",
            "vla",
            "shift-overflow"
        }
        disablewarnings { "unused-parameter", "conversion", "missing-field-initializers" }

    filter "toolset:gcc*"
        warnings "Extra"
        externalwarnings "Off"
        linkgroups "on" -- activate position independent linking
        enablewarnings {
            "array-bounds=2",
            "duplicated-branches",
            "duplicated-cond",
            "logical-op",
            "arith-conversion",
            "stringop-overflow=4",
            "implicit-fallthrough=3",
            "trampolines"
        }

    filter "toolset:clang*"
        warnings "Extra"
        externalwarnings "Everything"
        enablewarnings {
            "array-bounds",
            "long-long",
            "implicit-fallthrough", 
        }
    filter {}

    files {
        "**.c",
        "**.h"
    }

    includedirs {
        RaylibDir .. "/src"
    }

    externalincludedirs {
        RaylibDir .. "/src"
    }

    links "raylib"

    filter "system:windows"
        links {
            "Winmm",
            "opengl32",
            "gdi32",
            "shell32",
            "User32"
        }

    filter "system:linux"
        links {
            "GL",
            "X11",
            "rt",
            "dl",
            "m"
        }

    filter "system:macosx"
        linkoptions "-framework AppKit -framework iokit -framework OpenGl"
        disablewarnings { "sign-conversion" }

    filter { "configurations:Debug" }
        kind "ConsoleApp"
        floatingpoint "default"

    filter { "configurations:Release" }
        kind "WindowedApp"
        entrypoint "mainCRTStartup"
        floatingpoint "fast"
    filter {}
