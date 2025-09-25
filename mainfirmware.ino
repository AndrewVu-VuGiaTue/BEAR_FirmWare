#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

PulseOximeter pox;

#define REPORTING_PERIOD_MS 1000

#define SERVICE_UUID        "180D"   // Heart Rate Service
#define CHARACTERISTIC_UUID "2A37"   // Heart Rate Measurement

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

uint32_t lastReport = 0;
float bpm = 0;
float spo2 = 0;

// BLE server callback
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void onBeatDetected() {
  Serial.println("Beat detected!");
}

void setup() {
  Serial.begin(115200);

  // MAX30100 init
  if (!pox.begin()) {
    Serial.println("FAILED to initialize MAX30100");
    for(;;);
  } else {
    Serial.println("MAX30100 ready");
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);

  // BLE init
  BLEDevice::init("ESP32_HEALTHBAND");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  pServer->getAdvertising()->start();
  Serial.println("BLE Heart Rate Service started");
}

void loop() {
  pox.update();

  if (millis() - lastReport > REPORTING_PERIOD_MS) {
    lastReport = millis();
    bpm = pox.getHeartRate();
    spo2 = pox.getSpO2();

    Serial.print("BPM: ");
    Serial.print(bpm);
    Serial.print(" | SpO2: ");
    Serial.println(spo2);

    // Gửi qua BLE mỗi 10 giây
    static unsigned long lastSend = 0;
    if (deviceConnected && millis() - lastSend > 10000) {
      lastSend = millis();

      // Dữ liệu Heart Rate Measurement (theo chuẩn BLE)
      uint8_t heartRate[2];
      heartRate[0] = 0x06;         // Flags
      heartRate[1] = (uint8_t)bpm; // BPM

      pCharacteristic->setValue(heartRate, 2);
      pCharacteristic->notify();

      Serial.println("Sent HR via BLE: " + String(bpm) + " | SpO2: " + String(spo2));
    }
  }
}
