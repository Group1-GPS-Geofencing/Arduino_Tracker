// Include required libraries
#if defined(ESP32)
#include <WiFi.h>
#include <time.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <time.h>
#endif
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>

//including congigurations file for wifi and firebase credentials
#include "config.h"

// Define Firebase Data object, Firebase authentication, and configuration
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// NTP Server to request time
const char* ntpServer = "pool.ntp.org";
// Time offset in seconds (adjust as needed)
const long  gmtOffset_sec = 7200;  // GMT+2 for Malawi
const int   daylightOffset_sec = 0;

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Print Firebase client version
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  // Assign the API key
  config.api_key = API_KEY;

  // Assign the user sign-in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the callback function for the long-running token generation task
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // Begin Firebase with configuration and authentication
  Firebase.begin(&config, &auth);

  // Reconnect to Wi-Fi if necessary
  Firebase.reconnectWiFi(true);

  // Initialize NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

String getTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "";
  }
  char timeString[30];
  strftime(timeString, sizeof(timeString), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return String(timeString);
}

void loop() {
  // Define the path to the Firestore document
  String documentPath = "EspData/GPS";

  // Create a FirebaseJson object for storing data
  FirebaseJson content;

  // Set dummy GPS coordinates for Chikanda, Zomba, Malawi
  float dummyLatitude = -15.387723;
  float dummyLongitude = 35.3181;

  // Get the current timestamp
  String timestamp = getTime();

  // Add GPS data to FirebaseJson object
  content.set("fields/Latitude/doubleValue", String(dummyLatitude, 6));
  content.set("fields/Longitude/doubleValue", String(dummyLongitude, 6));
  content.set("fields/Timestamp/stringValue", timestamp);

  Serial.print("Update/Add GPS Data with Timestamp... ");

  // Use the patchDocument method to update the Firestore document
  if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "Latitude,Longitude,Timestamp")) {
    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
  } else {
    Serial.println(fbdo.errorReason());
  }

  // Delay before the next reading (60 seconds)
  delay(60000);
}
