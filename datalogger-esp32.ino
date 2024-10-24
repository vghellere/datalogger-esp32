#include <OneWire.h>
#include <DallasTemperature.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "consts.h"
#include "utils.h"

// 16 bytes struct
struct Sample {
  time_t event_datetime; // 8 bytes
  float temp_value; // 4 bytes
};

BLEServer *pServer = NULL;
BLECharacteristic *pTempCharacteristic = NULL;
BLECharacteristic *pDatetimeCharacteristic = NULL;
BLE2902 *descriptor_2902 = NULL;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS_PIN);

// Pass our oneWire reference to Dallas Temperature sensor lib
DallasTemperature sensors(&oneWire);

bool initial_time_sync_done = false;
Sample samples[MAX_SAMPLES];
int sample_index = 0;
unsigned long last_millis;


class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    println("connected");
  };

  void onDisconnect(BLEServer *pServer) {
    println("disconnected");
    pServer->startAdvertising();  // restart advertising
    println("start advertising");
  }
};

class TempCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic, esp_ble_gatts_cb_param_t *param) {
    String new_value = "";
    
    if (sample_index > 0) {
      new_value += String(samples[sample_index-1].event_datetime) + "," + String(samples[sample_index-1].temp_value);
      sample_index--;
    }

    pCharacteristic->setValue(new_value);
  }
};

class DatetimeCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic, esp_ble_gatts_cb_param_t *param) {
    // when the DatetimeCharacteristic is written to we update the internal RTC
    set_time_epoch(pCharacteristic->getValue().toInt()); // String.toInt actually returns a long ¯\_(ツ)_/¯
    initial_time_sync_done = true;
  }
};

void setup_ble() {
  // Create the BLE Device
  BLEDevice::init("ESP32 Datalogger");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  // Create the BLE Services
  BLEService *pTempService = pServer->createService(TEMP_SERVICE_UUID);
  BLEService *pDatetimeService = pServer->createService(DATETIME_SERVICE_UUID);

  // Create a BLE Temperature Characteristic
  pTempCharacteristic = pTempService->createCharacteristic(
    TEMP_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ
  );
  pTempCharacteristic->setCallbacks(new TempCharacteristicCallbacks());

  // Create a BLE Datetime Characteristic
  pDatetimeCharacteristic = pDatetimeService->createCharacteristic(
    DATETIME_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pDatetimeCharacteristic->setCallbacks(new DatetimeCharacteristicCallbacks());

  // Start the services
  pTempService->start();
  pDatetimeService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(TEMP_SERVICE_UUID);
  pAdvertising->addServiceUUID(DATETIME_CHARACTERISTIC_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x0);
};

void setup() {
  #if DEBUG
  Serial.begin(115200);
  #endif

  last_millis = millis();

  // Start the DS18B20 sensor
  sensors.begin();

  setup_ble();
  BLEDevice::startAdvertising();

  println("Waiting a client connection to notify...");
}

bool is_sample_buffer_full() {
  return sample_index >= MAX_SAMPLES;
}

void sample_temperature() {
  println("Time to sample...");

  // only get sample if time is in sync
  if (!initial_time_sync_done) {
    println("Sampling aborted: time out of sync");
    return;
  };

  // only get sample if buffer is not full
  if (is_sample_buffer_full()) {
    println("Sampling aborted: the buffer is full");
    return;
  }

  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);

  samples[sample_index].event_datetime = get_time_epoch();
  samples[sample_index].temp_value = temperatureC;
  sample_index++;

  println("Sampling done!!!");
}

void loop() {
  unsigned long current_millis = millis();
  bool time_to_sample = (current_millis - last_millis) > SAMPLE_TIME;
  // checks if millis() has overflown, it happens every 50 days
  bool has_millis_overflown = current_millis < last_millis;

  if (has_millis_overflown) last_millis = current_millis;

  if (time_to_sample) {
    sample_temperature();
    last_millis = current_millis;
  }
}