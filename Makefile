CC = gcc
CFLAGS = -Wall -Iheaders
SDLFLAGS = `sdl2-config --cflags --libs`

# ==== Sunucu (Server) ====
SERVER_SRC = server.c ai.c map.c list.c drone.c survivor.c globals.c
SERVER_TARGET = server

# ==== Drone İstemci (Client) ====
CLIENT_SRC = drone_client/drone_client.c drone_client/main.c drone_client/drone_manager.c
CLIENT_TARGET = drone_client_exec

# ==== Liste Testi ====
LIST_TEST_SRC = tests/listtest.c list.c
LIST_TEST_TARGET = listtest

# ==== Görsel Viewer (server/general, SDL ile) ====
VIEWER_SRC = view.c map.c list.c drone.c survivor.c globals.c
VIEWER_TARGET = viewer_test

# ==== Görsel Client Viewer (SDL ile) ====
CLIENT_VIEWER_SRC = client_viewer.c
CLIENT_VIEWER_TARGET = client_viewer_exec

# ==== Varsayılan Derleme ====
all: $(SERVER_TARGET) $(CLIENT_TARGET) $(LIST_TEST_TARGET) $(VIEWER_TARGET) $(CLIENT_VIEWER_TARGET)

$(SERVER_TARGET): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread -ljson-c

$(CLIENT_TARGET): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $@ $^ -ljson-c -lm -lpthread

$(LIST_TEST_TARGET): $(LIST_TEST_SRC)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

$(VIEWER_TARGET): $(VIEWER_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(SDLFLAGS)

$(CLIENT_VIEWER_TARGET): $(CLIENT_VIEWER_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(SDLFLAGS)

clean:
	find . -name "*.o" -type f -delete
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET) $(LIST_TEST_TARGET) $(VIEWER_TARGET) $(CLIENT_VIEWER_TARGET)

.PHONY: all clean