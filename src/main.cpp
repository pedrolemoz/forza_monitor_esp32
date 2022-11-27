#include <AsyncUDP.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>

// Wireless Connection
const char* ssid = "";
const char* password = "";
const int port = 1101;

// Byte size of a float in C++
const int floatByteSize = 4;

// Forza Horizon 5 Offset
const int offset = 12;

// Data bytes of UDP packet
const int rpmByte = 16;
const int gearByte = 307 + offset;
const int speedByte = 244 + offset;
const int racePositionByte = 302 + offset;
const int isRaceOnByte = 0;
const int lapByte = 300 + offset;

// Factor for speed conversion
const float milesPerHourFactor = 2.237;
const float kilometerPerHourFactor = 3.6;

// Values that WILL change
float rpm;
float speed;
int gear;
int racePosition;
int isRaceOn;
int lap;
bool hasReceivedFirstData = false;

LiquidCrystal_I2C lcd(0x27, 16, 2);
AsyncUDP udp;

float parseFloat(AsyncUDPPacket packet, int byte) {
  char data[floatByteSize];
  memcpy(data, (packet.data() + byte), floatByteSize);
  return *((float*)data);
}

int parseInt(AsyncUDPPacket packet, int byte) {
  return (int)packet.data()[byte];
}

float speedInKMH(float speed) {
  return speed * kilometerPerHourFactor;
}

float speedInMPH(float speed) {
  return speed * milesPerHourFactor;
}

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) delay(500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected!");
  lcd.setCursor(0, 1);
  lcd.printf("%s:%d", WiFi.localIP().toString(), port);

  if (udp.listen(port)) udp.onPacket(
      [](AsyncUDPPacket packet) {
        if (!hasReceivedFirstData) lcd.clear();
        hasReceivedFirstData = true;

        if (packet.length() == 324) {
          rpm = parseFloat(packet, rpmByte);
          speed = parseFloat(packet, speedByte);
          gear = parseInt(packet, gearByte);
          racePosition = parseInt(packet, racePositionByte);
          isRaceOn = parseInt(packet, isRaceOnByte);
          lap = parseInt(packet, lapByte);
        } else {
          lcd.setCursor(0, 0);
          lcd.print("Waiting for data");
          lcd.clear();
        }
      });
}

void loop() {}