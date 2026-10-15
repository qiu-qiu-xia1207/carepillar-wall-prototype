# carepillar-wall-prototype
Arduino-based interactive mobile wall prototype for Carepillar, integrating RFID authentication, RGB feedback, and bidirectional motor control.

## Interaction

- RFID-based identity authentication
- Green LED feedback for authorized access
- Red LED feedback for unauthorized access
- Bidirectional wall movement triggered by authorized RFID scans
- Alternating forward / backward movement for repeated spatial reconfiguration

## Hardware

- Arduino UNO R3
- D24A / TB6612 Motor Driver
- N20 DC Geared Motors ×2
- I²C RFID Reader
- RGB LED Module
- Dual 18650 Battery Pack

## Code

The full Arduino control logic is available in:

`carepillar_wall_controller.ino`
