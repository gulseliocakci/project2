# Compiler ve flags
CC = gcc
CFLAGS = -Wall -Iheaders -lpthread `sdl2-config --cflags` -ljson-c `sdl2-config --libs`

# Kaynak dosyalar
SRC = server.c main.c ai.c view.c map.c
OBJ = $(SRC:.c=.o)

# Hedef program
TARGET = main_program

# Varsayılan hedef
all: $(TARGET)

# Hedef programı oluşturma
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# .c dosyalarını .o dosyalarına dönüştürme
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Temizlik
clean:
	rm -f $(OBJ) $(TARGET)