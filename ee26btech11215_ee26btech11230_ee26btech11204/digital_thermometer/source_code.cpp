#include <LiquidCrystal.h>

LiquidCrystal lcd(7,6, 5, 4, 3, 2);

const int sensorPin = A0;
const float Vcc = 5.04; // Measured supply rail voltage

// Custom LSQ Quadratic Coefficients (T = a*X^2 + b*X + c)
const float a = 92.78519902;
const float b = -218.78275173;
const float c = 7.15801199;


void setup() {
  lcd.begin(16, 2);
}

void loop() {
  // 1. Oversample 16 ADC readings for noise reduction
  long adcSum = 0;
  for (int i = 0; i < 16; i++) {
    adcSum += analogRead(sensorPin);
    delay(2);
  }
  float avgADC = (float)adcSum / 16.0;

  // 2. Convert raw ADC average to voltage X
  float X = avgADC * (Vcc / 1023.0);

  // 3. Compute exact temperature using your custom LSQ model
  float tempC = (a * X * X) + (b * X) + c;

  // 4. Update LCD Display
  lcd.setCursor(0, 0);
  lcd.print("V: ");
  lcd.print(X, 3);
  lcd.print("V   ");

  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(tempC, 2);
  lcd.print(" ");
  lcd.write(223); // Degree symbol
  lcd.print("C   ");

  delay(300);
}
