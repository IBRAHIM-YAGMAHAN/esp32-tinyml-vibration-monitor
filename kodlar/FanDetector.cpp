#include "FanDetector.h"

// karekok hesabi (newton-raphson) 
float mySqrt(float sayi) {
  if (sayi <= 0) return 0;
  float tahmin = sayi / 2.0;
  for (int i = 0; i < 20; i++) {
    tahmin = (tahmin + sayi / tahmin) / 2.0;
  }
  return tahmin;
}

void mpuBaslat() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
  delay(200);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C);
  Wire.write(0x00);
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1A);
  Wire.write(0x05);
  Wire.endTransmission(true);
  delay(200);
}

bool mpuOku(float &ax, float &ay, float &az) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(MPU_ADDR, 6, true) != 6) return false;

  int16_t axR = (Wire.read() << 8) | Wire.read();
  int16_t ayR = (Wire.read() << 8) | Wire.read();
  int16_t azR = (Wire.read() << 8) | Wire.read();

  ax = axR / 16384.0;
  ay = ayR / 16384.0;
  az = azR / 16384.0;
  return true;
}

//edge impluse callback
int getFeatureData(size_t offset, size_t length, float *out_ptr) {
  memcpy(out_ptr, features + offset, length * sizeof(float));
  return 0;
}

void renkYaz(int r, int g, int b) {
  ledcWrite(RED_LED,   r);  
  ledcWrite(GREEN_LED, g);
  ledcWrite(BLUE_LED,  b);
}

void renkGuncelle(const char* tahmin, float rms) {
  Serial.print("LED icin gelen tahmin: ");
  Serial.println(tahmin);

  if (strcmp(tahmin, "fan_off") == 0) {
    renkYaz(0, 0, 255);   // mavi
    return;
  }

  if (strcmp(tahmin, "anomaly") == 0) {
    renkYaz(255, 0, 0);   // kirmizi
    return;
  }

  if (strcmp(tahmin, "normal") == 0) {
    renkYaz(0, 255, 0);   // yesil
    return;
  }

  // Bilinmeyen sinif gelirse beyaz yak
  renkYaz(255, 255, 255);
}

void wifiBaslat() {
  Serial.print("WiFi baglaniyor");
  WiFi.begin(ssid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi baglandi! IP: " + WiFi.localIP().toString());
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.println("MQTT baglaniyor...");
    if (client.connect("ESP32_Fan_AI", mqttUser, mqttPassword)) {
      Serial.println("MQTT baglandi!");
    } else {
      Serial.print("Hata rc=");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

void mqttGonder(const char* sonuc, float guven, float rms, int anomalyCount) {
  String payload = "{";
  payload += "\"status\":\"" + String(sonuc) + "\",";
  payload += "\"guven\":" + String(guven, 1) + ",";
  payload += "\"rms\":" + String(rms, 4) + ",";
  payload += "\"anomaly_count\":" + String(anomalyCount);
  payload += "}";
  client.publish(topic, payload.c_str());
  Serial.println("MQTT: " + payload);
}

void IRAM_ATTR butonKesme() {
  butonBasildi = true;
}