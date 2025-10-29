# Compiler and Linker
CC = gcc

# Compiler flags to enable warnings and error checking
CFLAGS = -Wall -Wextra --coverage
LDFLAGS = -pthread -lgcov

# Project name and source files
TARGET = fire_guard
SRCS = bbb_dht_read.c bbb_mmio.c common_dht_read.c gpio.c main.c
HEADERS = bbb_dht_read.h bbb_mmio.h common_dht_read.h gpio.h

# Object files
OBJS = $(SRCS:.c=.o)

# Default rule (build the target)
all: $(TARGET)

# Rule to link the object files into the executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule to compile each .c file into .o object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove generated files
clean:
	rm -f $(OBJS) $(TARGET) *.gcda *.gcno *.gcov

# Rule to check for dependencies
.PHONY: all clean
