# -*- coding: utf-8 -*-
"""
Created on Sun May 10 23:43:42 2026

@author: ibrah
"""
import serial
import time

PORT = "COM10"
BAUD = 115200
DOSYA_ADI = "anomaly.csv"
KAYIT_SURESI = 60  # saniye, 3 dakika

ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)

baslangic = time.time()

with open(DOSYA_ADI, "w", encoding="utf-8") as f:
    print("Kayit basladi...")

    while time.time() - baslangic < KAYIT_SURESI:
        satir = ser.readline().decode("utf-8", errors="ignore").strip()

        if satir:
            print(satir)
            f.write(satir + "\n")

ser.close()
print("Kayit bitti:", DOSYA_ADI)