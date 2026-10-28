// EMG-Controlled Prosthetic Robotic Arm - Arduino Uno
// A muscle sensor (e.g. MyoWare) placed on the muscle above the elbow measures
// electrical activity (EMG). Flexing the muscle toggles the robotic hand between
// open and closed. The first 3 seconds are used to learn the resting level.
//
// Wiring:
//   EMG sensor ENV/SIG -> A0, + -> 5V, - -> GND (electrodes on the upper-arm muscle)
//   Grip servo signal -> D9, servo power from a separate 5-6 V supply (common GND!)
//   Status LED -> D13 (on while the muscle is flexed)

#include "emg_controller.h"

const uint8_t EMG_PIN = A0;
const uint8_t LED_PIN = 13;   // servo is on D9 (Timer1 / OC1A)

EmgConfig config = {
  3000UL,  // 3 s calibration at rest
  120,     // flex when envelope rises 120 above resting level
  60,      // relaxed when it falls below 60 above resting level
  150UL,   // flex must last 150 ms (ignores twitches)
  600UL,   // 0.6 s between toggles
  10,      // open hand angle
  160,     // closed grip angle
  3        // max 3 degrees per 10 ms update (smooth motion)
};

EmgController controller(config);
unsigned long lastUpdate = 0, lastPrint = 0;

// Minimal servo driver on D9 using Timer1 hardware PWM (50 Hz, 0.5 us resolution).
void servoBegin() {
  pinMode(9, OUTPUT);
  TCCR1A = _BV(COM1A1) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11);  // fast PWM, TOP = ICR1, prescaler 8
  ICR1 = 39999;                                   // 16 MHz / 8 / 40000 = 50 Hz
}

void servoWrite(int angle) {
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;
  unsigned int pulseUs = 544 + (unsigned long)angle * (2400 - 544) / 180;
  OCR1A = pulseUs * 2;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  servoBegin();
  servoWrite(config.openAngle);
  Serial.println(F("Prosthetic arm: keep the arm relaxed for calibration..."));
}

void loop() {
  unsigned long now = millis();
  if (now - lastUpdate >= 10) {   // 100 Hz control loop
    lastUpdate = now;
    servoWrite(controller.update(analogRead(EMG_PIN), now));
    digitalWrite(LED_PIN, controller.isFlexing() ? HIGH : LOW);
  }

  if (now - lastPrint >= 250) {
    lastPrint = now;
    if (!controller.isCalibrated()) { Serial.println(F("calibrating...")); return; }
    Serial.print(F("baseline="));
    Serial.print(controller.getBaseline());
    Serial.print(F(" envelope="));
    Serial.print(controller.getEnvelope());
    Serial.print(F(" grip="));
    Serial.print(controller.isGripClosed() ? F("CLOSED") : F("OPEN"));
    Serial.print(F(" angle="));
    Serial.println(controller.getAngle());
  }
}
