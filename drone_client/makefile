CC = gcc
CFLAGS = -Wall -g -I.
LDFLAGS = -pthread -ljson-c

SOURCES = drone_client.c main.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = drone_client

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean