#ifndef MISSION_H
#define MISSION_H

#define MAX_MISSIONS 100

typedef struct {
    int id;                 // Görev ID'si
    int assigned_drone_id;  // Görevi üstlenen drone ID'si
    int target_x;          // Hedef X koordinatı
    int target_y;          // Hedef Y koordinatı
    int status;            // Görev durumu
    int priority;          // Görev önceliği
} Mission;

extern Mission mission_list[MAX_MISSIONS]; // Görev listesi
extern int mission_list_size;             // Görev listesi boyutu

// Görev durumları için enum
enum MissionStatus {
    MISSION_PENDING,    // Beklemede
    MISSION_ACTIVE,     // Aktif/Devam ediyor
    MISSION_COMPLETED,  // Tamamlandı
    MISSION_FAILED      // Başarısız
};

#endif