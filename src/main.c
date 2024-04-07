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

// define key array
uint8_t keys[16] = {0};

void key_callback(GLFWwindow* window, int key, int scancode, int action) {
    if (action == GLFW_PRESS || action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_1: keys[0x1] = action == GLFW_PRESS ? 1 : 0; break;
            case GLFW_KEY_2: keys[0x2] = action == GLFW_PRESS ? 1 : 0; break;
            case GLFW_KEY_3: keys[0x3] = action == GLFW_PRESS ? 1 : 0; break;
            case GLFW_KEY_4: keys[0xC] = action == GLFW_PRESS ? 1 : 0; break;
            case GLFW_KEY_Q: keys[0x4] = action == GLFW_PRESS ? 1 : 0; break;
            case GLFW_KEY_W: keys[0x5] = action == GLFW_PRESS ? 1 : 0; break;
            case GLFW_KEY_E: keys[0x6] = action == GLFW_PRESS ? 1 : 0; break;
            case GLFW_KEY_R: keys[0xD] = action == GLFW_PRESS ? 1 : 0; break;
            case GLFW_KEY_A: keys[0x7] = action == GLFW_PRESS ? 1 : 0; break;
            case GLFW_KEY_S: keys[0x8] = action == GLFW_PRESS ? 1 : 0; break;
            case GLFW_KEY_D: keys[0x9] = action == GLFW_PRESS ? 1 : 0; break;
            case GLFW_KEY_F: keys[0xE] = action == GLFW_PRESS ? 1 : 0; break;
            case GLFW_KEY_Z: keys[0xA] = action == GLFW_PRESS ? 1 : 0; break;
            case GLFW_KEY_X: keys[0x0] = action == GLFW_PRESS ? 1 : 0; break;
            case GLFW_KEY_C: keys[0xB] = action == GLFW_PRESS ? 1 : 0; break;
            case GLFW_KEY_V: keys[0xF] = action == GLFW_PRESS ? 1 : 0; break;
        }
    }
}

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
        fprintf(stderr, "failed to initialize GLFW\n");
        return 1;
    }

    // create glfw window
    GLFWwindow* window = glfwCreateWindow(640, 320, "CHIP-8 Emulator", NULL, NULL);
    if (!window) {
        fprintf(stderr, "failed to create GLFW window\n");
        glfwTerminate();
        return 1;
    }

    // make current
    glfwMakeContextCurrent(window);

    // opengl intialization
    glViewport(0, 0, 640, 320);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 640, 320, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glfwSetKeyCallback(window, key_callback);

    double prev_time = glfwGetTime();
    // 60hz frame rate
    const double frame_time = 1.0 / 60.0;
    const int cycles_per_frame = 10;

    // emulation infinite loop
    while(!glfwWindowShouldClose(window)) {

        // get difference between time
        double current_time = glfwGetTime();
        double delta_time = current_time - prev_time;

        // temp variables for underflow and carry in arithmetic instructions
        int underflow;
        int carry;
        int display_wait = 0;

        // decrement delay timer
        if (chip8.delay_timer > 0) {
            chip8.delay_timer--;
        }   

        // run 10 cycles per frame
        for (int i = 0; i < cycles_per_frame; i++) {

            // fetch
            uint16_t opcode = FETCH_OPCODE();
            // printf("opcode: %d", opcode);
            chip8.pc += 2;

            /*
            // decrement delay timer
            if (chip8.delay_timer > 0) {
                chip8.delay_timer--;
            } 
            */

            // decode and execute
            switch (opcode & 0xF000) {
                // clear
                case 0x0000:
                    switch (opcode & 0x00FF) {
                        case 0x00E0:
                            // clears the display by setting the display 2d array to 0
                            // sets the next display size bytes of the display memory block to zero
                            memset(chip8.display, 0x0, sizeof(chip8.display));
                            break;
                        case 0x00EE:
                            // pop the last address from stack and set pc to it
                            chip8.pc = chip8_pop(&chip8);
                            break;
                    break;
                    }
                    break;
                // jump
                case 0x1000:
                    // sets the program counter to the address given by NNN
                    chip8.pc = EXTRACT_NNN(opcode);
                    break;

                // subroutine
                case 0x2000: 
                    // calls subroutine at memory location NNN
                    // first, pushes the current PC to the stack
                    chip8_push(&chip8, chip8.pc);
                    chip8.pc = EXTRACT_NNN(opcode);
                    break;

                // skip one instruction if VX == NN
                case 0x3000:
                    if (chip8.V[EXTRACT_X(opcode)] == EXTRACT_NN(opcode)) {
                        chip8.pc += 2;
                    }
                    break;

                // skip one instruction if VX != NN
                case 0x4000:
                    if (chip8.V[EXTRACT_X(opcode)] != EXTRACT_NN(opcode)) {
                        chip8.pc += 2;
                    }
                    break;
                // skip one instruction if VX == VY
                case 0x5000:
                    if (chip8.V[EXTRACT_X(opcode)] == chip8.V[EXTRACT_Y(opcode)]) {
                        chip8.pc += 2;
                    }
                    break;
                // skip one instruction if VX != VY
                case 0x9000:
                    if (chip8.V[EXTRACT_X(opcode)] != chip8.V[EXTRACT_Y(opcode)]) {
                        chip8.pc += 2;
                    }
                    break;
                // set vx
                case 0x6000:
                    // sets the register vx to the value nn
                    chip8.V[EXTRACT_X(opcode)] = EXTRACT_NN(opcode);
                    break;
                // add to vx
                case 0x7000:
                    // adds nn to register vx
                    chip8.V[EXTRACT_X(opcode)] += EXTRACT_NN(opcode);
                    break;

                // logical instructions
                case 0x8000:
                    switch (EXTRACT_N(opcode)) {
                        // set
                        case 0x0000:
                            // set VX to VY
                            chip8.V[EXTRACT_X(opcode)] = chip8.V[EXTRACT_Y(opcode)];
                            break;

                        // ON ORIGINAL CHIP-8, OR, AND, XOR SET VF TO 0 
                        // binary or
                        case 0x0001:
                            // vx is set to vx | vy
                            chip8.V[EXTRACT_X(opcode)] = chip8.V[EXTRACT_X(opcode)] | chip8.V[EXTRACT_Y(opcode)];
                            chip8.V[0xF] = 0;
                            break;
                        // binary and
                        case 0x0002:
                            // vx is set to vx & vy
                            chip8.V[EXTRACT_X(opcode)] = chip8.V[EXTRACT_X(opcode)] & chip8.V[EXTRACT_Y(opcode)];
                            chip8.V[0xF] = 0;
                            break;
                        // logical xor
                        case 0x0003:
                            // vx is set to vx ^ vy
                            chip8.V[EXTRACT_X(opcode)] = chip8.V[EXTRACT_X(opcode)] ^ chip8.V[EXTRACT_Y(opcode)];
                            chip8.V[0xF] = 0;
                            break;
                        case 0x0004:
                        
                            // if vy + vx is > 255, then vf set to 1
                            if (chip8.V[EXTRACT_X(opcode)] + chip8.V[EXTRACT_Y(opcode)] > 255) {
                                underflow = 1;
                            }
                            else underflow = 0;

                            // add vy to vx
                            chip8.V[EXTRACT_X(opcode)] = chip8.V[EXTRACT_X(opcode)] + chip8.V[EXTRACT_Y(opcode)];

                            if (underflow) {
                                chip8.V[0xF] = 1;
                            }
                            else chip8.V[0xF] = 0;

                            break;
                        // subtract
                        case 0x0005:
                            // if vx is larger than vy, then vf is set to 1, else vf is set to 0
                            
                            if (chip8.V[EXTRACT_X(opcode)] < chip8.V[EXTRACT_Y(opcode)]) {
                                underflow = 0;
                            }
                            else underflow = 1;
                                
                            // vx is set to vx-vy
                            chip8.V[EXTRACT_X(opcode)] = chip8.V[EXTRACT_X(opcode)] - chip8.V[EXTRACT_Y(opcode)];
                            
                            if (underflow) {
                                chip8.V[0xF] = 1;
                            }
                            else chip8.V[0xF] = 0;

                            break;
                        // subtract
                        case 0x0007:
                            
                            // if vy is larger than vx, then vf is set to 1, else vf is set to 0
                            if (chip8.V[EXTRACT_Y(opcode)] < chip8.V[EXTRACT_X(opcode)]) {
                                underflow = 0;
                            }
                            else underflow = 1;
                            // vx is set to vy-vx
                            chip8.V[EXTRACT_X(opcode)] = chip8.V[EXTRACT_Y(opcode)] - chip8.V[EXTRACT_X(opcode)];
                            
                            if (underflow) {
                                chip8.V[0xF] = 1;
                            }
                            else chip8.V[0xF] = 0;

                            break;

                        // shift
                        // using old behavior here, where vx is set to vy first
                        case 0x0006:
                            // chip8.V[EXTRACT_X(opcode)] = chip8.V[EXTRACT_Y(opcode)];

                            // set VF to the least significant bit of VX
                            carry = chip8.V[EXTRACT_X(opcode)] & 0x01;
                            // shift VX to the right by 1 bit
                            chip8.V[EXTRACT_X(opcode)] >>= 1;

                            chip8.V[0xF] = carry;
                            break;

                        case 0x000E:
                            // chip8.V[EXTRACT_X(opcode)] = chip8.V[EXTRACT_Y(opcode)];

                            // set VF to the most significant bit of VX
                            carry = (chip8.V[EXTRACT_X(opcode)] & 0x80) >> 7;
                            // shift VX to the left by 1 bit
                            chip8.V[EXTRACT_X(opcode)] <<= 1;

                            chip8.V[0xF] = carry;
                            break;
                    }
                    break;

                // jump with offset
                case 0xB000:
                    chip8.pc = chip8.V[0] + EXTRACT_NNN(opcode);
                    break;

                // random number
                case 0xC000:
                    // generate a random number and binary and with NN
                    // store value into vx
                    ;
                    int r = rand() % 99; 
                    r = r & EXTRACT_NN(opcode);
                    chip8.V[EXTRACT_X(opcode)] = r;
                    break;

                // set I
                case 0xA000:
                    // set index register I to NNN
                    chip8.I = EXTRACT_NNN(opcode);
                    break;

                // draw
                case 0xD000: {
                    // draw sprite at coordinate (VX, VY) with N bytes of sprite data starting at the address stored in I

                    // extract x, y, and n
                    // modulo by 64 and 32 so position can wrap
                    uint8_t x = chip8.V[EXTRACT_X(opcode)] % 64;
                    uint8_t y = chip8.V[EXTRACT_Y(opcode)] % 32;
                    uint8_t n = EXTRACT_N(opcode);

                    // set displaywait to 1
                    display_wait = 1;

                    // set register vf to 0
                    chip8.V[0xF] = 0;

                    // loops through n rows
                    for (int row = 0; row < n; row++) {
                        // get the nth byte of sprite data from memory (starts at I)
                        uint8_t sprite_byte = chip8.memory[chip8.I + row];

                        // loop through each 8 pixels of the sprite row
                        for (int col = 0; col < 8; col++) {
                            // get the current screen pixel
                            // initialize as a pointer to the memory address of the coordinates of the screen given
                            // by the sprite starting position, offset by the row and column
                            uint32_t* screen_pixel = &chip8.display[(y + row)][(x + col)];

                            // check to see if the sprite pixel is on
                            if ((sprite_byte & (0x80 >> col))) {
                                // then, if the associated screen pixel is also on, set the VF register to 1
                                // also turn the pixel off
                                if (*screen_pixel == 0xFF) {
                                    *screen_pixel = 0x00;
                                    chip8.V[0xF] = 1;
                                }
                                // otherwise, set the screen pixel on
                                else {
                                    *screen_pixel = 0xFF;
                                }
                            }

                            // hitting the right edge of the screen will stop drawing the current row
                            
                            if (x + col >= 64) {
                                break;
                            }
                            
                            
                        }
    
                        // printf("x: %d, after increment y: %d\n", x, y);
                        // reaching the bottom of the screen will stop
                        
                        if (y + row >= 32) {
                            break;
                        }
                        
                    }
                    break;
                }
                // skip if key
                case 0xE000:
                    switch (opcode & 0x00FF) {
                        // valid keys 0-F
                        case 0x009E:
                            // skip an instruction if the key corresponding to the value in vx is pressed
                            if (keys[chip8.V[EXTRACT_X(opcode)]]) {
                                chip8.pc += 2;
                            }
                            break;
                        case 0x00A1:
                            // skip an instruction if the key corresponding to the value in vx is not pressed
                            if (!keys[chip8.V[EXTRACT_X(opcode)]]) {
                                chip8.pc += 2;
                            }
                            break;
                    }
                
                // timer keys
                case 0xF000:
                    switch (opcode & 0x00FF) {
                        case 0x0007:
                            // sets vx to the current value of the delay timer
                            chip8.V[EXTRACT_X(opcode)] = chip8.delay_timer;
                            break;
                        case 0x0015:
                            // sets delay timer to vx
                            chip8.delay_timer = chip8.V[EXTRACT_X(opcode)];
                            break;
                        case 0x0018:
                            // sets sound timer to vx
                            chip8.sound_timer = chip8.V[EXTRACT_X(opcode)];
                            break;
                        
                        case 0x001E:
                            // add vx to register I
                            chip8.I += chip8.V[EXTRACT_X(opcode)];
                            break;

                        // get key
                        case 0x000A:
                            // stop instructions
                            // aka decrement pc, unless a key is pressed
                            // timers should still decrease
                            // if key is pressed while this is waiting for input,
                            // put hexadecimal value into vx and continue
                            ;
                            chip8.pc -= 2;
                            while (1) {
                                // Check if any key is pressed
                                for (int i = 0; i < 16; i++) {
                                    if (keys[i]) {
                                        // Wait for the key to be released
                                        while (keys[i]) {
                                            // Update the delay and sound timers
                                            if (chip8.delay_timer > 0) {
                                                chip8.delay_timer--;
                                            }
                                            if (chip8.sound_timer > 0) {
                                                chip8.sound_timer--;
                                            }

                                            // Yield the CPU to allow other events to be processed
                                            glfwWaitEventsTimeout(0.016); // Approximately 60Hz
                                        }

                                        // Store the key value in the register and resume execution
                                        chip8.V[EXTRACT_X(opcode)] = i;
                                        chip8.pc += 2;
                                        break;
                                    }
                                }

                                // If no key is pressed, update the timers and yield the CPU
                                if (chip8.pc == opcode - 2) {
                                    if (chip8.delay_timer > 0) {
                                        chip8.delay_timer--;
                                    }
                                    if (chip8.sound_timer > 0) {
                                        chip8.sound_timer--;
                                    }
                                    glfwWaitEventsTimeout(0.016);
                                } else {
                                    break;
                                }
                            }
                            break;
                        
                        // font
                        case 0x0029:
                            // index register I is set to address of hexadecimal character stored in vx
                            // point I to the right font memory address 
                            chip8.I = 0x050 + (chip8.V[EXTRACT_X(opcode)] * 5);
                            break;
                        
                        // binary coded decimal conversion
                        case 0x0033:
                            // take number in vx and convert to three digit decimal
                            // store at memory address I, I+1, I+2
                            ;
                            uint8_t value = chip8.V[EXTRACT_X(opcode)];
                            chip8.memory[chip8.I + 2] = value % 10;
                            value /= 10;
                            chip8.memory[chip8.I + 1] = value % 10;
                            value /= 10;
                            chip8.memory[chip8.I] = value % 10;
                            break;

                        // store and load memory
                        // use a temp value for indexing

                        // OLD BEHAVIOR INCREMENTS I
                        // will not implement here for sake of testing roms
                        case 0x0055:
                            // from registers v0 to vx (get x)
                            // the values of them will be stored in
                            // I, I+1, I+X
                            for (int i = 0; i <= EXTRACT_X(opcode); i++) {
                                chip8.memory[chip8.I + i] = chip8.V[i];
                                // chip8.I++;
                            }
                            break;
                        case 0x0065:
                            // from memory address I, store the 
                            // values in those addresses into
                            // registers v0 to vx
                            for (int i = 0; i <= EXTRACT_X(opcode); i++) {
                                chip8.V[i] = chip8.memory[chip8.I + i];
                                // chip8.I++;
                            }
                            break;
                    }
                

            }

            // implement display wait quirk
            // break the cycle per frame loop after a draw
            /*
            if (opcode & 0xD000) {
                break;
            }
            */

            if (display_wait) {
                break;
            }

        }

        glClear(GL_COLOR_BUFFER_BIT);

        // render the CHIP-8 display using triangles
        glBegin(GL_TRIANGLES);
        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 64; x++) {
                if (chip8.display[y][x]) {
                    // scale the pixel size to 10x10 (adjust as needed)
                    // going to be defining triangles 
                    // top left triangle
                    glVertex2i(x * 10, y * 10);
                    glVertex2i(x * 10 + 10, y * 10);
                    glVertex2i(x * 10, y * 10 + 10);

                    // bottom right triangle
                    glVertex2i(x * 10 + 10, y * 10);
                    glVertex2i(x * 10 + 10, y * 10 + 10);
                    glVertex2i(x * 10, y * 10 + 10);
                }
            }
        }
        glEnd();
        glfwSwapBuffers(window);

        if (delta_time < frame_time) {
            double sleep_time = frame_time - delta_time;
            glfwWaitEventsTimeout(sleep_time);
        }

        prev_time = current_time;

    }

    // end glfw clean
    glfwTerminate();

    return 0;
}