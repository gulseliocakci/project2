#TODO
CC = gcc
CFLAGS = -Wall -Iheaders -lpthread -ljson-c
SRC = server.c main.c headers/*.c
OBJ = $(SRC:.c=.o)
TARGET = main_program

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)