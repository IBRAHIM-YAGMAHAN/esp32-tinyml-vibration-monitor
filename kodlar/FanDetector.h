#ifndef FAN_DETECTOR_H
#define FAN_DETECTOR_H

#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <EEPROM.h>


#define MPU_ADDR  0x68
#define GREEN_LED 25
#define RED_LED   26
#define BLUE_LED 14
#define BUTON_PIN 18
#define EEPROM_ADRES 0  // anomaly sayisini hangi adrese kaydet

//wifi ve mqtt
extern const char* ssid;        
extern const char* wifiPassword;
extern const char* mqttServer;
extern const int  mqttPort;
extern const char* mqttUser;
extern const char* mqttPassword;
extern const char* topic;
//wifi ve mqtt nesneleri
extern WiFiClientSecure secureClient;
extern PubSubClient client;
//golobal degiskenler icin
extern float filtAx, filtAy, filtAz;
extern float oldAx, oldAy, oldAz;
extern const float alpha;
extern float features[];
extern unsigned long sonOkumaZamani;
extern const unsigned long okumaPeriodu;
extern volatile bool butonBasildi;
extern int anomalyCount;

//fonksiyon imzalari
float mySqrt(float sayi);
void mpuBaslat();
bool mpuOku(float &ax, float &ay, float &az);
int getFeatureData(size_t offset, size_t length, float *out_ptr);
void renkYaz(int r, int g, int b);
void renkGuncelle(const char* tahmin, float rms);
void wifiBaslat();
void reconnectMQTT();
void mqttGonder(const char* sonuc, float guven, float rms, int anomalyCount);
void IRAM_ATTR butonKesme();

#endif
