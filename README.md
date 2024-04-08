# CHIP-8 Emulator
This is my implementation of the CHIP-8 interpreter, written in C, with graphical support from OpenGL and GLFW 3.3.

To run ROMs, goto /src, and enter ./chip8 _rom_.ch8
 
The CHIP-8 interpreted programming language was invented by Joe Weisbecker in 1977. Also the inventor of the COSMAC VIP microcomputer, he invented the language to make games easier to program for said computer. CHIP-8 is considered to be the 'Hello World' of video game emulators, so I took a stab at it to learn more about low-level programming and to practice my skills with C. 

The COSMAC VIP was a 4k system with 4096 bytes of memory. It utilizes 16 registers, V0 to VF. CHIP-8 is originally made for a 64x32 pixel display, though further adaptations of the language like SUPER-CHIP extends that to 128x64. In order to register inputs, CHIP-8 uses a 16-key hexadecimal keyboard with keys from 0-F. 

The language has 35 different opcodes. The interpreter works by running a Fetch, Decode, and Execute loop while any particular ROM is running. By using switch statements in C, one can efficiently loop through at 60 frames per second and match the current instruction. The way the next instruction is retrieved is through incrementing the program counter, which is a pointer to the instruction in memory.

There are several quirks and different implementations that classic and modern ROMs will differ on. While an extended implementation would allow these to be configurable, I have not added such functionality as of yet. Some examples are:

- 8XY1, 8XY2, and 8XY3 opcodes setting the VF register to 0

- FX55 and FX65 register not incrementing the VI register 

- Even though the display is refreshed once every 1/60 seconds, instructions are still relatively slow at 60 instructions per second. The way to accelerate this is to run the Fetch and Decode steps on a cycles_per_frame loop, effectively 10xing the speed at which instructions are called. However, when drawing to the display, one must alter the display instruction such that it will break and wait for the frame refresh, limiting the display instruction to a max of 60 calls per second. This is done to avoid screen tearing.

- When the display instruction attempts to write past the boundaries of 64x32, the sprite will get cut off (clipped) instead of wrapping around.

- 8XY6 and 8XYE do not store the values of VY into VX.

- BNNN instruction Jump with offset uses original COSMAC VIP functionality where V0 is added to the intended jump location.

Some of these 'quirks' are features more present on ROMs developed for SUPER-CHIP, XO-CHIP, and related adaptations.

TODO:
 - Graphical interface to select ROMs?
 - Add sound
 - Improve flickering on display changes
