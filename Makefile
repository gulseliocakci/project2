# ==== Derleyici ve Bayraklar ====
CC = gcc
CFLAGS = -Wall -Iheaders -lpthread -ljson-c `sdl2-config --cflags --libs`

# ==== Sunucu ve Ana Program Kaynakları ====
SERVER_SRC = server.c ai.c map.c view.c list.c
SERVER_OBJ = $(SERVER_SRC:.c=.o)
SERVER_TARGET = server_main

# ==== Drone İstemci Kaynakları ====
CLIENT_SRC = drone_client.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
CLIENT_TARGET = drone_client

# ==== Survivor Üretici Testi ====
SURVIVOR_GEN_SRC = survivor_generator.c list.c map.c
SURVIVOR_GEN_OBJ = $(SURVIVOR_GEN_SRC:.c=.o)
SURVIVOR_GEN_TARGET = survivor_generator_test

# ==== Liste Testi ====
LIST_TEST_SRC = tests/list_test.c list.c
LIST_TEST_OBJ = $(LIST_TEST_SRC:.c=.o)
LIST_TEST_TARGET = list_test

# ==== SDL Viewer Testi (opsiyonel) ====
VIEWER_TEST_SRC = tests/viewer_test.c view.c map.c list.c
VIEWER_TEST_OBJ = $(VIEWER_TEST_SRC:.c=.o)
VIEWER_TEST_TARGET = viewer_test

# ==== Varsayılan Derleme ====
all: $(SERVER_TARGET) $(CLIENT_TARGET) $(SURVIVOR_GEN_TARGET) $(LIST_TEST_TARGET) $(VIEWER_TEST_TARGET)

# ==== Ana Sunucu ====
$(SERVER_TARGET): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# ==== Drone Client ====
$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# ==== Survivor Producer Test ====
$(SURVIVOR_GEN_TARGET): $(SURVIVOR_GEN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# ==== Liste Testi ====
$(LIST_TEST_TARGET): $(LIST_TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# ==== Viewer Testi ====
$(VIEWER_TEST_TARGET): $(VIEWER_TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# ==== Temizlik ====
clean:
	rm -f *.o $(SERVER_TARGET) $(CLIENT_TARGET) $(SURVIVOR_GEN_TARGET) $(LIST_TEST_TARGET) $(VIEWER_TEST_TARGET)

.PHONY: all clean