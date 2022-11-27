#include <AsyncUDP.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>

#define SSID "your-ssid"
#define PASSWORD "password"
#define PORT 1101

#define FLOAT_BYTE_SIZE 4

#define FH5_OFFSET 12

#define RPM_BYTE 16
#define GEAR_BYTE 307 + FH5_OFFSET
#define SPEED_BYTE 244 + FH5_OFFSET
#define RACE_POSITION_BYTE 302 + FH5_OFFSET
#define LAP_BYTE 300 + FH5_OFFSET
#define IS_RACE_ON_BYTE 0

#define MPH_FACTOR 2.237
#define KMH_FACTOR 3.6

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
  char data[FLOAT_BYTE_SIZE];
  memcpy(data, (packet.data() + byte), FLOAT_BYTE_SIZE);
  return *((float*)data);
}

int parseInt(AsyncUDPPacket packet, int byte) {
  return (int)packet.data()[byte];
}

float speedInKMH(float speed) {
  return speed * KMH_FACTOR;
}

float speedInMPH(float speed) {
  return speed * MPH_FACTOR;
}

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print(SSID);

  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) delay(500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected!");
  lcd.setCursor(0, 1);
  lcd.printf("%s:%d", WiFi.localIP().toString(), PORT);

  if (udp.listen(PORT)) udp.onPacket(
      [](AsyncUDPPacket packet) {
        if (!hasReceivedFirstData) lcd.clear();
        hasReceivedFirstData = true;

        if (packet.length() == 324) {
          rpm = parseFloat(packet, RPM_BYTE);
          speed = parseFloat(packet, SPEED_BYTE);
          gear = parseInt(packet, GEAR_BYTE);
          racePosition = parseInt(packet, RACE_POSITION_BYTE);
          isRaceOn = parseInt(packet, IS_RACE_ON_BYTE);
          lap = parseInt(packet, LAP_BYTE);
        } else {
          lcd.setCursor(0, 0);
          lcd.print("Waiting for data");
          lcd.clear();
        }
      });
}

void loop() {}