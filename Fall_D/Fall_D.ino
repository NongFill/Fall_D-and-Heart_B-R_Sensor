#include <Arduino.h>
#include "Seeed_Arduino_mmWave.h"
#include <HardwareSerial.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <time.h>
#include <Preferences.h> 

// เปลี่ยน ZONE_ID ตามแต่ล้ะตัวที่เขียนเชื่อมฝั่ง Android
#define ZONE_ID ".........."

// WiFi 
const char* ssid     = "......";
const char* password = "......";

// Firebase 
#define API_KEY      ".........."
#define PROJECT_ID   ".........."
#define DATABASE_URL ".........."

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Cooldown 
unsigned long lastFallSendTime = 0;
const unsigned long FIREBASE_COOLDOWN = 15000;

// WiFi Monitor Config
bool wifiWasConnected = false;
unsigned long lastWifiCheck = 0;
const unsigned long WIFI_CHECK_INTERVAL = 5000;

// Bluetooth Connect
#define SERVICE_UUID        "........."
#define CHARACTERISTIC_UUID "........."

BLECharacteristic *pCharacteristic;
bool deviceConnected  = false;
bool forceUpdate      = false;
bool blePendingLog    = false;  

// ค่าที่จะตั้งค่า ตัวอุปกรณ์ Seeed Studio
// mmWave 
float    height      = 2.2;
float    threshold   = 0.4;
uint32_t sensitivity = 8;

Preferences preferences; 
HardwareSerial mmwaveSerial(0);
SEEED_MR60FDA2 mmWave;

// Status 
typedef enum { NO_PEOPLE, PEOPLE_IN_AREA, PEOPLE_FALL } MMWAVE_STATUS;
MMWAVE_STATUS status     = NO_PEOPLE;
MMWAVE_STATUS lastStatus = NO_PEOPLE;

int fallCounter = 0;
const int FALL_CONFIRM_THRESHOLD = 3;

#define MOTION_CONFIRM_TIME 1200
#define LOST_CONFIRM_TIME   2500
#define PRINT_INTERVAL      600

unsigned long motionStartTime = 0;
unsigned long lostStartTime   = 0;
unsigned long lastPrintTime   = 0;

bool human_confirmed = false;

// ข้อมูล TIME & Day 
String getTimeTH() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "00:00:00";
  char buffer[10];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
  return String(buffer);
}

String getDateTH() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "2026-01-01";
  char buffer[15];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
  return String(buffer);
}

//  ระบบจำค่าที่บันทึกไว ของ Flash Memory
void loadSettings() {
  preferences.begin("fall_cfg", true); 
  height      = preferences.getFloat("h", 2.2);
  threshold   = preferences.getFloat("t", 0.4);
  sensitivity = preferences.getUInt("s", 8);
  preferences.end();
  Serial.printf(" Loaded Saved Config -> H:%.2f T:%.2f S:%d\n", height, threshold, sensitivity);
}

void saveSettings() {
  preferences.begin("fall_cfg", false); 
  preferences.putFloat("h", height);
  preferences.putFloat("t", threshold);
  preferences.putUInt("s", sensitivity);
  preferences.end();
  Serial.println(" Config saved to Flash Memory!");
}

//  Report Config to BLE 
void reportCurrentConfigToBLE() {
  if (deviceConnected) {
    char configMsg[64];
    snprintf(configMsg, sizeof(configMsg), "H:%.1f|T:%.1f|S:%d", height, threshold, (int)sensitivity);
    pCharacteristic->setValue(configMsg);
    pCharacteristic->notify();
    Serial.printf("📡 Reported Config: %s\n", configMsg);
  }
}

