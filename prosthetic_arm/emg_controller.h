// Pure signal-processing and control logic for the EMG prosthetic arm.
// No Arduino dependencies, so it can be unit-tested on a PC.
#ifndef EMG_CONTROLLER_H
#define EMG_CONTROLLER_H

#include <stdint.h>

struct EmgConfig {
  uint32_t calibrationMs;   // time at rest used to learn the baseline
  int onAboveBaseline;      // envelope must rise this far above baseline to count as a flex
  int offAboveBaseline;     // ...and fall below this to count as relaxed (hysteresis)
  uint32_t minFlexMs;       // ignore twitches shorter than this
  uint32_t refractoryMs;    // ignore new flexes for this long after a toggle
  int openAngle;            // servo angle for an open hand
  int closedAngle;          // servo angle for a closed grip
  int maxStepDeg;           // max servo movement per update (smooth motion)
};

class EmgController {
 public:
  explicit EmgController(const EmgConfig& c)
      : cfg(c), envelope(0), baseline(0), calSum(0), calCount(0), calibrated(false),
        flexing(false), flexStart(0), flexCounted(false), lastToggle(0), hasToggled(false),
        gripClosed(false), angle(c.openAngle) {}

  // Feed one EMG sample (0-1023) and the time. Returns the servo angle to write.
  int update(int raw, uint32_t nowMs) {
    // Envelope: exponential moving average of the signal (alpha = 1/8).
    envelope += (raw - envelope) / 8.0f;

    if (!calibrated) {
      calSum += raw; calCount++;
      if (nowMs >= cfg.calibrationMs && calCount > 0) {
        baseline = (float)calSum / calCount;
        envelope = baseline;
        calibrated = true;
      }
      return stepToward(cfg.openAngle);
    }

    float above = envelope - baseline;
    if (!flexing && above >= cfg.onAboveBaseline) {
      flexing = true; flexStart = nowMs; flexCounted = false;
    } else if (flexing && above <= cfg.offAboveBaseline) {
      flexing = false;
    }

    // A flex held long enough toggles the grip once.
    if (flexing && !flexCounted && nowMs - flexStart >= cfg.minFlexMs &&
        (!hasToggled || nowMs - lastToggle >= cfg.refractoryMs)) {
      gripClosed = !gripClosed;
      flexCounted = true;
      lastToggle = nowMs; hasToggled = true;
    }
    return stepToward(gripClosed ? cfg.closedAngle : cfg.openAngle);
  }

  bool isCalibrated() const { return calibrated; }
  bool isGripClosed() const { return gripClosed; }
  bool isFlexing() const { return flexing; }
  int getBaseline() const { return (int)baseline; }
  int getEnvelope() const { return (int)envelope; }
  int getAngle() const { return angle; }

 private:
  int stepToward(int target) {
    if (angle < target) angle = (target - angle > cfg.maxStepDeg) ? angle + cfg.maxStepDeg : target;
    else if (angle > target) angle = (angle - target > cfg.maxStepDeg) ? angle - cfg.maxStepDeg : target;
    return angle;
  }

  EmgConfig cfg;
  float envelope, baseline;
  long calSum; long calCount;
  bool calibrated;
  bool flexing; uint32_t flexStart; bool flexCounted;
  uint32_t lastToggle; bool hasToggled;
  bool gripClosed;
  int angle;
};

#endif
