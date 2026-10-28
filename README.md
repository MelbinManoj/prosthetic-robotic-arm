# 🦾 EMG-Controlled Prosthetic Robotic Arm

> A low-cost prosthetic hand controlled by muscle signals (EMG) from the upper arm, built with an Arduino Uno and a servo.

![badge](https://img.shields.io/badge/Arduino%20Uno-00878F?logo=arduino&logoColor=white&style=flat) ![badge](https://img.shields.io/badge/C%2B%2B-00599C?logo=cplusplus&logoColor=white&style=flat) ![badge](https://img.shields.io/badge/Biomedical-8E44AD?style=flat)
![tests](https://github.com/MelbinManoj/prosthetic-robotic-arm/actions/workflows/tests.yml/badge.svg)

> 📝 **About this code:** I built this project during my B.Tech, but the original source code was lost. The code here is a **rebuilt version (2026)** based on the original design. It compiles for the Arduino Uno, and its logic is checked by automated tests that run on simulated sensor data. It has not yet been re-tested on the physical hardware.

## 📖 Overview

Many prosthetics are either purely cosmetic or very expensive. This project reads the **electrical activity of the muscle above the elbow** (EMG) with a muscle sensor. When the user flexes that muscle, the robotic hand **opens or closes its grip**, giving back a useful grasping movement.

## ✨ Features

- **Automatic calibration**: learns the resting muscle level during the first 3 s
- Signal processing: an exponential moving-average **envelope** smooths the raw EMG
- **Hysteresis** thresholds (flex above +120, relax below +60) for stable detection
- Ignores twitches under 150 ms and waits 0.6 s between toggles
- Holding a long flex toggles only once; the grip doesn't flicker
- **Smooth servo motion** of 3° per 10 ms, using a dependency-free Timer1 servo driver

**States:** Calibrating → each deliberate flex toggles `OPEN` ⇄ `CLOSED`

## 🧰 Components

| Component | Qty |
|---|:---:|
| Arduino Uno | 1 |
| EMG muscle sensor (e.g. MyoWare) + electrodes | 1 |
| MG996R / SG90 servo for the grip | 1 |
| 3D-printed or cardboard hand with tendon strings | 1 |
| 5–6 V supply for the servo | 1 |
| Jumper wires | — |

## 🔌 Wiring

| Connection | Arduino pin |
|---|---|
| EMG ENV / SIG | A0 |
| EMG + / − | 5V / GND |
| Electrodes | Muscle above the elbow (biceps), reference on a bony spot |
| Servo signal | D9 |
| Servo power | Separate 5–6 V supply, **common GND** with Arduino |
| Status LED | D13 (on while flexing) |

## 🚀 Getting Started

1. Open [`prosthetic_arm/prosthetic_arm.ino`](./prosthetic_arm/prosthetic_arm.ino) in the Arduino IDE. The `emg_controller.h` file in the same folder opens with it.
2. Select **Arduino Uno** (or Nano) and upload. No extra libraries are needed.
3. Open the Serial Monitor to watch the live readings.
4. **Calibrate:** Watch `baseline` and `envelope` on the Serial Monitor (115200 baud) while flexing, and tune `onAboveBaseline` / `offAboveBaseline` to your sensor and electrode placement.

## 🧪 Tests

All decision logic lives in [`emg_controller.h`](./prosthetic_arm/emg_controller.h), with no Arduino dependencies, so it can be tested on a PC with simulated sensor data:

```bash
cd test
g++ -std=c++11 -I../prosthetic_arm test_emg_controller.cpp -o test && ./test
```

What's tested:

- ✅ Calibration learns the resting level and keeps the hand open
- ✅ A strong flex closes the grip, with smooth servo travel
- ✅ A second flex opens it again
- ✅ A short twitch (< 150 ms) is ignored
- ✅ A long continuous flex toggles only once
- ✅ A weak contraction below the threshold does nothing

These tests run automatically on every push with GitHub Actions.

## 📂 Project Structure

```text
prosthetic-robotic-arm/
├── prosthetic_arm/
│   ├── prosthetic_arm.ino      # hardware layer: pins, sensors, outputs
│   └── emg_controller.h      # decision logic (unit-tested)
├── test/
│   └── test_emg_controller.cpp
└── README.md
```

## 👤 Author

**Melbin Manoj** — [GitHub](https://github.com/MelbinManoj) · [LinkedIn](https://www.linkedin.com/in/melbin-manoj-8ba512269/)
