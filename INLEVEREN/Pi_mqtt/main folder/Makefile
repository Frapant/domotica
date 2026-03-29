# --- 1. Compiler Settings ---
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Wconversion
LIBS = -lmosquitto -lmariadb
TARGET = main

# --- 2. Path Configuration ---
# Set these to 'yes' to include, 'no' to disable
USE_PARENT = yes
USE_PATH1 = yes
USE_PATH2 = yes
USE_PATH3 = yes

# Define the actual directory names
DIR1 = reamo
DIR2 = frap
DIR3 = silent-ghost

# --- 3. Logic to Collect Sources ---
SRCS = $(wildcard *.c)
INCLUDES = -I.

# Add Parent Folder sources
ifeq ($(USE_PARENT), yes)
    SRCS += $(wildcard ../*.c)
    INCLUDES += -I..
endif

# Add Subdirectory sources
ifeq ($(USE_PATH1), yes)
    SRCS += $(wildcard $(DIR1)/*.c)
    INCLUDES += -I$(DIR1)
endif

ifeq ($(USE_PATH2), yes)
    SRCS += $(wildcard $(DIR2)/*.c)
    INCLUDES += -I$(DIR2)
endif

ifeq ($(USE_PATH3), yes)
    SRCS += $(wildcard $(DIR3)/*.c)
    INCLUDES += -I$(DIR3)
endif

# --- 4. Object Generation ---
# Note: This handles the paths automatically
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

# Link Step
$(TARGET): $(OBJS)
        $(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

# Compilation Step
# This rule tells make how to handle files regardless of which folder they are in
%.o: %.c
        $(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
        rm -f $(TARGET) $(OBJS)
        rm -f ../*.o $(DIR1)/*.o $(DIR2)/*.o $(DIR3)/*.o
        @echo "Cleaned all objects and binaries."