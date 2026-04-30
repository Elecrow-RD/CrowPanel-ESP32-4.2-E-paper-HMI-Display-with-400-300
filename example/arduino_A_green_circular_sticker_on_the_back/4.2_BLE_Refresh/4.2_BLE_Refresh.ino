#include <BLEDevice.h>          // BLE core library (device functions)
#include <BLEServer.h>          // BLE server functionality
#include <BLEUtils.h>           // BLE utility helpers
#include <BLE2902.h>            // BLE descriptor (used for notifications)
#include <Arduino.h>            // Arduino core functions
#include "EPD.h"                // E-paper display driver
#include "EPD_GUI.h"            // E-paper drawing GUI library
#include "Ap_29demo.h"          // Demo assets (background images etc.)
#include "FS.h"                 // File system base library
#include "SPIFFS.h"             // SPIFFS flash file system

// Image buffer
uint8_t Image_BW[15000];        // Frame buffer for EPD display (black & white image)

// Image size definitions
#define txt_size 3808           // Size of text image data
#define pre_size 4576           // Size of price image data

File fsUploadFile;              // File object for SPIFFS writing

#define SERVICE_UUID          "fb1e4001-54ae-4a28-9f74-dfccb248601d"   // BLE service UUID
#define CHARACTERISTIC_UUID   "fb1e4002-54ae-4a28-9f74-dfccb248601d"   // BLE characteristic UUID

BLECharacteristic *pCharacteristicRX;     // Pointer to BLE characteristic (RX)

std::vector<uint8_t> dataBuffer;          // Buffer to store received BLE data
size_t totalReceivedBytes = 0;            // Total number of received bytes
bool dataReceived = false;                // Flag indicating data reception complete
    
String filename;                          // File name for SPIFFS storage
    
int flag_txt = 0;                         // Flag indicating text image exists
int flag_pre = 0;                         // Flag indicating price image exists

unsigned char price_formerly[pre_size];   // Buffer for price image data
unsigned char txt_formerly  [txt_size];   // Buffer for text image data

// ======================== Clear Screen ========================
void clear_all()
{
  EPD_Clear();                                          // Clear EPD display memory
  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE);     // Create new blank canvas
  EPD_Full(WHITE);                                      // Fill display with white
  EPD_Display_Part(0, 0, EPD_W, EPD_H, Image_BW);       // Refresh full screen
}

// ======================== BLE Server Callbacks ========================
class MyServerCallbacks : public BLEServerCallbacks {

    void onConnect(BLEServer* pServer) {
        Serial.println("[BLE] ===== Device Connected =====");     // Print connection event
        Serial.println("[BLE] Phone is now connected ✔");        // Confirm connection
    }

    void onDisconnect(BLEServer* pServer) {
        Serial.println("[BLE] ===== Device Disconnected =====");  // Print disconnect event
        Serial.println("[BLE] Restart advertising...");           // Restart BLE advertising

        BLEDevice::startAdvertising();                            // Restart advertising after disconnect
    }
};

// ======================== BLE Characteristic Callback ========================
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();                          // Get received BLE data

    if (value.length() > 0) {

      Serial.printf("[BLE] Chunk received: %d bytes\n", value.length());      // Print chunk size

      dataBuffer.insert(dataBuffer.end(), value.begin(), value.end());        // Append data to buffer
      totalReceivedBytes += value.length(); // Update total received size

      Serial.printf("[BLE] Total received: %d bytes\n", totalReceivedBytes);   // Print progress

      if (totalReceivedBytes == txt_size || totalReceivedBytes == pre_size) {
        Serial.println("[BLE] ===== Receive Complete =====");                  // Mark reception complete
        dataReceived = true;                                                   // Set flag to trigger processing
      }
    }
  }
};

// ======================== Screen Refresh Function ========================
void refresh_epd()
{
  EPD_RESET();          // Reset EPD display
  delay(50);            // Wait for reset
  EPD_Init();           // Initialize EPD driver
  delay(50);            // Stability delay

  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE); // Create canvas
  EPD_Full(WHITE);      // Clear screen

  // Draw top background image
  EPD_ShowPicture(0, 0, EPD_W, 40, background_top, WHITE);

  if (flag_txt)
  {
    Serial.println("[EPD] Drawing TXT image"); // Debug print
    EPD_ShowPicture(20, 60, 272, 112, txt_formerly, WHITE); // Draw text image
  }

  if (flag_pre)
  {
    Serial.println("[EPD] Drawing PRICE image"); // Debug print
    EPD_ShowPicture(20, 190, 352, 104, price_formerly, WHITE); // Draw price image
  }

  EPD_Display_Fast(Image_BW); // Update display quickly
  Serial.println("[EPD] Display update..."); // Debug print

  delay(100);         // Short delay
  EPD_Sleep();        // Put display into low power mode
}

