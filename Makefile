CC = gcc
CFLAGS = -Wall -Iheaders
SDLFLAGS = `sdl2-config --cflags --libs`

# ==== Sunucu (Server) ====
SERVER_SRC = server.c ai.c map.c list.c drone.c survivor.c
SERVER_OBJ = $(SERVER_SRC:.c=.o)
SERVER_TARGET = server

# ==== Drone İstemci (Client) ====
CLIENT_SRC = drone_client/drone_client.c drone_client/main.c drone_client/drone_manager.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
CLIENT_TARGET = drone_client_exec

# ==== Liste Testi ====
LIST_TEST_SRC = tests/listtest.c list.c
LIST_TEST_OBJ = $(LIST_TEST_SRC:.c=.o)
LIST_TEST_TARGET = listtest

# ==== Görsel Viewer (SDL ile) ====
VIEWER_SRC = client_viewer.c map.c list.c view.c
VIEWER_OBJ = $(VIEWER_SRC:.c=.o)
VIEWER_TARGET = viewer_test

# ==== Varsayılan Derleme ====
all: $(SERVER_TARGET) $(CLIENT_TARGET) $(LIST_TEST_TARGET) $(VIEWER_TARGET)

# ==== Hedefler ====
$(SERVER_TARGET): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread -ljson-c

$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -ljson-c -lm -lpthread

$(LIST_TEST_TARGET): $(LIST_TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

$(VIEWER_TARGET): $(VIEWER_OBJ)
	$(CC) $(CFLAGS) $(SDLFLAGS) -o $@ $^

# ==== Temizlik ====
clean:
	find . -name "*.o" -type f -delete
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET) $(LIST_TEST_TARGET) $(VIEWER_TARGET)

.PHONY: all clean