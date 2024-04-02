# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -I.
LDFLAGS = -lglfw3 -lGL -lX11 -lXrandr -lXinerama -lXcursor -lXi -ldl -lm -pthread

# Source files and object files
SRCS = main.c chip8.c
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = chip8

# Default target
all: $(TARGET)

# Linking object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compiling source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target to remove object files and executable
clean:
	rm -f $(OBJS) $(TARGET)