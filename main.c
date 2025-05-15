#include <pthread.h>
#include "headers/drone_manager.h"  // Sunucu tarafı drone yönetimi
#include "headers/client_viewer.h" // İstemci tarafı görselleştirme
#include <stdio.h>
#include <unistd.h>

int main() {
    // Sunucu portu
    int port = 8080;

    // Görselleştirme için bir iş parçacığı oluşturuluyor
    pthread_t viewer_thread;

    // Görselleştirme modülünü ayrı bir iş parçacığında başlat
    if (pthread_create(&viewer_thread, NULL, (void*)start_client_viewer, NULL) != 0) {
        fprintf(stderr, "Görselleştirme iş parçacığı başlatılamadı.\n");
        return 1;
    }

    // Drone sunucusunu başlat (Phase 2'ye uygun şekilde)
    start_drone_server(port);

    // Görselleştirme iş parçacığının tamamlanmasını bekle (isteğe bağlı)
    pthread_join(viewer_thread, NULL);

    return 0;
}