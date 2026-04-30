#include <Arduino.h>
#include "EPD.h"
#include "EPD_GUI.h"
#include <WiFi.h>

String ssid     = "elecrow888";       // your wifi name
String password = "elecrow2014";      // your wifi password

uint8_t Image_BW[15000];

// ================= Clear screen function (optimized version) =================
void clear_all() {
  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE); // Create a new image buffer
  EPD_Full(WHITE);                                  // Fill the entire buffer with white
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // ===== Power control (same as previous code) =====
  pinMode(41, OUTPUT);    // Set GPIO41 as output (power control)
  digitalWrite(41, HIGH); // Enable power

  pinMode(7, OUTPUT);     // Set GPIO7 as output
  digitalWrite(7, HIGH);  // Enable display power

  //e-paper screen initialization
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

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  char buffer[40];
  strcpy(buffer, "WiFi connected");
  EPD_ShowString(0, 0 + 0 * 20, buffer, 16, BLACK);

  strcpy(buffer, "IP address: ");
  strcat(buffer, WiFi.localIP().toString().c_str());
  EPD_ShowString(0, 0 + 1 * 20, buffer, 16, BLACK);

  // ===== Unified refresh (do NOT use partial refresh) =====
  EPD_Display_Fast(Image_BW);   // Perform full screen refresh

  delay(500);                  // Wait for display to stabilize
  EPD_Sleep();                 // Put display into sleep mode to save power
}

void loop() {
  // put your main code here, to run repeatedly:

}