// Send Log -> Firestore
void sendLog(String event, String detail = "") {
  if (!Firebase.ready()) return;
  String date = getDateTH();
  String timeStr = getTimeTH();
  String path = "FallDetected/Log/" + date + "/" + timeStr;
  FirebaseJson content;
  content.set("fields/event/stringValue", event);
  if (detail != "") content.set("fields/detail/stringValue", detail);
  content.set("fields/zone/stringValue", ZONE_ID);
  content.set("fields/time/stringValue", timeStr);
  content.set("fields/date/stringValue", date);
  Firebase.Firestore.createDocument(&fbdo, PROJECT_ID, "", path.c_str(), content.raw());
}

//Send Fall -> Firestore 
void sendFallToFirebase() {
  if (!Firebase.ready()) return;

  String date = getDateTH();
  String timeStr = getTimeTH();
  
  // สร้าง Path 
  String path = "FallDetected/Log/" + date + "/" + timeStr;

  FirebaseJson content;

  // ข้อมูลพื้นฐาน
  content.set("fields/event/stringValue", "FALL_DETECTED");
  content.set("fields/zone/stringValue", ZONE_ID);
  content.set("fields/time/stringValue", timeStr);
  content.set("fields/date/stringValue", date);

  // การเก็บค่า Config ขณะล้ม
  content.set("fields/config/mapValue/fields/height/doubleValue", (double)height);
  content.set("fields/config/mapValue/fields/threshold/doubleValue", (double)threshold);
  content.set("fields/config/mapValue/fields/sensitivity/integerValue", (int)sensitivity);

  // ส่งข้อมูลไปที่ Firestore
  if (Firebase.Firestore.createDocument(&fbdo, PROJECT_ID, "", path.c_str(), content.raw())) {
    Serial.println("🔥 Fall Log with Config sent to Firestore!");
  } else {
    Serial.println("❌ Firestore Error: " + fbdo.errorReason());
  }
}

//Callbacks BLE -> OTA 
class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    if (value.startsWith("SET:")) {
      if (value.indexOf("H=") != -1) {
        int hIdx = value.indexOf("H=") + 2;
        int endIdx = value.indexOf(",", hIdx);
        if (endIdx == -1) endIdx = value.length();
        height = value.substring(hIdx, endIdx).toFloat();
        mmWave.setInstallationHeight(height);
      }
      if (value.indexOf("T=") != -1) {
        int tIdx = value.indexOf("T=") + 2;
        int endIdx = value.indexOf(",", tIdx);
        if (endIdx == -1) endIdx = value.length();
        threshold = value.substring(tIdx, endIdx).toFloat();
        mmWave.setThreshold(threshold);
      }
      if (value.indexOf("S=") != -1) {
        int sIdx = value.indexOf("S=") + 2;
        sensitivity = value.substring(sIdx).toInt();
        mmWave.setSensitivity(sensitivity);
      }
      
      saveSettings(); 
      
      Serial.println("OTA Updated Success!");
      reportCurrentConfigToBLE();
    }
  }
};
// Log Callbacks BLE
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    forceUpdate = true; 
    Serial.println("BLE Connected ✅");
  }
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    blePendingLog = true;
    BLEDevice::startAdvertising();
    Serial.println("BLE Disconnected ❌");
  }
};

// SETUP 
void setup() {
  Serial.begin(115200);
  delay(1500);

  // เรียกใช้ค่าจาก Flash Memory ก่อน
  loadSettings(); 

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi Connected!");
  wifiWasConnected = true;

  // ดึงเวลา
  configTime(7 * 3600, 0, "pool.ntp.org");

  // Values ​​in Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  // Auth Firebase
  auth.user.email = "......";         
  auth.user.password = ".....";      
  fbdo.setResponseSize(1024); 
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  while (!Firebase.ready()) { delay(300); Serial.print("."); }
  Serial.println("\nFirebase Ready!");

  //Setting BLE
  BLEDevice::init(ZONE_ID);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  // เชื่อมต่อ Sensor MMwave
  mmwaveSerial.setRxBufferSize(512); 
  mmwaveSerial.begin(115200);
  mmWave.begin(&mmwaveSerial);

  // สั่งค่าเข้าเซนเซอร์โดยใช้ค่าที่โหลดมาจากความจำถาวร
  mmWave.setInstallationHeight(height);
  mmWave.setThreshold(threshold);
  mmWave.setSensitivity(sensitivity);

  Serial.println(" Fall Detector Ready with Persistent Memory!");
}

