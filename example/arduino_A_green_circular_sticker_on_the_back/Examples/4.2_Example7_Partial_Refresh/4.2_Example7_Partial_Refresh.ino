#include <Arduino.h>        // Include the Arduino core library for development on the Arduino platform
#include "EPD.h"            // Include the EPD library to control the electronic ink screen (E-Paper Display)
#include "EPD_GUI.h"        // Include the EPD_GUI library which provides graphical user interface functionalities
#include "pic_scenario.h"   // Include the header file containing image data

uint8_t Image_BW[15000];    // Declare an array of size 15000 bytes to store black and white image data

// ================= Clear screen function (optimized version) =================
void clear_all() {
  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE); // Create a new image buffer
  EPD_Full(WHITE);                                  // Fill the entire buffer with white
}

void setup() {
  // Initialization settings, executed only once when the program starts
  // Initialize screen power
  // ===== Power control (same as previous code) =====
  pinMode(41, OUTPUT);    // Set GPIO41 as output (power control)
  digitalWrite(41, HIGH); // Enable power

  pinMode(7, OUTPUT);     // Set GPIO7 as output
  digitalWrite(7, HIGH);  // Enable display power

  // ===== Initialize EPD GPIO =====
  EPD_GPIOInit();   // Initialize display GPIO interface
  
  EPD_Clear();          // Clear screen
  delay(500);           // Wait for stability

  // ===== Correct initialization process (critical) =====
  EPD_RESET();       // Reset the e-paper display
  delay(100);        // Short delay for stability

  EPD_Init();        // Initialize the display
  delay(300);        // Wait for initialization to complete

  clear_all();       // Clear the image buffer

  // Display the boot interface
  EPD_ShowPicture(0, 0, 312, 152, gImage_1, WHITE); // Display picture gImage_1 with starting coordinates (0, 0), width 312 and height 152 and background color white.
  // ===== Unified refresh (do NOT use partial refresh) =====
  EPD_Display_Fast(Image_BW);   // Perform full screen refresh

  delay(500);                  // Wait for display to stabilize
  EPD_Sleep();                 // Put display into sleep mode to save power
}

void loop() {
  // Main loop function, currently does not perform any actions
  // Code that needs to be executed repeatedly can be added in this function
}

