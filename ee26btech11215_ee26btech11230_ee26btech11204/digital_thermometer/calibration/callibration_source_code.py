import numpy as np

X_data = np.array([
    2.359883, 2.374663, 2.394369, 2.414076, 2.438710,
    2.458416, 2.478123, 2.497633, 2.517537, 2.542170,
    2.566803, 2.581583, 2.601260, 2.620997, 2.630850,
    2.650557, 2.665337, 2.689971, 2.714604
])

T_data = np.array([
    7.3, 10.8, 15.1, 20.1, 25.8, 30.2, 34.8,
    39.5, 44.4, 50.5, 56.9, 60.4, 65.5, 71.1,
    73.9, 79.4, 83.2, 90.3, 96.8
])

# 2. Fit a 2nd-order polynomial (Quadratic: T = a*X^2 + b*X + c)
a, b, c = np.polyfit(X_data, T_data, 2)

print(f"const float a = {a:.8f};")
print(f"const float b = {b:.8f};")
print(f"const float c = {c:.8f};")