// LOOP 
void loop() {
  bool is_human = false;
  bool is_fall  = false;

  while (mmwaveSerial.available()) { mmWave.update(mmwaveSerial.read()); }
  mmWave.getHuman(is_human);
  mmWave.getFall(is_fall);
  unsigned long now = millis();

  // WIFI Check & Auto Connect
  if (now - lastWifiCheck >= WIFI_CHECK_INTERVAL) {
    lastWifiCheck = now;
    if (WiFi.status() != WL_CONNECTED) {
      if (wifiWasConnected) { wifiWasConnected = false; sendLog("WIFI_DISCONNECTED"); }
      WiFi.reconnect();
    } else if (!wifiWasConnected) { wifiWasConnected = true; }
  }

  // BLE Log Sync 
  if (blePendingLog && Firebase.ready()) { sendLog("BLE_DISCONNECTED"); blePendingLog = false; }

  // HUMAN DETECTION 
  if (is_human) {
    lostStartTime = 0;
    if (!human_confirmed) {
      if (motionStartTime == 0) motionStartTime = now;
      if (now - motionStartTime >= MOTION_CONFIRM_TIME) human_confirmed = true;
    }
  } else {
    motionStartTime = 0;
    if (human_confirmed) {
      if (lostStartTime == 0) lostStartTime = now;
      if (now - lostStartTime >= LOST_CONFIRM_TIME) { human_confirmed = false; fallCounter = 0; status = NO_PEOPLE; }
    }
  }

  // FALL DETECTION 
  if (human_confirmed) {
    if (is_fall) {
      if (fallCounter < FALL_CONFIRM_THRESHOLD) fallCounter++;
      if (fallCounter >= FALL_CONFIRM_THRESHOLD) status = PEOPLE_FALL;
    } else { fallCounter = 0; status = PEOPLE_IN_AREA; }
  }

  // FIREBASE SEND 
  if (status == PEOPLE_FALL && lastStatus != PEOPLE_FALL) {
    if (now - lastFallSendTime > FIREBASE_COOLDOWN) { sendFallToFirebase(); lastFallSendTime = now; }
  }

  // BLE UPDATE Status
  if ((status != lastStatus || forceUpdate) && deviceConnected) {
    String msg;
    if (status == NO_PEOPLE)      msg = String(ZONE_ID) + " : NO PEOPLE 🟢";
    if (status == PEOPLE_IN_AREA) msg = String(ZONE_ID) + " : PEOPLE IN AREA 🔵";
    if (status == PEOPLE_FALL)    msg = String(ZONE_ID) + " : FALL DETECTED 🔴";

    pCharacteristic->setValue(msg.c_str());
    pCharacteristic->notify();
    
    if (forceUpdate) {
      delay(200); 
      reportCurrentConfigToBLE();
    }
    forceUpdate = false;
  }

  // Serial Monitor
  if ((now - lastPrintTime >= PRINT_INTERVAL) || (status != lastStatus)) {
    lastPrintTime = now;
    Serial.println("\n--------------------------------------------------");
    Serial.printf("STATUS: %s\n",
      (status == NO_PEOPLE) ? "NO PEOPLE 🟢" :
      (status == PEOPLE_FALL) ? "FALL! 🔴" : "PEOPLE 🔵"
    );
    Serial.printf("ZONE: %s\n ", ZONE_ID);
    Serial.printf("H: %.2f | S: %d | T: %.2f\n", height, (int)sensitivity, threshold);
    Serial.println("--------------------------------------------------");
  }

  lastStatus = status;
}