#include <Arduino.h>            // Include the Arduino core library
#include <WiFi.h>               // Include the WiFi library for wireless networking
#include <HTTPClient.h>         // Include the HTTP client library for web requests
#include <Arduino_JSON.h>       // Include the JSON parsing library
#include "EPD.h"                // Include the E-paper display driver
#include "EPD_GUI.h"            // Include the E-paper graphics library
#include "pic.h"                // Include image resources for the display

// ================= EPD buffer =================
uint8_t Image_BW[15000];        // Create a frame buffer for the E-paper display image

// ================= WiFi =================
const char* ssid     = "elecrow888" ;      // WiFi network name
const char* password = "elecrow2014";      // WiFi network password

// ================= API =================
String openWeatherMapApiKey = "Your-API-Key"; // OpenWeatherMap API key
String city = "Shenzhen";                   // City name for weather query
String countryCode = "1795564";             // City ID for weather query
// String city = "London";                  // Alternative city name
// String countryCode = "2643743";          // Alternative city ID

// ================= timing =================
unsigned long lastTime = 0;                 // Store the last refresh time
// unsigned long timerDelay = 600000;       // Refresh every 10 minutes
unsigned long timerDelay = 300000;          // Refresh every 5 minutes

// ================= JSON =================
String jsonBuffer;                          // Store the JSON response text
JSONVar myObject;                           // Store the parsed JSON object

// ================= data =================
String city_js;                             // Store the returned city name
String weather;                             // Store the weather condition
String temperature;                         // Store the temperature
String humidity;                            // Store the humidity
String wind_speed;                          // Store the wind speed
String visibility;                          // Store the visibility distance

int weather_flag = 0;                       // Store the weather icon index
int httpResponseCode = 0;                   // Store the HTTP response code

// =====================================================
// HTTP request
// =====================================================
String httpGETRequest(const char* serverName) // Function to send an HTTP GET request
{
  WiFiClient client;                        // Create a WiFi client object
  HTTPClient http;                         // Create an HTTP client object

  http.begin(client, serverName);          // Initialize the HTTP connection
  httpResponseCode = http.GET();           // Send the GET request

  String payload = "{}";                   // Default empty JSON response

  if (httpResponseCode == 200)             // Check if request was successful
  {
    payload = http.getString();            // Read the response body
  }
  else
  {
    Serial.print("[HTTP ERROR] ");         // Print an HTTP error label
    Serial.println(httpResponseCode);      // Print the HTTP error code
  }

  http.end();                              // Close the HTTP connection
  return payload;                          // Return the response content
}

// =====================================================
// JSON parsing
// =====================================================
bool js_analysis()                         // Function to parse weather JSON data
{
  if (WiFi.status() != WL_CONNECTED)       // Check whether WiFi is connected
  {
    Serial.println("WiFi disconnected");   // Print disconnect message
    return false;                          // Return failure
  }

  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + // Build API URL
               city + "," + countryCode +
               "&APPID=" + openWeatherMapApiKey +
               "&units=metric";

  jsonBuffer = httpGETRequest(url.c_str()); // Send request and get JSON data

  Serial.println("========== RAW JSON =========="); // Print JSON header
  Serial.println(jsonBuffer);                     // Print raw JSON text

  myObject = JSON.parse(jsonBuffer);             // Parse the JSON text

  if (JSON.typeof(myObject) == "undefined")      // Check whether parsing failed
  {
    Serial.println("JSON parse failed");         // Print parse error
    return false;                                // Return failure
  }

  city_js     = (const char*)myObject["name"];               // Read city name
  weather     = (const char*)myObject["weather"][0]["main"]; // Read weather condition
  temperature = String((double)myObject["main"]["temp"]);    // Read temperature
  humidity    = String((int)myObject["main"]["humidity"]);   // Read humidity
  wind_speed  = String((double)myObject["wind"]["speed"]);   // Read wind speed
  // visibility  = String((int)myObject["visibility"]);      // Direct visibility read (disabled)

  if (myObject.hasOwnProperty("visibility"))     // Check if visibility exists
    visibility = String((int)myObject["visibility"]); // Read visibility value
  else
    visibility = "N/A";                          // Use fallback text if missing

  Serial.println("========== DATA ==========");  // Print parsed data header
  Serial.print("The City    : "); Serial.println(city_js);     // Print city
  Serial.print("Weather     : "); Serial.println(weather);     // Print weather
  Serial.print("Temperature : "); Serial.println(temperature); // Print temperature
  Serial.print("Humidity    : "); Serial.println(humidity);    // Print humidity
  Serial.print("Wind_Speed  : "); Serial.println(wind_speed);  // Print wind speed
  Serial.print("Visibility  : "); Serial.println(visibility);  // Print visibility
  Serial.println("=========================");               // Print footer line

  // Set the weather flag based on the weather description
  if (weather.indexOf("clouds") != -1 || weather.indexOf("Clouds") != -1 ) { // Check for clouds
    weather_flag = 1;                    // Set cloudy icon

  } else if (weather.indexOf("clear sky") != -1 || weather.indexOf("Clear sky") != -1) { // Check for clear sky
    weather_flag = 3;                    // Set clear icon

  } else if (weather.indexOf("rain") != -1 || weather.indexOf("Rain") != -1) { // Check for rain
    weather_flag = 5;                    // Set rainy icon

  } else if (weather.indexOf("thunderstorm") != -1 || weather.indexOf("Thunderstorm") != -1) { // Check thunderstorm
    weather_flag = 2;                    // Set thunderstorm icon

  } else if (weather.indexOf("snow") != -1 || weather.indexOf("Snow") != -1) { // Check snow
    weather_flag = 4;                    // Set snow icon

  } else if (weather.indexOf("mist") != -1 || weather.indexOf("Mist") != -1) { // Check mist
    weather_flag = 0;                    // Set mist icon
  }

  return true;                           // Return success
}

