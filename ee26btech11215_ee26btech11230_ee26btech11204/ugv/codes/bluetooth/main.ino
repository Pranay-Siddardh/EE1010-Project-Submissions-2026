#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>

const int IN1 = 32;
const int IN2 = 33;
const int IN3 = 25;
const int IN4 = 26;
const int ENA = 27;
const int ENB = 14;

const int PWM_FREQ = 5000;
const int PWM_RES  = 8;
const int CH_A = 0;
const int CH_B = 1;

const int MIN_LEVEL   = 3;
const int MAX_LEVEL   = 7;
const int START_LEVEL = 5;
const int MIN_DUTY    = 90;
const int MAX_DUTY    = 255;

const bool INVERT_LEFT  = false;
const bool INVERT_RIGHT = true;
const bool DEBUG        = true;

const float LEFT_TRIM  = 1.00;
const float RIGHT_TRIM = 0.95;

int   level        = START_LEVEL;
bool lastTriangle = false;
bool lastCross    = false;

void pwmBegin(int pin, int channel) {
  ledcAttach(pin, PWM_FREQ, PWM_RES);
}

void pwmWrite(int pin, int channel, int duty) {
  ledcWrite(pin, duty);
}

void driveMotor(int pinA, int pinB, int enPin, int ch, float v, bool invert, float trim) {
  float a = fabsf(v);
  if (a < 1.0) {
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, LOW);
    pwmWrite(enPin, ch, 0);
    return;
  }
  bool fwd = (v > 0);
  if (invert) fwd = !fwd;
  digitalWrite(pinA, fwd ? HIGH : LOW);
  digitalWrite(pinB, fwd ? LOW : HIGH);
  
  int rawDuty = MIN_DUTY + (int)((MAX_DUTY - MIN_DUTY) * (a - 1.0) / 6.0);
  
  int finalDuty = (int)(rawDuty * trim);
  finalDuty = constrain(finalDuty, 0, MAX_DUTY);
  
  pwmWrite(enPin, ch, finalDuty);
}

void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pwmBegin(ENA, CH_A);
  pwmBegin(ENB, CH_B);
  driveMotor(IN1, IN2, ENA, CH_A, 0, false, 1.0);
  driveMotor(IN3, IN4, ENB, CH_B, 0, false, 1.0);

  Dabble.begin("ESP32_RC_Car");
  Serial.println("Ready. In Dabble connect to ESP32_RC_Car, open Gamepad, use DIGITAL mode");
}

void loop() {
  Dabble.processInput();

  bool up    = GamePad.isUpPressed();
  bool down  = GamePad.isDownPressed();
  bool leftB = GamePad.isLeftPressed();
  bool rightB = GamePad.isRightPressed();

  bool tri   = GamePad.isTrianglePressed();
  bool cross = GamePad.isCrossPressed();
  if (tri   && !lastTriangle) level = min(MAX_LEVEL, level + 1);
  if (cross && !lastCross)    level = max(MIN_LEVEL, level - 1);
  lastTriangle = tri;
  lastCross    = cross;

  float y = (up ? level : 0) - (down ? level : 0);
  float x = (rightB ? level : 0) - (leftB ? level : 0);

  float left  = y + x;
  float right = y - x;
  float limit = fmaxf(fabsf(x), fabsf(y));
  float m = fmaxf(fabsf(left), fabsf(right));
  if (m > limit && m > 0) {
    left  *= limit / m;
    right *= limit / m;
  }

  driveMotor(IN1, IN2, ENA, CH_A, left,  INVERT_LEFT,  LEFT_TRIM);
  driveMotor(IN3, IN4, ENB, CH_B, right, INVERT_RIGHT, RIGHT_TRIM);

  if (DEBUG) {
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 200) {
      lastPrint = millis();
      Serial.printf("level=%d  left=%.1f right=%.1f\n", level, left, right);
    }
  }
}
