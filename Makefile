CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lSDL2 -lm -pthread -ljson-c

# Source files
SOURCES = main.c controller.c globals.c list.c drone.c survivor.c ai.c view.c map.c server.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = drone_simulator

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Cleaned build files"

# Force rebuild
rebuild: clean all

.PHONY: all clean rebuild