#include <Arduino.h>
#include "EPD.h"
#include "EPD_GUI.h"

#include "SD.h"

#define SD_MOSI 40
#define SD_MISO 13
#define SD_SCK 39
#define SD_CS 10

SPIClass SD_SPI = SPIClass(HSPI);
uint8_t Image_BW[15000];

// ================= Clear screen function (optimized version) =================
void clear_all() {
  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE); // Create a new image buffer
  EPD_Full(WHITE);                                  // Fill the entire buffer with white
}

// ================= Display update function =================
void update_display(const char* text) {
  // ===== Correct initialization process (critical) =====
  EPD_RESET();       // Reset the e-paper display
  delay(100);        // Short delay for stability

  EPD_Init();        // Initialize the display
  delay(300);        // Wait for initialization to complete

  clear_all();       // Clear the image buffer

  // ===== Draw all content into buffer first =====
  EPD_ShowString(0, 0, text, 16, BLACK);

  // ===== Unified refresh (do NOT use partial refresh) =====
  EPD_Display_Fast(Image_BW);   // Perform full screen refresh

  delay(500);                  // Wait for display to stabilize
  EPD_Sleep();                 // Put display into sleep mode to save power
}

void setup() {
  Serial.begin(115200);

  // Screen power.
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);

  // Initialize e-ink screen GPIO.
  EPD_GPIOInit();
  EPD_Clear();          // Clear screen
  delay(500);           // Wait for stability

  // Turn on SD card power.
  pinMode(42, OUTPUT);
  digitalWrite(42, HIGH);
  delay(10);

  // SD card.
  SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
  if (!SD.begin(SD_CS, SD_SPI, 80000000)) {
    Serial.println(F("ERROR: File system mount failed!"));
    update_display("SD: ERROR");
  } else {
    Serial.printf("SD Size: %lluMB \n", SD.cardSize() / (1024 * 1024));
    char buffer[30];
    int length = sprintf(buffer, "SD Size:%lluMB", SD.cardSize() / (1024 * 1024));
    buffer[length] = '\0';
    Serial.println(buffer);
    update_display(buffer);
  }
}

void loop() {
 delay(10);
}
