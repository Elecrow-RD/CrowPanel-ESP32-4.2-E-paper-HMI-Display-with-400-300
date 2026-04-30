#include <Arduino.h>            // Include Arduino core functions (setup, loop, GPIO, etc.)
#include "EPD.h"                // Include E-paper display driver library
#include "EPD_GUI.h"            // Include graphics drawing functions for EPD
#include "Ap_29demo.h"          // Include demo resources (such as background images)
#include <WiFi.h>              // Include WiFi functionality for ESP32
#include <WebServer.h>         // Include WebServer library to create HTTP server
#include "FS.h"                // Include file system base library
#include "SPIFFS.h"            // Include SPIFFS file system for flash storage

uint8_t Image_BW[15000];       // Buffer to store the full E-paper image (black & white)

#define txt_size 3808          // Define expected size of text image binary
#define pre_size 4576          // Define expected size of price image binary

WebServer server(80);          // Create a web server instance on port 80

const char *AP_SSID = "E_Paper_42_Config"; // Define WiFi hotspot name (SSID)

String HTML_UPLOAD = "<form method=\"post\" action=\"/ok\" enctype=\"multipart/form-data\">\
<input type=\"file\" name=\"msg\">\
<input type=\"submit\" value=\"Upload\">\
</form>";                     // HTML page for uploading a file

String HTML_OK = "<!DOCTYPE html><html><body><h1>OK</h1></body></html>"; // HTML response after upload success

File fsUploadFile;            // File object used to write uploaded file into SPIFFS

unsigned char price_formerly[pre_size]; // Buffer to store uploaded price image
unsigned char txt_formerly[txt_size];   // Buffer to store uploaded text image

String filename;              // Store current uploading file name

int flag_txt = 0;             // Flag indicating whether txt image is available
int flag_pre = 0;             // Flag indicating whether price image is available


// ======================== Refresh E-paper display ========================
void refresh_epd()
{
  EPD_RESET();                // Reset the E-paper display hardware
  delay(50);                  // Short delay for stability
  EPD_Init();                 // Initialize the display controller
  delay(50);                  // Wait for initialization

  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE); // Create a new canvas
  EPD_Full(WHITE);           // Fill the canvas with white color

  // Draw fixed top background image
  EPD_ShowPicture(0, 0, EPD_W, 40, background_top, WHITE);

  if (flag_txt)              // If text image exists
    EPD_ShowPicture(20, 60, 272, 112, txt_formerly, WHITE); // Draw text image

  if (flag_pre)              // If price image exists
    EPD_ShowPicture(20, 190, 352, 104, price_formerly, WHITE); // Draw price image

  EPD_Display_Fast(Image_BW); // Refresh the display with buffer content

  delay(100);                // Short delay after display update
  EPD_Sleep();               // Put display into low-power sleep mode
}

// ======================== Root page handler ========================
void handle_root()
{
  server.send(200, "text/html", HTML_UPLOAD); // Send upload page to browser
}

// ======================== File upload handler ========================
void handle_upload()
{
  HTTPUpload &upload = server.upload(); // Get upload object from server

  if (upload.status == UPLOAD_FILE_START) // When upload starts
  {
    Serial.println("Upload Start"); // Print debug info

    if (upload.totalSize == txt_size) // Check if it's text image
      filename = "/txt.bin";         // Assign filename for text
    else
      filename = "/pre.bin";         // Otherwise assign price filename

    fsUploadFile = SPIFFS.open(filename, FILE_WRITE); // Open file for writing
  }

  else if (upload.status == UPLOAD_FILE_WRITE) // When receiving a chunk
  {
    fsUploadFile.write(upload.buf, upload.currentSize); // Write chunk to file
  }

  else if (upload.status == UPLOAD_FILE_END) // When upload is finished
  {
    fsUploadFile.close(); // Close file after writing

    Serial.println("Upload End"); // Print debug info
    Serial.printf("File: %s\n", filename.c_str()); // Print file name
    Serial.printf("Size: %d\n", upload.totalSize); // Print file size

    File file = SPIFFS.open(filename, FILE_READ); // Re-open file for reading

    if (upload.totalSize == txt_size) // If text image
    {
      file.read(txt_formerly, txt_size); // Load into buffer
      flag_txt = 1;                      // Set text flag
      Serial.println("txt OK");          // Debug info
    }
    else
    {
      file.read(price_formerly, pre_size); // Load into price buffer
      flag_pre = 1;                        // Set price flag
      Serial.println("price OK");          // Debug info
    }

    file.close(); // Close file after reading

    refresh_epd(); // Update display with new content

    server.send(200, "text/html", HTML_OK); // Send success page to browser
  }
}


// ======================== Setup ========================
void setup()
{
  Serial.begin(115200); // Initialize serial communication

  pinMode(41, OUTPUT);  // Set GPIO41 as output (power control)
  digitalWrite(41, HIGH); // Turn on power

  pinMode(7, OUTPUT);   // Set GPIO7 as output
  digitalWrite(7, HIGH); // Enable display power

  EPD_GPIOInit();       // Initialize display GPIO pins
  EPD_Clear();          // Clear screen
  delay(500);           // Wait for stability

  EPD_RESET();          // Reset display
  delay(100);
  EPD_Init();           // Initialize display controller

  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE); // Create canvas
  EPD_Full(WHITE);     // Fill with white

  EPD_ShowPicture(0, 0, EPD_W, 40, background_top, WHITE); // Show top image
  EPD_Display_Fast(Image_BW); // Display initial screen

  delay(2000);          // Wait 2 seconds
  EPD_Sleep();          // Enter low-power mode

  // Initialize SPIFFS file system
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS Failed, formatting...");
    SPIFFS.format();   // Format SPIFFS if failed
    ESP.restart();     // Restart device
  }

  // Set WiFi to AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, ""); // Start hotspot without password

  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP()); // Print AP IP address

  // Configure HTTP routes
  server.on("/", handle_root); // Root path
  server.on("/ok", HTTP_POST, []() {}, handle_upload); // Upload handler

  server.begin(); // Start server
  Serial.println("HTTP server started");
}

// ======================== Main loop ========================
void loop()
{
  server.handleClient(); // Handle incoming HTTP requests
}

