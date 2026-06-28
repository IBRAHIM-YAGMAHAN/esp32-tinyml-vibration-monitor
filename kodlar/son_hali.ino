#include "FanDetector.h"
#include <Vibration_Anomaly_Detection_inferencing.h>
//wifi ve mqtt
const char* ssid         = "AndroidAP82b7";
const char* wifiPassword = "xpva5555";
const char* mqttServer   = "df8ebb6c6c1e49acbfb78ed3df735c1e.s1.eu.hivemq.cloud";
const int   mqttPort     = 8883;
const char* mqttUser     = "pub_sub";
const char* mqttPassword = "14310014060aA";
const char* topic        = "motor/durum";

WiFiClientSecure secureClient;
PubSubClient client(secureClient);


float filtAx = 0, filtAy = 0, filtAz = 0;
float oldAx  = 0, oldAy  = 0, oldAz  = 0;
const float alpha = 0.3;
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE]; //oto aliyor kendisine en uygununu
unsigned long sonOkumaZamani = 0;
const unsigned long okumaPeriodu = 20;
volatile bool butonBasildi = false;
int anomalyCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  ledcAttach(RED_LED,   5000, 8);
  ledcAttach(GREEN_LED, 5000, 8);
  ledcAttach(BLUE_LED,  5000, 8);

  renkYaz(255, 255, 255); // Baslangicta beyaz

  Wire.begin(21, 22);
  Wire.setClock(100000);
  mpuBaslat();

  wifiBaslat();
  secureClient.setInsecure();
  client.setServer(mqttServer, mqttPort);

  EEPROM.begin(4);
  anomalyCount = EEPROM.read(EEPROM_ADRES);
  Serial.print("Onceki anomaly sayisi: ");
  Serial.println(anomalyCount);

  pinMode(BUTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTON_PIN), butonKesme, FALLING);

  Serial.println("Sistem hazir!");
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();

  int ornekSayisi = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE / 5;
  float sonRms = 0;

  for (int i = 0; i < ornekSayisi; i++) {
    float ax, ay, az;
    if (!mpuOku(ax, ay, az)) { i--; delay(10); continue; }

    filtAx = alpha * ax + (1 - alpha) * filtAx;
    filtAy = alpha * ay + (1 - alpha) * filtAy;
    filtAz = alpha * az + (1 - alpha) * filtAz;

    float farkX = filtAx - oldAx;
    float farkY = filtAy - oldAy;
    float farkZ = filtAz - oldAz;
    float vibration = mySqrt(farkX*farkX + farkY*farkY + farkZ*farkZ);
    float rms       = mySqrt(filtAx*filtAx + filtAy*filtAy + filtAz*filtAz);

    oldAx = filtAx; oldAy = filtAy; oldAz = filtAz;
    sonRms = rms;

    int idx = i * 5;
    features[idx + 0] = filtAx;
    features[idx + 1] = filtAy;
    features[idx + 2] = filtAz;
    features[idx + 3] = vibration;
    features[idx + 4] = rms;

    while (millis() - sonOkumaZamani < okumaPeriodu) {}
    sonOkumaZamani = millis();
  }

  signal_t signal;
  signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
  signal.get_data = &getFeatureData;

  ei_impulse_result_t result = {0};
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
  if (res != EI_IMPULSE_OK) {
    Serial.print("Classifier hatasi: ");
    Serial.println(res);
    return;
  }

  float maxVal = 0;
  const char* tahmin = "";
  for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    if (result.classification[i].value > maxVal) {
      maxVal = result.classification[i].value;
      tahmin = result.classification[i].label;
    }
  }

  Serial.println("--- Siniflandirma ---");
  for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    Serial.print("  ");
    Serial.print(result.classification[i].label);
    Serial.print(": %");
    Serial.println(result.classification[i].value * 100, 1);
  }
  Serial.print("SONUC: ");
  Serial.print(tahmin);
  Serial.print(" (%");
  Serial.print(maxVal * 100, 1);
  Serial.println(")");

  renkGuncelle(tahmin, sonRms);
  mqttGonder(tahmin, maxVal * 100, sonRms, anomalyCount);

  if (String(tahmin) == "anomaly") {
    anomalyCount++;
    EEPROM.write(EEPROM_ADRES, anomalyCount);
    EEPROM.commit();
    Serial.print("Toplam anomaly: ");
    Serial.println(anomalyCount);
  }

  if (butonBasildi) {
    butonBasildi = false;
    anomalyCount = 0;
    EEPROM.write(EEPROM_ADRES, 0);
    EEPROM.commit();
    Serial.println("Anomaly sayaci sifırlandi!");
  }
}