#include "./chip8.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

// intialzies the passed in Chip8 struct
void chip8_init(Chip8* chip8) {
    chip8_init_memory(chip8);
    chip8_init_registers(chip8);
    chip8_load_font(chip8);
    chip8->pc = 0x200;  
    chip8->top = 0;      
    
}

// intializing emulator memory
// takes in a pointer to a Chip8 struct
// loops through and sets all memory to 0
void chip8_init_memory(Chip8* chip8) {
    for (int i = 0; i < 4096; i++) {
        chip8->memory[i] = 0;
    }
}

// initializing emulator registers
// takes in a pointer to a Chip8 struct
// loops through and sets all the registers to 0
void chip8_init_registers(Chip8* chip8) {
    for (int i = 0; i < 16; i++) {
        chip8->V[i] = 0;
    }
}

// initalize memory location 050 - 09F to font data
// each char is 5 bytes
void chip8_load_font(Chip8* chip8) {
    // declare font 
    uint8_t font_data[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    // loop through memory and set memory starting at 0x050 to specific font
    for (int i = 0; i < sizeof(font_data); i++) {
        chip8->memory[0x050 + i] = font_data[i];
    }
}

// loading the rom into a given Chip8 memory
void load_rom(Chip8* chip8, const char* filename) {
    // open the file in binary read
    FILE* file = fopen(filename, "rb");
    
    // check if null
    if (file == NULL) {
        fprintf(stderr, "failed to open ROM file: %s\n", filename);
        exit(1);
    }

    // check to see if the file is bigger than 4096 - 512
    fseek(file, 0, SEEK_END);
    // get size of file
    long size = ftell(file);
    // move fp back to start
    rewind(file);

    // trigger error (not really necessary)
    if (size > 4096 - 512) {
        fprintf(stderr, "ROM size exceeds available memory\n");
        exit(1);
    }

    // write the file contents to Chip8 memory starting at 0x200
    fread(&chip8->memory[0x200], 1, size, file);
    fclose(file);
}

// stack push function
void chip8_push(Chip8* chip8, uint16_t value) {
    // can do stack check here

    chip8->stack[chip8->top] = value;
    chip8->top++;
}

// stack pop function
uint16_t chip8_pop(Chip8* chip8) {
    // can do stack check here

    chip8->top--;
    return chip8->stack[chip8->top];
}

// print display to terminal (for bug testing)
void print_display(Chip8* chip8) {
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            printf("%d ", chip8->display[y][x] ? 1 : 0);
        }
        printf("\n");
    }
    printf("\n");
}