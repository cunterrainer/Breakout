project "Breakout"
    language "C"
    cdialect "C11"
    flags "FatalWarnings"

    -- gcc* clang* msc*
    filter "toolset:msc*"
        warnings "High" -- High
        externalwarnings "Default" -- Default
        buildoptions { "/sdl" }

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
