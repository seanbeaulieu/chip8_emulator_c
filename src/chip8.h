
#define CHIP8_H
#include <stdint.h>

typedef struct Chip8 {
    // 4kb of memory
    uint8_t memory[4096];

    // 16 registers, general purpose 8-bit registers
    // V0 - VF
    uint8_t V[16];

    // declare a stack
    // stores return addresses when a subroutine is called
    // array of unsigned 16-bit int
    uint16_t stack[16];
    uint8_t top; // stack pointer to top

    // program counter
    // holds memory address of the current instruction
    // unsigned 16-bit int
    uint16_t pc;

    // index register
    // stores memory addresses for operations 
    // unsigned 16-bit int
    uint16_t I;

    // delay timer
    // decremets at 60hz until reaching 0
    // unsigned 8-bit int
    uint8_t delay_timer;

    // sound timer
    // when != 0, sound is played
    // unsigned 8-bit int
    uint8_t sound_timer;

    // display buffer
    // 2d array of unsigned 8-bit int
    // 64 x 32
    // 1 is white, 0 is black
    uint8_t display[64][32];

} Chip8;

// stack functions
void chip8_push(Chip8* chip8, uint16_t value);
uint16_t chip8_pop(Chip8* chip8);

// chip8 init functions
void chip8_init(Chip8* chip8);
void chip8_init_memory(Chip8* chip8);
void chip8_init_registers(Chip8* chip8);
void chip8_load_font(Chip8* chip8);
void load_rom(Chip8* chip8, const char* filename);
