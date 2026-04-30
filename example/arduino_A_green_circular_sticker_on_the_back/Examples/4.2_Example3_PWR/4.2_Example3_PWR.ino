#include <Arduino.h>
#include "EPD.h"
#include "EPD_GUI.h"

uint8_t Image_BW[15000];

// Home key.
#define HOME_KEY 2
int HOME_NUM = 0;

// ================= Clear screen function (optimized version) =================
void clear_all() {
  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE); // Create a new image buffer
  EPD_Full(WHITE);                                  // Fill the entire buffer with white
}

// ================= Display update function =================
void update_display() {
  char buffer[30];

  // ===== Correct initialization process (critical) =====
  EPD_RESET();       // Reset the e-paper display
  delay(100);        // Short delay for stability

  EPD_Init();        // Initialize the display
  delay(300);        // Wait for initialization to complete

  clear_all();       // Clear the image buffer

  // ===== Draw all content into buffer first =====
  if (HOME_NUM == 1) {
    digitalWrite(41, HIGH);
    strcpy(buffer, "PWR:ON");
  } else {
    digitalWrite(41, LOW);
    strcpy(buffer, "PWR:OFF");
  }
  EPD_ShowString(0, 0 + 0 * 20, buffer, 16, BLACK);

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

  // POWER indicator light.
  pinMode(41, OUTPUT);

  pinMode(HOME_KEY, INPUT);

  // Initialize e-ink screen.
  EPD_GPIOInit();
  
  EPD_Clear();          // Clear screen
  delay(500);           // Wait for stability
}

void loop() {
  int flag = 0;

  if (digitalRead(HOME_KEY) == 0) {
    delay(100);
    if (digitalRead(HOME_KEY) == 1) {
      Serial.println("HOME_KEY");
      HOME_NUM = !HOME_NUM;
      flag = 1;
    }
  }

  if (flag == 1) {
    update_display();
    flag = 0;
  }
}