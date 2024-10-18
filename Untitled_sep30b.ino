#include "arduino_secrets.h"
#include "thingProperties.h"
#include <DHT.h>            // Library for DHT sensor
#include <TFT_eSPI.h>       // TFT display library
#include <WiFiUdp.h>        // For NTP time synchronization
#include <NTPClient.h>      // NTP client for getting time from the internet

#define DHTPIN 4            // DHT sensor pin
#define DHTTYPE DHT11       // DHT sensor type (DHT11)
#define BACKLIGHT_PIN 5     // Pin controlling the display's backlight

DHT dht(DHTPIN, DHTTYPE);   // DHT sensor instance
TFT_eSPI tft = TFT_eSPI();  // TFT display instance

WiFiUDP ntpUDP;             // UDP instance for NTPClient
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600 * 2); // Set timezone to +2 hours for CET (adjust for daylight savings)

void setup() {
  Serial.begin(9600);
  delay(1500); // Wait for the serial port to open

  dht.begin(); // Initialize DHT sensor

  pinMode(BACKLIGHT_PIN, OUTPUT); // Set backlight pin as output
  analogWrite(BACKLIGHT_PIN, 255); // Set backlight to maximum brightness

  // Initialize TFT display
  tft.init();
  tft.setRotation(1);  // Set display orientation
  tft.fillScreen(TFT_BLACK);  // Clear the display with black

  // Initialize NTP client
  timeClient.begin();
  timeClient.update();

  // Initialize Arduino IoT Cloud settings
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2); // Set debug message level
  ArduinoCloud.printDebugInfo(); // Print network and IoT Cloud connection info
}

void loop() {
  ArduinoCloud.update(); // Update IoT Cloud state

  static unsigned long previousMillis = 0;
  const long interval = 5000; // 5-second measurement interval

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    status = true; // Set status to true during sampling
    ArduinoCloud.update();

    // Read sensor data
    float temp = dht.readTemperature(); // Read temperature in Celsius
    float hum = dht.readHumidity();     // Read humidity

    if (!isnan(hum) && !isnan(temp)) {
      // Update Cloud variables
      temperature = temp;
      humidity = hum;

      // Display data on the screen
      displayData(temp, hum);
    } else {
      Serial.println("Failed to read from DHT sensor!");
    }
    
    status = false; // Reset status after sampling
    ArduinoCloud.update();
  }

  // Update NTP time
  timeClient.update();
  
  // Display time at the bottom of the screen
  displayTime();
}

// Display temperature and humidity on the screen
void displayData(float temp, float hum) {
  tft.fillScreen(TFT_BLACK);  // Clear the screen
  tft.setTextColor(TFT_WHITE);  // White text color

  // Display temperature
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(10, 10);
  tft.setTextSize(3);
  tft.print("Temp:");
  tft.print(temp);
  tft.println("C");

  // Display humidity
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(10, 70);
  tft.setTextSize(3);
  tft.print("Hum:");
  tft.print(hum);
  tft.println("%");
}

// Display the current time at the bottom of the screen
void displayTime() {
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(10, 150);  // Adjust position to the bottom
  tft.setTextSize(2);
  tft.print("Time: ");
  tft.print(timeClient.getFormattedTime());
}

// Automatically generated functions when Cloud variables change
void onTemperatureChange() {
  // Handle temperature change
}

void onHumidityChange() {
  // Handle humidity change
}

void onBacklightChange() {
  analogWrite(BACKLIGHT_PIN, backlight); // Adjust backlight based on Cloud variable
}

void onStatusChange() {
  // Handle status change
}
