CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lSDL2 -lm -pthread

SOURCES = controller.c globals.c list.c drone.c survivor.c ai.c view.c map.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = drone_simulator

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean