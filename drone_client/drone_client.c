#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "headers/drone.h"
#include "headers/coord.h"

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = {.sin_family = AF_INET,
                                    .sin_port = htons(8080),
                                    .sin_addr.s_addr = inet_addr("127.0.0.1")};
    
    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    while (1) {
        // Send status update
        DroneStatus status = {.x = 0, .y = 0, .status = IDLE};
        send(sock, &status, sizeof(status), 0);
        
        // Receive mission
        Coord target;
        recv(sock, &target, sizeof(target), 0);
        
        // TODO: Implement navigation logic
        sleep(1);
    }
    
    close(sock);
    return 0;
}