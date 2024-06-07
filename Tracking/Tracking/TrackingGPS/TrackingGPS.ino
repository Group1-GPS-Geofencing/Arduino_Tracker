// Include required libraries
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// Include configuration file for Wi-Fi and Firebase credentials
#include "Conf.h"

// Define Firebase Data object, Firebase authentication, and configuration
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Specifies the URL for the World Time API to get the current time for Blantyre, Malawi
const char* timeAPiurl = "http://worldtimeapi.org/api/timezone/Africa/Blantyre";

// Create an instance of the TinyGPS++ object
TinyGPSPlus gps;

// Create an instance of the HardwareSerial object for GPS
HardwareSerial gpsSerial(2);  // Use UART2 (GPIO 16, 17)

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);

  // Initialize GPS Serial
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);  // TX, RX

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
}

String getTime() {
  HTTPClient http; // Create an HTTP client object
  http.begin(timeAPiurl); // Initialize the HTTP client with the world time API url
  int httpRensponseCode = http.GET();
  String currentTime = "";

  if(httpRensponseCode == 200){
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload); // Create a JSON document to parse the response
    currentTime = doc["datetime"].as<String>(); // Parse the Json payload
    currentTime.remove(19);
  } else {
    Serial.print("Error on HTTP request: ");
    Serial.println(httpRensponseCode);
  }
  http.end();
  return currentTime;
}

void loop() {
  // Define the path to the Firestore document
  String documentPath = "CurrentLocation/current_Location";

  // Create a FirebaseJson object for storing data
  FirebaseJson content;
  FirebaseJson geoPoint;

  // Read data from the GPS module
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Check if we have a valid GPS location
  if (gps.location.isUpdated()) {
    double latitude = gps.location.lat();
    double longitude = gps.location.lng();

    // Get the current time
    String time = getTime();

    if (time != "") {
      // Create a GeoPoint
      geoPoint.set("geoPointValue/latitude", String(latitude, 6));
      geoPoint.set("geoPointValue/longitude", String(longitude, 6));

      // Add GeoPoint and time to the FirebaseJson object
      content.set("fields/position", geoPoint);
      content.set("fields/time/stringValue", time);

      Serial.print("Update/Add GPS Data with time... ");

      // Use the patchDocument method to update the Firestore document
      if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "position,time")) {
        Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      } else {
        Serial.println(fbdo.errorReason());
      }
    }
  } else {
    Serial.println("No valid GPS data available.");
  }

  // Delay before the next reading (2 minutes)
  delay(120000);
}
