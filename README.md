# Super8_jitcore
Chip8 Emulator using a dynamic recompiler core (dynarec/jit).

This project is a WIP. See the accompanying document "Introduction to Dynamic Recompilation in Emulation" under my Dynarec_Guide repo for a basic introduction behind dynamic recompilers and the problems and solutions associated with them.

The main entry point for this emulator lies in Super8_jitcore/Source/Super8.cpp

The engine behind the emulator lies in Super8_jitcore/Source/Chip8Engine/Chip8Engine*

This project uses unmodified copies of the SDL2 & SDL2_ttf libraries, and they are provided under the zlib license (https://www.libsdl.org/index.php).

This project was developed in Visual Studio 2015 Community Edition. In order to cross-compile, you will need a replacement function for the VirtualAlloc function (See Chip8Engine_CacheHandler.cpp) referenced, as well as the appropriate *.dll replacements for the SDL libraries (or compile everything yourself).

This project is distributed under the GPLv3 license. See the LICENSE file for the full license.