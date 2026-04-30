#include <Arduino.h>
#include "EPD.h"      // Include the EPD library for controlling the e-ink display
#include "EPD_GUI.h"  // Include the EPD_GUI library which provides drawing functions

uint8_t Image_BW[15000]; // Declare an array to store the black and white image

// Define button pins
#define HOME_KEY 2   // Home key pin
int HOME_NUM = 0; // Home key press count

#define EXIT_KEY 1   // Exit key pin
int EXIT_NUM = 0; // Exit key press count

#define PRV_KEY 6    // Previous page key pin
int PRV_NUM = 0;  // Previous page key press count

#define NEXT_KEY 4   // Next page key pin
int NEXT_NUM = 0; // Next page key press count

#define OK_KEY 5     // Confirm key pin
int OK_NUM = 0;   // Confirm key press count

int NUM_btn[5] = {0}; // Array to store the counts of five buttons

// ================= Clear screen function (optimized version) =================
void clear_all() {
  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE); // Create a new image buffer
  EPD_Full(WHITE);                                  // Fill the entire buffer with white
}

// Function: Display button press counts
void count_btn(int NUM[5]) {
  char buffer[30]; // Buffer to store the display string

  // ===== Correct initialization process (critical) =====
  EPD_RESET();       // Reset the e-paper display
  delay(100);        // Short delay for stability

  EPD_Init();        // Initialize the display
  delay(300);        // Wait for initialization to complete

  clear_all();       // Clear the image buffer

  // ===== Draw all content into buffer first =====
  // Display home key press count
  int length = sprintf(buffer, "HOME_KEY_NUM:%d", NUM[0]);
  buffer[length] = '\0'; // Ensure the string has a null terminator at the end
  EPD_ShowString(0, 0 + 0 * 20, buffer, 16, BLACK);
  memset(buffer, 0, sizeof(buffer)); // Clear the buffer

  // Display exit key press count
  length = sprintf(buffer, "EXIT_KEY_NUM:%d", NUM[1]);
  buffer[length] = '\0';
  EPD_ShowString(0, 0 + 1 * 20, buffer, 16, BLACK);
  memset(buffer, 0, sizeof(buffer));

  // Display previous page key press count
  length = sprintf(buffer, "PRV_KEY_NUM:%d", NUM[2]);
  buffer[length] = '\0';
  EPD_ShowString(0, 0 + 2 * 20, buffer, 16, BLACK);
  memset(buffer, 0, sizeof(buffer));

  // Display next page key press count
  length = sprintf(buffer, "NEXT__NUM:%d", NUM[3]);
  buffer[length] = '\0';
  EPD_ShowString(0, 0 + 3 * 20, buffer, 16, BLACK);
  memset(buffer, 0, sizeof(buffer));

  // Display confirm key press count
  length = sprintf(buffer, "OK_NUM:%d", NUM[4]);
  buffer[length] = '\0';
  EPD_ShowString(0, 0 + 4 * 20, buffer, 16, BLACK);

  // ===== Unified refresh (do NOT use partial refresh) =====
  EPD_Display_Fast(Image_BW);   // Perform full screen refresh (changed from EPD_Display_Part)

  delay(500);                  // Wait for display to stabilize
  EPD_Sleep();                 // Put display into sleep mode to save power
}

// Initialization settings
void setup() {
  Serial.begin(115200); // Initialize serial communication with a baud rate of 115200

  // ===== Power control (same as previous code) =====
  pinMode(41, OUTPUT);    // Set GPIO41 as output (power control)
  digitalWrite(41, HIGH); // Enable power

  pinMode(7, OUTPUT); 
  digitalWrite(7, HIGH); // Turn on the screen power

  // Set button pins as input mode
  pinMode(HOME_KEY, INPUT);
  pinMode(EXIT_KEY, INPUT);
  pinMode(PRV_KEY, INPUT);
  pinMode(NEXT_KEY, INPUT);
  pinMode(OK_KEY, INPUT);

  // Initialize the e-ink display
  EPD_GPIOInit();
  EPD_Clear();          // Clear screen
  delay(500);           // Wait for stability
}

// Main loop function
void loop() {
  int flag = 0; // Flag to indicate if any button is pressed

  // Check if the home key is pressed
  if (digitalRead(HOME_KEY) == 0) {
    delay(100); // Debounce
    if (digitalRead(HOME_KEY) == 1) { // Execute after the key is released
      Serial.println("HOME_KEY"); // Print key information
      HOME_NUM++; // Increment home key count
      flag = 1; // Set flag
    }
  }
  // Check if the exit key is pressed
  else if (digitalRead(EXIT_KEY) == 0) {
    delay(100);
    if (digitalRead(EXIT_KEY) == 1) {
      Serial.println("EXIT_KEY");
      EXIT_NUM++;
      flag = 1;
    }
  }
  // Check if the previous page key is pressed
  else if (digitalRead(PRV_KEY) == 0) {
    delay(100);
    if (digitalRead(PRV_KEY) == 1) {
      Serial.println("PRV_KEY");
      PRV_NUM++;
      flag = 1;
    }
  }
  // Check if the next page key is pressed
  else if (digitalRead(NEXT_KEY) == 0) {
    delay(100);
    if (digitalRead(NEXT_KEY) == 1) {
      Serial.println("NEXT_KEY");
      NEXT_NUM++;
      flag = 1;
    }
  }
  // Check if the confirm key is pressed
  else if (digitalRead(OK_KEY) == 0) {
    delay(100);
    if (digitalRead(OK_KEY) == 1) {
      Serial.println("OK_KEY");
      OK_NUM++;
      flag = 1;
    }
  }
  
  // If a button is pressed, update the button count display
  if (flag == 1) {
    NUM_btn[0] = HOME_NUM;
    NUM_btn[1] = EXIT_NUM;
    NUM_btn[2] = PRV_NUM;
    NUM_btn[3] = NEXT_NUM;
    NUM_btn[4] = OK_NUM;

    count_btn(NUM_btn); // Update the display
    flag = 0; // Reset the flag
  }
}
