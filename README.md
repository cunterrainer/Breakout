# Breakout
Welcome to Breakout, a classic arcade game! This version reintroduces the excitement of breaking bricks and maneuvering a paddle to keep the ball in play. Get ready for a challenging experience with enhanced gameplay mechanics.

![image info](./docs/preview.png)

# Controls
## Keyboard
- **A/D or left/right arrow:** Move the paddle horizontally.  
- **Space:** Launch the ball at the start of the game or resume after a failed attempt.  
- **R:** Reset the game.  
- **M:** Mute the audio.  
- **X:** Render only the outlines of objects.  
- **G:** Make the bottom a hitbox (Game can no longer be lost). (Default: Off)  
- **P:** Toggle the paddles hitbox. (Default: On)  
- **B:** Show the speed of your ball. (Default: Off)  
- **I:** Toggle increment ball speed when scored. (Default: On)  
- **F3:** Show controlls (Switch between keyboard and controller with A/D/Left/Right)  
- **F:** Show FPS. (Default: Off)  
- **ESC:** Pause/resume the game.  
- **+/- or W/A:** Increase/Decrease the ball's speed.  
- **. or ,:** Increase/Decrease fps limit (if limit = 0, there is no limit).

## Controller
Here, the PS4 controller buttons are used as an example, but other controllers will work as well:
- **DPAD or Left Stick:** Move the paddle horizontally.  
- **X:** Launch the ball at the start of the game or resume after a failed attempt.  
- **△:** Reset the game.  
- **□**: Mute the audio.  
- **◯**: Render only the outlines of objects.  
- **Right Stick pressed:** Make the bottom a hitbox (Game can no longer be lost). (Default: Off)  
- **Left Stick pressed:** Toggle the paddles hitbox. (Default: On)  
- **L1:** Show the speed of your ball. (Default: Off)  
- **R1:** Toggle increment ball speed when scored. (Default: On)  
- **R2:** Show controlls (Switch between keyboard and controller with DPAD Left/Right)  
- **SHARE:** Show FPS.  
- **OPTIONS:** Pause/resume the game.  
- **DPAD Up/Down:**  Increase/Decrease the ball's speed.  

If you encounter issues with controller input, please verify that you have selected the correct controller, ensuring it corresponds to the first one assigned by your PC.

# Build Instructions
## Prerequisites
### Linux
Following libraries have to be installed and accessible to the current user:
- xorg (should contain:)
  - libx11
  - libxcursor
  - libxrandr
  - libxinerama
  - libxi

## Using premake
This project utilizes Premake as its build system, offering seamless integration with Visual Studio, Clang, and GCC. To set up the project, follow these steps:

## Clone the repository

``` bash
git clone https://github.com/pyvyx/Breakout.git
```
``` bash
cd Breakout
```

## Visual Studio

``` bash
vendor\premake5.exe vs2022
```
This should generate a .sln file

## Make

Windows:
``` bash
vendor\premake5.exe gmake2 [cc]
```

Linux:
``` bash
vendor/premake5linux gmake2 [cc]
```

macOS:
``` bash
vendor/premake5macos gmake2 [cc]
```

GCC should be the default compiler on Windows and Linux, macOS uses Clang by default, but you can explicitly specify it if you want.  
GCC:   --cc=gcc  
Clang: --cc=clang  
There are also other compilers available however building has only been tested with gcc, clang and msvc

### Build

``` bash
make [-j] config=<configuration>
```
Configurations:
 - debug_x86
 - debug_x64 (default, the same as just using `make`)
 - release_x86
 - release_x64

macOS:
 - debug_universal (default, the same as just using `make`)
 - release_universal

`-j` flag utilises multi-threaded compilation

``` bash
make help
```
to get a list of all the available configurations

## Additional Information
For more details on Premake options, use the following commands:

Windows:
``` bash
vendor\premake5.exe --help
```

Linux:
``` bash
vendor/premake5linux --help
```

macOS:
``` bash
vendor/premake5macos --help
```

## Troubleshooting
If you experience linking errors on Linux, resolve them by adding the necessary libraries to `Snake/premake5.lua`. Ensure these libraries are added before the last line in the file:

``` lua
filter {}
```

For linking errors, use this example:
``` lua
filter "system:linux"
    links {
        "GL",
        "X11",
        "rt",
        "dl",
        "m"
    }
```

For missing include directories, use this example:
``` lua
filter "system:linux"
    links {
        "GL",
        "X11",
        "rt",
        "dl",
        "m"
    }

    includedirs {
        "src/",
        "include/
    }
```

Make sure to tailor these adjustments to accommodate any specific libraries or directories required for successful compilation on the Linux platform.