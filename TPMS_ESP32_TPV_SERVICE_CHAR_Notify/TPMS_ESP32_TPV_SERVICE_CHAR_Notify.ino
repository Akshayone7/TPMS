#include "BLEDevice.h"
#include "BLEScan.h"

static BLEUUID serviceUUID2("470230b8-28cc-4c91-9a73-6dc094564b96");
static BLEUUID serviceUUID1("669a0c20-0008-a7ba-e311-0685c0f7978a");
static BLEUUID charUUIDTPV("470230ba-28cc-4c91-9a73-6dc094564b96");
static BLEUUID charUUIDAccel("470230bb-28cc-4c91-9a73-6dc094564b96");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pTPVCharacteristic;
static BLERemoteCharacteristic* pAccelCharacteristic;
static BLEAdvertisedDevice* myDevice;
String targetMac = "c8:eb:9a:e0:db:08";

const int LedPin = 2;

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    connected = true;
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

void readCharacteristicValue() {
  if (pTPVCharacteristic->canRead()) {
    String value = pTPVCharacteristic->readValue();
    Serial.print("Characteristic value: ");
    Serial.print("Pressure, Voltage, Temp: ");
    // Serial.print("The characteristic value was: ");
    for (int i = 0; i < value.length(); i++) {
      Serial.print(value[i], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
  } else {
    Serial.println("Failed to read characteristic value!");
  }

  if (pAccelCharacteristic->canRead()) {
    String value = pAccelCharacteristic->readValue();
    Serial.print("Characteristic value: ");
    Serial.print("Accel: ");
    // Serial.print("The characteristic value was: ");
    for (int i = 0; i < value.length(); i++) {

      Serial.print(value[i], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
  } else {
    Serial.println("Failed to read characteristic value!");
  }
}

bool connectToServer() {
  Serial.print("Connecting to BLE Server: ");
  Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient* pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

  if (!pClient->connect(myDevice)) {
    Serial.println("Failed to connect to server. Reconnecting...");
    delay(1000);
    return false;
  }
  Serial.println("Connected to server.");

  BLERemoteService* pRemoteService = pClient->getService(serviceUUID2);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find service UUID: ");
    Serial.println(serviceUUID2.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println("Found service.");

  pTPVCharacteristic = pRemoteService->getCharacteristic(charUUIDTPV);
  if (pTPVCharacteristic == nullptr) {
    Serial.print("Failed to find characteristic UUID: ");
    Serial.println(charUUIDTPV.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println("Found TPV characteristic.");

  delay(1000);

  pAccelCharacteristic = pRemoteService->getCharacteristic(charUUIDAccel);
  if (pAccelCharacteristic == nullptr) {
    Serial.print("Failed to find characteristic UUID: ");
    Serial.println(charUUIDAccel.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println("Found Accel characteristic.");


  connected = true;
  return true;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID1)) {
      // if (advertisedDevice.getName()== "TS00003BB5") {
      if (advertisedDevice.getAddress().toString() == targetMac) {


        Serial.print("Found BLE Advertised Device: ");
        Serial.println(advertisedDevice.toString().c_str());
        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        doScan = true;
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Client application...");
  BLEDevice::init("");

  pinMode(LedPin, OUTPUT);

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(0, false);
}

void loop() {
  if (doConnect) {
    if (connectToServer()) {
      Serial.println("Connected to BLE Server.");
    } else {
      Serial.println("Failed to connect to BLE Server.");
    }
    doConnect = false;
  }

  if (connected) {
    readCharacteristicValue();
    digitalWrite(LedPin, HIGH);
    delay(1000);
    digitalWrite(LedPin, LOW);
    delay(1000);
  }

  delay(1000);  // Adjust delay time as needed
}