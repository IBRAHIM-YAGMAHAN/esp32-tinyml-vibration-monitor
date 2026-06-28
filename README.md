# esp32-tinyml-vibration-monitor
A TinyML-powered embedded system for real-time vibration anomaly detection in fan/motor systems. 
Built on ESP32 with an MPU6050 accelerometer, the system classifies vibration states (normal, 
anomaly, fan_off) directly on the edge — no cloud dependency. Trained via Edge Impulse, achieving 
99.28% accuracy. Results are streamed via MQTT to a Node-RED dashboard with email alerting and 
EEPROM-based anomaly logging.
