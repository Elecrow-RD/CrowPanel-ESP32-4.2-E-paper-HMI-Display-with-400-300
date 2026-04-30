#include <Arduino.h>                 // Include Arduino core library
#include "EPD.h"                    // Include e-paper display driver
#include "EPD_GUI.h"                // Include e-paper graphics library  

// Image buffer for the e-paper display
uint8_t Image_BW[15000];            // Buffer used to store the full screen image

// Define the numbers of 12 GPIO pins
int pin_Num[12] = {8, 3, 14, 9, 16, 15, 18, 17, 20, 19, 38, 21}; // GPIO pins to monitor

// ================= Clear screen function (optimized version) =================
void clear_all() {
  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE); // Create a new image buffer
  EPD_Full(WHITE);                                  // Fill the entire buffer with white
}

// ================= GPIO status display function =================
void judgement_function(int* pin) {

  char buffer[30];   // Buffer to store formatted string output

  // ===== Correct initialization process (critical) =====
  EPD_RESET();       // Reset the e-paper display
  delay(100);        // Short delay for stability

  EPD_Init();        // Initialize the display
  delay(300);        // Wait for initialization to complete

  clear_all();       // Clear the image buffer

  // ===== Draw all content into buffer first =====
  for (int i = 0; i < 12; i++) {

    int state = digitalRead(pin[i]);  // Read current GPIO state

    if (state == HIGH) {
      sprintf(buffer, "GPIO%d : ON", pin[i]);   // Format string if HIGH
    } else {
      sprintf(buffer, "GPIO%d : OFF", pin[i]);  // Format string if LOW
    }

    // Each line spaced by 20 pixels vertically
    EPD_ShowString(10, 10 + i * 20, buffer, 16, BLACK); // Draw text onto buffer
  }

  // ===== Unified refresh (do NOT use partial refresh) =====
  EPD_Display_Fast(Image_BW);   // Perform full screen refresh

  delay(500);                  // Wait for display to stabilize
  EPD_Sleep();                 // Put display into sleep mode to save power
}

// ================= setup =================
void setup() {

  Serial.begin(115200);   // Initialize serial communication

  // ===== Power control (same as previous code) =====
  pinMode(41, OUTPUT);    // Set GPIO41 as output (power control)
  digitalWrite(41, HIGH); // Enable power

  pinMode(7, OUTPUT);     // Set GPIO7 as output
  digitalWrite(7, HIGH);  // Enable display power

  // ===== GPIO initialization =====
  for (int i = 0; i < 12; i++) {
    pinMode(pin_Num[i], OUTPUT);     // Set each GPIO as output
    digitalWrite(pin_Num[i], HIGH);  // Set each GPIO to HIGH
  }

  // ===== Initialize EPD GPIO =====
  EPD_GPIOInit();   // Initialize display GPIO interface
  
  EPD_Clear();          // Clear screen
  delay(500);           // Wait for stability
  // ===== Display once =====
  judgement_function(pin_Num);  // Show GPIO status on screen
}

// ================= loop =================
void loop() {
  delay(1000);  // Main loop delay (1 second)
}
