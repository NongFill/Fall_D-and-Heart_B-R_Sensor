# Fall Detection & Heart Rate Monitor
### ระบบตรวจจับการล้มและติดตามสัญญาณชีพด้วย mmWave Sensor

---

## 📋 Project Overview / ภาพรวมโปรเจกต์

This repository contains two Arduino projects using **Seeed Studio mmWave sensors** for elderly care and health monitoring. Both projects communicate via **Bluetooth (BLE)** to a mobile application.

โปรเจกต์นี้ประกอบด้วยระบบ 2 ส่วน ที่ใช้เซนเซอร์ **Seeed Studio mmWave** สำหรับดูแลผู้สูงอายุและติดตามสุขภาพ โดยทั้งสองส่วนเชื่อมต่อกับแอปมือถือผ่าน **Bluetooth (BLE)**

---

## 📁 Project Structure / โครงสร้างโปรเจกต์

```
Fall_D-and-Heart_B-R_Sensor/
├── Fall_D/
│   └── Fall_D.ino          ← Fall Detection / ระบบตรวจจับการล้ม
└── HB-B/
    └── HB-B.ino            ← Heart Rate & Breathing Monitor / ระบบวัดอัตราการเต้นของหัวใจและการหายใจ
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

### ภาษาไทย
ระบบตรวจจับการล้มโดยใช้เซนเซอร์ **Seeed MR60FDA2** ตรวจจับการเคลื่อนไหวในพื้นที่ เมื่อยืนยันว่ามีการล้มเกิดขึ้น ระบบจะส่งการแจ้งเตือนไปยัง **Firebase Firestore** และแจ้งเตือนแอปมือถือผ่าน **BLE**

**ฟีเจอร์หลัก:**
- ตรวจจับการล้มพร้อมระบบยืนยัน (ลด False Alarm)
- บันทึก Log การล้มไปยัง Firebase Firestore พร้อมเวลาและ Zone ID
- ปรับค่าตัวเซนเซอร์ผ่าน BLE ได้แบบ OTA (ความสูง, Threshold, ความไว)
- จำค่าการตั้งค่าไว้ใน Flash Memory (ค่าไม่หายเมื่อรีบูต)
- ตรวจสอบและเชื่อมต่อ WiFi ใหม่อัตโนมัติ

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

### ภาษาไทย
ระบบติดตามสัญญาณชีพโดยใช้เซนเซอร์ **Seeed MR60BHA2** วัดอัตราการเต้นของหัวใจและการหายใจแบบไม่ต้องสัมผัส และส่งข้อมูลแบบ Real-time ไปยังแอปมือถือผ่าน **BLE**

**ฟีเจอร์หลัก:**
- วัดอัตราการเต้นของหัวใจและการหายใจแบบไม่ต้องสัมผัสร่างกาย
- แบ่งระดับสถานะอัตราการเต้นของหัวใจ (ปกติ / เร็ว / ช้า / อันตราย)
- ตรวจจับการมีคนอยู่ก่อนเริ่มวัดค่า
- ส่งข้อมูลแบบ Real-time ผ่าน BLE

**Hardware:** Seeed MR60BHA2 + ESP32

---

## 🛠️ Dependencies / ไลบรารีที่ใช้

- [Seeed Arduino mmWave](https://github.com/Seeed-Studio/Seeed_Arduino_mmWave)
- [Firebase ESP Client](https://github.com/mobizt/Firebase-ESP-Client)
- ESP32 BLE Arduino (built-in)
- WiFi (built-in)

---