// ======================== BLE Image Processing ========================
void Receive_BLE_Images()
{
  if (dataReceived)
  {
    if (!dataBuffer.empty())
    {
      size_t bufferSize = dataBuffer.size(); // Get buffer size
      Serial.printf("Received size: %d\n", bufferSize); // Print size

      if (bufferSize == txt_size)
        filename = "/txt.bin"; // Text image file
      else
        filename = "/pre.bin";  // Price image file

      // Write to SPIFFS
      Serial.println("[SPIFFS] Writing file...");
      fsUploadFile = SPIFFS.open(filename, FILE_WRITE);
      fsUploadFile.write(dataBuffer.data(), bufferSize);
      fsUploadFile.close();

      Serial.println("[SPIFFS] Write complete"); // Confirm write
      Serial.println("Saved successfully");
      Serial.printf("File: %s\n", filename.c_str()); // Print file name

      File file = SPIFFS.open(filename, FILE_READ); // Reopen file

      if (bufferSize == txt_size)
      {
        file.read(txt_formerly, txt_size); // Load text image
        flag_txt = 1; // Set flag
      }
      else
      {
        file.read(price_formerly, pre_size); // Load price image
        flag_pre = 1; // Set flag
      }

      file.close(); // Close file

      refresh_epd(); // Refresh display

      dataBuffer.clear(); // Clear buffer
      totalReceivedBytes = 0; // Reset counter
      dataReceived = false; // Reset flag
    }
  }
}

// ======================== Setup Function ========================
void setup() {
  Serial.begin(115200); // Initialize serial communication

  pinMode(41, OUTPUT);  // Set GPIO41 as output (power control)
  digitalWrite(41, HIGH); // Turn on power

  pinMode(7, OUTPUT);   // Set GPIO7 as output
  digitalWrite(7, HIGH); // Enable display power

  EPD_GPIOInit();       // Initialize EPD GPIO
  EPD_Clear();          // Clear display
  delay(500);           // Stability delay

  EPD_RESET();          // Reset EPD
  delay(100);
  EPD_Init();           // Initialize display

  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE); // Create canvas
  EPD_Full(WHITE);     // Clear screen

  EPD_ShowPicture(0, 0, EPD_W, 40, background_top, WHITE); // Show top image
  EPD_Display_Fast(Image_BW); // Refresh screen

  delay(2000);          // Wait for stability
  EPD_Sleep();          // Enter low power mode

  Serial.println("Ink Screen Initialization successful"); // Debug info

  if (!SPIFFS.begin()) {
      Serial.println("SPIFFS Failed, formatting..."); // File system error
      SPIFFS.format();   // Format SPIFFS
      ESP.restart();     // Restart system
  }

  BLEDevice::init("42_E-Paper_BLE"); // Initialize BLE device name
  BLEServer *pServer = BLEDevice::createServer(); // Create BLE server

  pServer->setCallbacks(new MyServerCallbacks()); // Register connection callbacks

  BLEService *pService = pServer->createService(SERVICE_UUID); // Create BLE service

  pCharacteristicRX = pService->createCharacteristic(
                          CHARACTERISTIC_UUID,
                          BLECharacteristic::PROPERTY_WRITE
                      ); // Create writable characteristic

  pCharacteristicRX->setCallbacks (new MyCallbacks());  // Register data callback
  pCharacteristicRX->addDescriptor(new BLE2902());      // Add BLE descriptor

  pService->start(); // Start BLE service

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising(); // Get advertising object
  pAdvertising->addServiceUUID(SERVICE_UUID);                 // Add service UUID
  pAdvertising->start();                                      // Start advertising

  Serial.println("BLE Advertising started"); // Debug info
}

// ======================== Main Loop ========================
void loop() {
  Receive_BLE_Images(); // Continuously check BLE data
}