// =====================================================
// The UI displays weather information.
// =====================================================
void UI_weather_forecast()               // Function to draw the weather UI
{
  // ================= Initialize screen refresh =================
  EPD_RESET();                           // Reset the E-paper display
  delay(100);                            // Wait for reset completion

  EPD_Init();                            // Initialize the E-paper display
  delay(300);                            // Wait for initialization

  Paint_NewImage(Image_BW, EPD_W, EPD_H, 0, WHITE); // Create a new blank image buffer
  EPD_Full(WHITE);                       // Fill the display with white
  delay(300);                            // Wait for display stability
  
  char buffer[40];                       // Temporary text buffer

  // =====================================================
  // 1️⃣ First, draw all the icons
  // =====================================================
  EPD_ShowPicture(7  , 10 , 184 , 208,  Weather_Num[weather_flag], WHITE); // Draw weather icon
  EPD_ShowPicture(205, 22 , 184 , 88 ,  gImage_city, WHITE);                // Draw city icon
  EPD_ShowPicture(6  , 238, 96  , 40 ,  gImage_wind, WHITE);                // Draw wind icon
  EPD_ShowPicture(205, 120, 184 , 88 ,  gImage_hum , WHITE);                // Draw humidity icon
  EPD_ShowPicture(112, 238, 144 , 40 ,  gImage_tem , WHITE);                // Draw temperature icon
  EPD_ShowPicture(265, 238, 128 , 40 ,  gImage_visi, WHITE);                // Draw visibility icon
 
  // =====================================================
  // 2️⃣ Draw a line again
  // =====================================================
  EPD_DrawLine(0  , 230, 400, 230, BLACK); // Draw bottom horizontal line
  EPD_DrawLine(200, 0  , 200, 230, BLACK); // Draw vertical center line
  EPD_DrawLine(200, 115, 400, 115, BLACK); // Draw middle horizontal line

  // =====================================================
  // 3️⃣ Weather data information
  // =====================================================
  if (city_js.length() > 0)              // Check if city name is valid
  {
    snprintf(buffer, sizeof(buffer), "%s", city_js.c_str()); // Format city name
    EPD_ShowString(290, 74, buffer, 24, BLACK);              // Display city name
  }

  snprintf(buffer, sizeof(buffer), "%s %%", humidity.c_str()); // Format humidity text
  EPD_ShowString(290, 180, buffer, 24, BLACK);                 // Display humidity

  snprintf(buffer, sizeof(buffer), "%s m/s", wind_speed.c_str()); // Format wind speed
  EPD_ShowString(30, 273, buffer, 16, BLACK);                    // Display wind speed

  snprintf(buffer, sizeof(buffer), "%s C", temperature.c_str()); // Format temperature
  EPD_ShowString(160, 273, buffer, 16, BLACK);                   // Display temperature

  float vis_km = visibility.toFloat() / 1000.0; // Convert visibility from meters to km
  snprintf(buffer, sizeof(buffer), "%.1f km", vis_km); // Format visibility text
  EPD_ShowString(300, 273, buffer, 16, BLACK); // Display visibility

  // =====================================================
  // 4️⃣ Refresh only once
  // =====================================================
  EPD_Display_Fast(Image_BW);          // Perform a full display refresh

  delay(1000);                         // Wait before sleeping
  EPD_Sleep();                         // Put the display into sleep mode
}

// =====================================================
// setup
// =====================================================
void setup()
{
  Serial.begin(115200);                // Start serial communication

  pinMode(41, OUTPUT);                 // Configure power control pin
  digitalWrite(41, HIGH);              // Enable display power

  pinMode(7, OUTPUT);                  // Configure display enable pin
  digitalWrite(7, HIGH);               // Turn on display

  EPD_GPIOInit();                      // Initialize display GPIO pins

  EPD_RESET();                         // Reset display
  delay(100);                          // Wait after reset

  EPD_Init();                          // Initialize display
  delay(500);                          // Wait after init

  EPD_Clear();                         // Clear display
  delay(500);                          // Wait after clear

  // ================= WiFi =================
  WiFi.begin(ssid, password);          // Connect to WiFi

  Serial.print("Connecting WiFi");     // Print connection status

  while (WiFi.status() != WL_CONNECTED) // Wait until WiFi connects
  {
    delay(500);                        // Wait 500 ms
    Serial.print(".");                 // Print progress dot
  }

  Serial.println("\nWiFi OK");         // Print successful connection
  Serial.println(WiFi.localIP());      // Print local IP address

  // ================= First display =================
  if (js_analysis())                   // Get weather data
  {
    UI_weather_forecast();             // Show weather on display
  }
}

// =====================================================
// loop
// =====================================================
void loop()
{
  if (millis() - lastTime > timerDelay) // Check if refresh interval passed
  {
    if (js_analysis())                  // Update weather data
    {
      UI_weather_forecast();            // Refresh display
    }

    lastTime = millis();                // Update last refresh time
  }
}