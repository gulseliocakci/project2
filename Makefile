CC = gcc
CFLAGS = -Wall -Wextra -pthread -I./headers -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE
LDFLAGS = -lSDL2 -lm

# Kaynak dosyalar
SRCS = main.c globals.c list.c drone.c survivor.c ai.c view.c map.c
OBJS = $(SRCS:.c=.o)
TARGET = drone_simulator

# Test dosyaları
TEST_SRCS = test.c globals.c list.c drone.c survivor.c ai.c view.c map.c
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_TARGET = test_simulator

.PHONY: all clean test

# Varsayılan hedef
all: $(TARGET)

# Ana program
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Test programı
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(TEST_OBJS) -o $(TEST_TARGET) $(LDFLAGS)

# Nesne dosyaları
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Temizleme
clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET)

# Her ikisini de derle
all-targets: $(TARGET) $(TEST_TARGET)