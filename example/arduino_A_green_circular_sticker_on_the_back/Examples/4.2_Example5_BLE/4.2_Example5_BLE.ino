#include <Arduino.h>
#include "EPD.h"                  // e-paper display library
#include "EPD_GUI.h"              // e-paper display graphical interface library
#include "BLEDevice.h"            // BLE device library
#include "BLEServer.h"            // BLE server library
#include "BLEUtils.h"             // BLE utility functions library
#include "BLE2902.h"              // BLE descriptor library

// Define BLE service and characteristic UUIDs
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // RX characteristic UUID
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // TX characteristic UUID

// Declare BLE object pointers
BLECharacteristic *pCharacteristic;
BLEServer *pServer;
BLEService *pService;

// Flag to track whether the device is connected
bool deviceConnected = false;

// BLE data buffer
char BLEbuf[32] = {0};

// BLE server callback class
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("------> BLE connect ."); // Prompt when the device connects
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("------> BLE disconnect ."); // Prompt when the device disconnects
      pServer->startAdvertising(); // Restart advertising to allow reconnection
      Serial.println("start advertising");
    }
};

// BLE characteristic callback class
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue(); // Read the value written to the characteristic

      if (rxValue.length() > 0) {
        Serial.print("------>Received Value: ");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]); // Print each received character
        }
        Serial.println();

        // Respond based on the received characters
        if (rxValue.find("A") != -1) {
          Serial.print("Rx A!");
        }
        else if (rxValue.find("B") != -1) {
          Serial.print("Rx B!");
        }
        Serial.println();
      }
    }
};

// e-paper display image buffer
uint8_t Image_BW[15000];

// ================= Clear screen function (optimized version) =================
void clear_all() {
  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE); // Create a new image buffer
  EPD_Full(WHITE);                                  // Fill the entire buffer with white
}

// ================= Display update function =================
void update_display(const char* line1, const char* line2 = nullptr) {
  // ===== Correct initialization process (critical) =====
  EPD_RESET();       // Reset the e-paper display
  delay(100);        // Short delay for stability

  EPD_Init();        // Initialize the display
  delay(300);        // Wait for initialization to complete

  clear_all();       // Clear the image buffer

  // ===== Draw all content into buffer first =====
  EPD_ShowString(0, 0 + 0 * 20, line1, 16, BLACK); // Display first line
  if (line2 != nullptr) {
    EPD_ShowString(0, 0 + 1 * 20, line2, 16, BLACK); // Display second line
  }

  // ===== Unified refresh (do NOT use partial refresh) =====
  EPD_Display_Fast(Image_BW);   // Perform full screen refresh

  delay(500);                  // Wait for display to stabilize
  EPD_Sleep();                 // Put display into sleep mode to save power
}

void setup() {
  Serial.begin(115200); // Initialize serial communication

  pinMode(7, OUTPUT); // Set pin 7 to output for power control
  digitalWrite(7, HIGH); // Enable e-paper display power

  // Initialize BLE device and set device name
  BLEDevice::init("CrowPanel4-2");
  pServer = BLEDevice::createServer(); // Create BLE server
  pServer->setCallbacks(new MyServerCallbacks()); // Set server callbacks

  pService = pServer->createService(SERVICE_UUID); // Create BLE service
  
  // Create TX characteristic
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902()); // Add notification descriptor
  
  // Create RX characteristic
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  pRxCharacteristic->setCallbacks(new MyCallbacks()); // Set RX characteristic callbacks

  pService->start(); // Start BLE service
  pServer->getAdvertising()->start(); // Start advertising to allow device connections

  // Initialize e-paper display GPIO
  EPD_GPIOInit();
}

int flag = 0;

void loop() {
  if (deviceConnected) {
    memset(BLEbuf, 0, 32); // Clear the buffer
    memcpy(BLEbuf, (char*)"Hello BLE APP!", 32); // Set the buffer value
    pCharacteristic->setValue(BLEbuf); // Update characteristic value
    pCharacteristic->notify(); // Notify connected devices

    Serial.print("*** Sent Value: ");
    Serial.print(BLEbuf); // Print the sent value
    Serial.println(" ***");

    if (flag != 2)
      flag = 1; // Update flag to indicate device connection status
  } else {
    if (flag != 4)
      flag = 3; // Update flag to indicate device disconnection status
  }

  // Update e-paper display based on connection status
  if (flag == 1) {
    char buffer[30];
    strcpy(buffer, "Bluetooth connected");
    
    char bleMsg[32];
    strcpy(bleMsg, "Sent Value:");
    strcat(bleMsg, "Hello BLE APP!");
    
    update_display(buffer, bleMsg); // Display connection status and sent value
    flag = 2; // Update flag
  } else if (flag == 3) {
    update_display("Bluetooth not connected!"); // Display disconnection status
    flag = 4; // Update flag
  }

  delay(1000); // Wait for 1 second
}
