# PT100-Based Digital Thermometer

A digital thermometer built using a PT100 platinum resistance temperature detector (RTD), an Arduino, and a 16×2 character LCD. The project focuses on resistance-based temperature measurement, analogue voltage acquisition, experimental calibration, and a compact hardware implementation.

## Project Team

- Nehan Mohammad
- T. Pranay Siddarth Reddy
- Akshit

**Project Guide:** Prof. Dr. G. V. V. Sharma

## Overview

The PT100 is a resistance temperature detector whose nominal resistance is 100 Ω at 0°C. Its resistance increases with temperature, allowing temperature to be estimated by measuring its resistance.

The Arduino does not measure resistance directly. Instead, a resistor-based sensing circuit converts the resistance variation into a voltage. The Arduino's analogue-to-digital converter (ADC) measures this voltage, and the program calculates the corresponding temperature.

The result is displayed on a 16×2 character LCD.

## Hardware Components

- Arduino board (Arduino Uno-compatible setup)
- PT100 resistance temperature detector
- Reference resistor of approximately 120 Ω, as used in the project circuit
- 16×2 character LCD with a parallel interface
- Potentiometer for LCD contrast adjustment
- Breadboard and jumper wires for prototyping
- Perfboard and soldered connections for the permanent assembly
- Multimeter for continuity, resistance, and voltage measurements

## Working Principle

The measurement process follows these steps:

1. The PT100 resistance changes with temperature.
2. The sensing circuit converts the resistance into a measurable voltage.
3. The Arduino reads the voltage through an analogue input.
4. The program converts the measurement into a temperature estimate using the calibration equation.
5. The calculated temperature and resistance are displayed on the LCD.

The potentiometer adjusts the LCD contrast; it is not part of the temperature-calibration algorithm.

## Calibration Method

### Quadratic Polynomial Least-Squares Regression

The voltage-to-temperature relationship is approximated using a second-degree polynomial:

\[
T = aV^2+bV+c
\]

where:

- \(T\) is the estimated temperature in °C.
- \(V\) is the measured voltage in volts.
- \(a\), \(b\), and \(c\) are experimentally determined calibration coefficients.

The coefficients are obtained from measured voltage-temperature pairs using **ordinary least-squares regression**. This method minimizes the sum of squared differences between the reference temperatures and the temperatures predicted by the polynomial:

\[
\min_{a,b,c}\sum_{i=1}^{N}
\left[T_i-(aV_i^2+bV_i+c)\right]^2
\]

Python and NumPy can be used to determine the coefficients from the experimental dataset. The resulting constants are then transferred to the Arduino implementation.

Example Python fitting code:

```python
import numpy as np

# Replace these arrays with the measured calibration data.
V_data = np.array([2.359883, 2.374663, 2.394369])
T_data = np.array([7.3, 15.1, 20.1])

a, b, c = np.polyfit(V_data, T_data, 2)

print(f"const float a = {a:.8f};")
print(f"const float b = {b:.8f};")
print(f"const float c = {c:.8f};")
```

**Note:** The three measurements above are only an illustrative example. Use the complete experimental dataset for the final calibration. A quadratic fitted to three points passes through those points exactly, whereas fitting all available measurements by least squares balances the errors across the dataset.

## LCD Interface

The project uses a standard 16×2 character LCD connected directly to the Arduino through its parallel interface, without an I²C backpack.

The LCD provides the temperature display, while the potentiometer is used to set a readable contrast level. The exact pin assignments should match the final wiring and the Arduino sketch.

The Arduino `LiquidCrystal` library supports the parallel LCD interface.

## ADC and Voltage Reference

The Arduino ADC converts the analogue input voltage into a digital reading. The voltage calculation depends on the ADC reference voltage, so the reference value used in the program must correspond to the actual hardware configuration.

An incorrect reference voltage can introduce a systematic voltage error and, consequently, a temperature error.

The calibration coefficients must be used consistently with the voltage values and measurement circuit for which they were determined.

## Hardware Development and Troubleshooting

The development process included:

- Testing the LCD independently before integrating the PT100.
- Checking jumper-wire continuity when the display did not work correctly.
- Correcting reversed LCD data connections that caused corrupted characters.
- Verifying the circuit with a multimeter before permanent assembly.
- Transferring the verified circuit to perfboard.
- Correcting an unintended solder bridge across the PT100 terminals.
- Designing a compact enclosure using OpenSCAD.
- Comparing thermometer readings against a reference thermometer during calibration.

These steps helped isolate hardware faults from software and calibration issues.

## Limitations

- Calibration accuracy depends on the quality and range of the experimental data.
- The polynomial should not be assumed accurate outside the calibrated voltage and temperature range.
- Incorrect ADC reference voltage, resistor values, lead resistance, contact resistance, and wiring faults can affect the readings.
- Averaging ADC readings can reduce short-term fluctuations but cannot eliminate systematic errors.
- Accuracy should be evaluated by comparing the thermometer against a suitable reference at multiple temperatures.

The project is an experimental thermometer prototype. Its suitability for industrial measurement requires further validation and uncertainty analysis.

## References

1. **Project repository — PT100 thermometer:**  
   https://github.com/gadepall/pt100

2. **Arduino Uno R3 documentation:**  
   https://docs.arduino.cc/hardware/uno-rev3/

3. **Arduino Uno R3 datasheet:**  
   https://docs.arduino.cc/resources/datasheets/A000066-datasheet.pdf

4. **Arduino LCD display tutorial:**  
   https://docs.arduino.cc/learn/electronics/lcd-displays/

5. **Arduino LiquidCrystal library:**  
   https://github.com/arduino-libraries/LiquidCrystal

6. **Arduino `analogRead()` reference:**  
   https://docs.arduino.cc/language-reference/en/functions/analog-io/analogRead/

7. **Arduino `analogReference()` reference:**  
   https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReference/

8. **NumPy `polyfit()` documentation:**  
   https://numpy.org/doc/stable/reference/generated/numpy.polyfit.html

9. **NumPy polynomial least-squares fitting:**  
   https://numpy.org/doc/stable/reference/generated/numpy.polynomial.polynomial.polyfit.html

## Project Demonstration

https://youtube.com/shorts/N8T5c-BsDCI?feature=share

## License

https://www.gnu.org/licenses/gpl-3.0.en.html
