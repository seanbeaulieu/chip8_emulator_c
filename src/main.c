#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "./chip8.h"

// openGL
#include <GL/gl.h>
#include "./glfw3.h"


// these allow us to grab the instructions from each fetch
#define FETCH_OPCODE() (chip8.memory[chip8.pc] << 8 | chip8.memory[chip8.pc + 1])
#define EXTRACT_X(opcode) ((opcode & 0x0F00) >> 8)
#define EXTRACT_Y(opcode) ((opcode & 0x00F0) >> 4)
#define EXTRACT_N(opcode) (opcode & 0x000F)
#define EXTRACT_NN(opcode) (opcode & 0x00FF)
#define EXTRACT_NNN(opcode) (opcode & 0x0FFF)

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <rom_file>\n", argv[0]);
        return 1;
    }

    // initialize Chip8
    Chip8 chip8;
    chip8_init(&chip8);

    // load chip8 into memory
    load_rom(&chip8, argv[1]);

    // initialize glfw
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    // create glfw window
    GLFWwindow* window = glfwCreateWindow(800, 600, "CHIP-8 Emulator", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return 1;
    }

    // make current
    glfwMakeContextCurrent(window);

    // opengl intialization
    glViewport(0, 0, 800, 600);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 800, 600, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();



    // emulation infinite loop
    while(!glfwWindowShouldClose(window)) {

        // fetch
        uint16_t opcode = FETCH_OPCODE();
        chip8.pc += 2;

        // decode and execute
        switch (opcode & 0xF000) {
            case 0x0000:
                switch (opcode & 0x00FF) {
                    case 0x00E0:
                        // clears the display by setting the display 2d array to 0
                        // sets the next display size bytes of the display memory block to zero
                        memset(chip8.display, 0, sizeof(chip8.display));
                        break;
                }
                break;
            case 0x1000:
                // sets the program counter to the address given by NNN
                chip8.pc = EXTRACT_NNN(opcode);
                break;
            case 0x6000:
                // sets the register vx to the value nn
                chip8.V[EXTRACT_X(opcode)] = EXTRACT_NN(opcode);
                break;
            case 0x7000:
                // adds nn to register vx
                chip8.V[EXTRACT_X(opcode)] += EXTRACT_NN(opcode);
                break;
            case 0xA000:
                // Set index register I to NNN
                chip8.I = EXTRACT_NNN(opcode);
                break;
            case 0xD000: {
                // draw sprite at coordinate (VX, VY) with N bytes of sprite data starting at the address stored in I

                // extract x, y, and n
                // modulo by 64 and 32 so position can wrap
                uint8_t x = chip8.V[EXTRACT_X(opcode)] % 64;
                uint8_t y = chip8.V[EXTRACT_Y(opcode)] % 32;
                uint8_t n = EXTRACT_N(opcode);

                // set register vf to 0
                chip8.V[0xF] = 0;

                // loops through n rows
                for (int row = 0; row < n; row++) {
                    // get the nth byte of sprite data from memory (starts at I)
                    uint8_t sprite_byte = chip8.memory[chip8.I + row];

                    // loop through each 8 pixels of the sprite row
                    for (int col = 0; col < 8; col++) {
                        // get the current sprite pixel by shifting mask to the left by col, starts at most significant bit
                        uint8_t sprite_pixel = sprite_byte & (0x80 >> col);

                        // get the current screen pixel
                        // initialize as a pointer to the memory address of the coordinates of the screen given
                        // by the sprite starting position, offset by the row and column
                        uint32_t* screen_pixel = &chip8.display[y + row][x + col];

                        // check to see if the sprite pixel is on
                        if (sprite_pixel) {
                            // then, if the associated screen pixel is also on, set the VF register to 1
                            if (*screen_pixel == 0xFFFFFFFF) {
                                chip8.V[0xF] = 1;
                            }
                            // otherwise, set the screen pixel on
                            *screen_pixel ^= 0xFFFFFFFF;
                        }

                        // hitting the right edge of the screen will stop drawing the current row
                        if (x + col >= 64) {
                            break;
                        }
                    }

                    // reaching the bottom of the screen will stop
                    if (y + row >= 32) {
                        break;
                    }
                }
                break;

                default:
                    // breaks if operation unknown
                    break;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);

        // Render the CHIP-8 display
        glBegin(GL_POINTS);
        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 64; x++) {
                if (chip8.display[y][x]) {
                    glVertex2i(x * 10, y * 10);
                }
            }
        }
        glEnd();
        glfwSwapBuffers(window);
    }

    // end glfw clean
    glfwTerminate();

    return 0;
}