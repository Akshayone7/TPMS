#include <bluefruit.h>

BLEService bleService = BLEService("adcf8131-6018-4aa9-b29c-467c69333b5d");                       // Custom BLE service
BLECharacteristic bleCharacteristic = BLECharacteristic("598876ed-e093-459c-b9b0-2cfe9a749e59");  // Custom BLE characteristic
unsigned long lastAdvertisementUpdate = 0;                        // Timer to track 1-second intervals
int value = 101;
const int pin = 34;

// Function to start advertising
void startAdvertising() {
  // Clear current advertising data
  // Bluefruit.Advertising.clearData();

  // Add device name to the advertisement
  Bluefruit.Advertising.addName();  // Advertise the device name

  // Add custom data "Hello" to the advertisement
  uint8_t helloMessage[] = { 'H', 'e', 'l', 'l', 'o' };
  Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, helloMessage, sizeof(helloMessage));

  // Configure advertising interval (in units of 0.625 ms, so 160 * 0.625ms = 100ms)
  Bluefruit.Advertising.setInterval(160, 160);

  // Set how long to advertise before stopping (0 = never stop)
  Bluefruit.Advertising.setFastTimeout(30);

  // Start advertising
  Bluefruit.Advertising.start(0);
}

// Callback when a central device connects
void connect_callback(uint16_t conn_handle) {
  Serial.println("Connected to a central device!");

  // Send a "Hello World" notification when connected
  // bleCharacteristic.notify(conn_handle, (uint8_t*)"Hello World", 11);  // Send 11 bytes ("Hello World")
}

// Callback when a central device disconnects
void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  Serial.println("Disconnected, restarting advertising...");
  startAdvertising();  // Start advertising again after disconnect
}

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);

  // Initialize Bluefruit module
  Bluefruit.begin();
  Bluefruit.setName("My_BLE_Device");

  // Set up the service and characteristic
  bleService.begin();
  bleCharacteristic.setProperties(CHR_PROPS_NOTIFY);                 // Set the characteristic as "Notify"
  bleCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);  // No read access, only notify
  bleCharacteristic.setFixedLen(11);                                 // Fixed length for "Hello World"
  bleCharacteristic.begin();

  // Set connection and disconnection callbacks
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // Start advertising
  startAdvertising();
}

void loop() {
  // Periodically read sensor value and check condition
  // value = analogRead(pin);
  Serial.println(value);

  if (returnState(value)) {
    if (Bluefruit.connected()) {
      bleCharacteristic.notify((uint8_t*)"Hello World", 11);  // Send 11 bytes ("Hello World")
    }
    // Serial.println("Value > 100, connection initiated.");
    // Since connection is handled by the ESP32 central, ensure we are advertising
    // startAdvertising();
  }

  delay(1000);  // Delay to avoid reading too frequently
}

// Function to check if sensor value exceeds threshold
bool returnState(int value) {
  if (value >= 100) {
    // Connection logic would be initiated by the central device (ESP32)
    // We ensure the device is advertising so the ESP32 can find and connect
    return true;
  }
  return false;
}