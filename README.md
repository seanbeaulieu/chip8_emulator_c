# CHIP-8 Emulator
 This is my implementation of the CHIP-8 interpreter, written in C.

 To run ROMs, goto /src, and enter ./chip8 _rom_.ch8
 
Some notes:
 - VF reset is on
 - Memory (I register incrementation) is off (FX55 and FX65) 
 - Display wait is set to 60
 - Clipping is on
 - Shifting is on
 - Jumping is off

TODO:
 - Graphical interface to select ROMs?
 - Add sound
 - Improve flickering on display changes
