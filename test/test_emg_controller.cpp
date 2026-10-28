// Host-side unit tests for the EMG prosthetic arm logic.
// Build & run:  g++ -std=c++11 -I../prosthetic_arm test_emg_controller.cpp -o t && ./t
#include <cstdio>
#include <cstdlib>
#include "emg_controller.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static EmgConfig cfg() { return {3000UL, 120, 60, 150UL, 600UL, 10, 160, 3}; }

// Simulated EMG: resting level ~200 with +/-15 noise, flex adds `extra`.
static uint32_t feed(EmgController& c, int extra, uint32_t t, uint32_t ms) {
  for (uint32_t end = t + ms; t < end; t += 10) c.update(200 + extra + (rand() % 31 - 15), t);
  return t;
}

int main() {
  srand(42);

  // calibration learns the resting level and keeps the hand open
  { EmgController c(cfg());
    uint32_t t = feed(c, 0, 0, 3100);
    CHECK(c.isCalibrated());
    CHECK(c.getBaseline() > 185 && c.getBaseline() < 215);
    t = feed(c, 0, t, 2000);
    CHECK(!c.isGripClosed()); CHECK(c.getAngle() == 10); }

  // a strong flex closes the grip; the servo moves smoothly, not in one jump
  { EmgController c(cfg());
    uint32_t t = feed(c, 0, 0, 3100);
    t = feed(c, 300, t, 300);
    CHECK(c.isGripClosed());
    CHECK(c.getAngle() > 10 && c.getAngle() < 160);   // still travelling
    t = feed(c, 0, t, 1500);
    CHECK(c.getAngle() == 160);                         // arrived and stays closed after relaxing
    // second flex opens it again
    t = feed(c, 300, t, 300); t = feed(c, 0, t, 1500);
    CHECK(!c.isGripClosed()); CHECK(c.getAngle() == 10); }

  // a short twitch (< 150 ms) is ignored
  { EmgController c(cfg());
    uint32_t t = feed(c, 0, 0, 3100);
    t = feed(c, 300, t, 60);
    t = feed(c, 0, t, 1000);
    CHECK(!c.isGripClosed()); }

  // holding one long flex toggles only once
  { EmgController c(cfg());
    uint32_t t = feed(c, 0, 0, 3100);
    t = feed(c, 300, t, 3000);
    CHECK(c.isGripClosed()); }

  // a weak contraction below the threshold does nothing
  { EmgController c(cfg());
    uint32_t t = feed(c, 0, 0, 3100);
    t = feed(c, 70, t, 1000); t = feed(c, 0, t, 500);
    CHECK(!c.isGripClosed()); }

  if (failures == 0) printf("All EMG controller tests passed.\n");
  return failures == 0 ? 0 : 1;
}
