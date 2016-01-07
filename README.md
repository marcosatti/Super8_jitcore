# Super8
Chip8 Emulator using a dynamic recompiler core (jit).

This project is a WIP. See the accompanying document "Introduction to Dynamic Recompilation in Emulation" under my Dynarec_Guide repo.

The main entry point for this emulator lies in Super8_jitcore/Source/Super8.cpp
The engine behind the emulator lies in Super8_jitcore/Source/Chip8Engine/Chip8Engine*

This project uses the SDL & SDL FreeType libraries.

This project was developed in Visual Studio 2015 Community Edition. In order to cross-compile, you will need a replacement function for the VirtualAlloc function (See Chip8Engine_CacheHandler.cpp) referenced, as well as the appropriate *.dll replacements for the SDL libraries (or compile everything yourself).