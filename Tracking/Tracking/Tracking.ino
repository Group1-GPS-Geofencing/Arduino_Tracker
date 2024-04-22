#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

static const int RXpin = 4, TXpin = 3;
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
int temp = 0,i;

SoftwareSerial ss(RXpin, TXpin);
String stringval = "";

void setup() {
  Serial.begin(9600);
  ss.begin(GPSBaud);
}

void loop() {

}
