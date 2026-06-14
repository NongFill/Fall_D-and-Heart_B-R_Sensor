# Fall Detection & Heart Rate Monitor
---

## 📋 Project Overview 

This repository contains two Arduino projects using **Seeed Studio mmWave sensors** for elderly care and health monitoring. Both projects communicate via **Bluetooth (BLE)** to a mobile application.

## 📁 Project Structure 

```
Fall_D-and-Heart_B-R_Sensor/
├── Fall_D/
│   └── Fall_D.ino          ← Fall Detection 
└── HB-B/
    └── HB-B.ino            ← Heart Rate & Breathing Monitor 
```

---

## 🧓🏻 Project 1: Fall Detection (Fall_D)

### English
A fall detection system using the **Seeed MR60FDA2** mmWave sensor. The device monitors a room and detects whether a person has fallen. When a fall is confirmed, it sends an alert to **Firebase Firestore** and notifies a connected mobile app via **BLE**.

**Key Features:**
- Fall detection with confirmation threshold (reduces false alarms)
- Sends fall event logs to Firebase Firestore with timestamp and zone ID
- OTA configuration via BLE (height, threshold, sensitivity)
- Persistent memory using Flash (settings saved after reboot)
- Auto WiFi reconnect monitoring

**Hardware:** Seeed MR60FDA2 + ESP32

---

## ♥️ Project 2: Heart Rate & Breathing Monitor (HB-B)

### English
A vital signs monitoring system using the **Seeed MR60BHA2** mmWave sensor. The device measures heart rate and breathing rate without physical contact, and sends real-time data to a connected mobile app via **BLE**.

**Key Features:**
- Non-contact heart rate and breathing rate measurement
- Heart rate status classification (Normal / Fast / Slow / Danger)
- Human presence detection before measuring
- Real-time data notification via BLE

**Hardware:** Seeed MR60BHA2 + ESP32

---

## 🛠️ Dependencies 

- [Seeed Arduino mmWave](https://github.com/Seeed-Studio/Seeed_Arduino_mmWave)
- [Firebase ESP Client](https://github.com/mobizt/Firebase-ESP-Client)
- ESP32 BLE Arduino (built-in)
- WiFi (built-in)

---
