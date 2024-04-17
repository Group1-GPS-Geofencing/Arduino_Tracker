#include <LiquidCrystal.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>

LiquidCrystal lcd(8, 7, 6, 5, 4, 3);
TinyGPS gps;
SoftwareSerial gsmSerial(10, 11); // RX, TX for GSM module

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  gsmSerial.begin(9600); // Initialize serial communication for GSM module
}

void loop() {
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second, parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;) {
    while (Serial.available()) {
      char c = Serial.read();
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData) {
    float flat, flon;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    lcd.clear();
    lcd.print("LAT:");
    lcd.print(flat, 5);

    lcd.setCursor(0, 1);
    lcd.print("LON:");
    lcd.print(flon, 5);

    // Send GPS coordinates via GSM
    gsmSerial.print("AT+CMGF=1\r"); // Set SMS mode to text
    delay(100);
    //gsmSerial.print("AT+CMGS=\"+1234567890\"\r"); // Replace with recipient's phone number
    delay(100);
    gsmSerial.print("Latitude: ");
    gsmSerial.print(flat, 5);
    gsmSerial.print(", Longitude: ");
    gsmSerial.print(flon, 5);
    gsmSerial.write(26); // ASCII code of CTRL+Z
    delay(100);
  }

  gps.stats(&chars, &sentences, &failed);
  delay(3000);
}
