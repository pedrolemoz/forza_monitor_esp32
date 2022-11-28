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
bool shouldClearDisplay = false;
bool isWaitingForData = false;

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

const char* getOrdinalSuffix(int number) {
  switch (number) {
    case 1:
      return "st";
    case 2:
      return "nd";
    case 3:
      return "rd";
    default:
      return "th";
  }
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
  shouldClearDisplay = true;

  if (udp.listen(PORT)) udp.onPacket(
      [](AsyncUDPPacket packet) {
        if (shouldClearDisplay) {
          lcd.clear();
        }
        shouldClearDisplay = false;

        if (packet.length() == 324) {
          isRaceOn = parseInt(packet, IS_RACE_ON_BYTE);
          rpm = parseFloat(packet, RPM_BYTE);
          speed = parseFloat(packet, SPEED_BYTE);
          gear = parseInt(packet, GEAR_BYTE);
          racePosition = parseInt(packet, RACE_POSITION_BYTE);
          lap = parseInt(packet, LAP_BYTE);

          if (isRaceOn) {
            if (isWaitingForData) {
              lcd.clear();

              isWaitingForData = false;
            }

            lcd.setCursor(0, 0);
            lcd.printf("%.0f KM/H ", speedInKMH(speed));

            lcd.setCursor(10, 0);
            lcd.print("GEAR ");
            if (gear == 0) {
              lcd.print("R");
            } else {
              lcd.printf("%d", gear);
            }

            lcd.setCursor(0, 1);
            lcd.printf("%.0f RPM ", rpm);

            if (racePosition != 0) {
              lcd.setCursor(10, 1);
              lcd.printf("%d%s", racePosition, getOrdinalSuffix(racePosition));
            }
          } else {
            lcd.setCursor(0, 0);
            lcd.print("Waiting for data");
            lcd.setCursor(0, 1);
            lcd.print("Game is paused");
            if (shouldClearDisplay) shouldClearDisplay = false;
            if (!isWaitingForData) isWaitingForData = true;
          }
        }
      });
}

void loop() {}