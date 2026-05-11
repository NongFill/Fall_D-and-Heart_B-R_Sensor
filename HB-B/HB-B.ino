#include <Arduino.h>
#include "Seeed_Arduino_mmWave.h"
#include <HardwareSerial.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// --- Bluetooth Setup ---
#define DEVICE_NAME         "......" 
#define SERVICE_UUID        "......"
#define CHARACTERISTIC_UUID "......"

// Connect BLE
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println(" มือถือเชื่อมต่อแล้ว!");
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println(" มือถือตัดการเชื่อมต่อ!");
    BLEDevice::startAdvertising(); 
  }
};

// --- mmWave Setup ---
HardwareSerial mmWaveSerial(0); 
SEEED_MR60BHA2 mmWave;

void setup() {
  Serial.begin(115200);
  
  // Initialize Bluetooth
  BLEDevice::init(DEVICE_NAME);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID,
                    BLECharacteristic::PROPERTY_READ | 
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
  
  pService->start();
  BLEDevice::startAdvertising();
  Serial.println("รอการเชื่อมต่อจากแอป...");

  // Initialize mmWave
  mmWave.begin(&mmWaveSerial);
  Serial.println("เซนเซอร์พร้อมทำงาน!");
}

//
void loop() {
  if (mmWave.update(1000)) {
    if (mmWave.isHumanDetected()) {
      Serial.println("Human Detected ✅");

      float total_phase, breath_phase, heart_phase;
      if (mmWave.getHeartBreathPhases(total_phase, breath_phase, heart_phase)) {
        Serial.printf("heart_phase: %.2f | breath_phase: %.2f | total_phase: %.2f\n", heart_phase , breath_phase ,total_phase);
      }
       Serial.println("----------------------------------------");
      float breath_rate;
      if (mmWave.getBreathRate(breath_rate)) {
        Serial.printf("Breath_rate: %.2f BPM\n", breath_rate);
      }
      // Logic
      float heart_rate;
      if (mmWave.getHeartRate(heart_rate)) {
        Serial.printf("Heart_rate: %.2f BPM  ", heart_rate);
        if (heart_rate > 150) {
          Serial.println("🔴 อันตราย! หัวใจเต้นเร็วมาก");
        }
        else if (heart_rate > 100) {
          Serial.println("🟡 หัวใจเต้นเร็ว"); 
        }
        else if (heart_rate < 50 && heart_rate > 0) {
          Serial.println("🔵 หัวใจเต้นช้าเกิน ");
        }
        else if (heart_rate >= 50 && heart_rate <= 100) {
          Serial.println("🟢 หัวใจอยู่ในช่วงปกติ ");
        }
        if (heart_rate <= 0) {
          Serial.println("💔 ไม่พบสัญญาณหัวใจ");
        }
         Serial.println("----------------------------------------");
      }
      float distance;
      if (mmWave.getDistance(distance)) {
        Serial.printf("distance: %.2f m\n", distance);
      }
       Serial.println("----------------------------------------");
     }else {Serial.println("No Human ❌");}
  }
}
