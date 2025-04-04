#include <FastLED.h>
#include "portal.h"
#include "knob.h"

#define NUM_NODES         6
#define NUM_LEDS_CENTER   9
#define NUM_LEDS_RING     24

// Blue to purple hue range (approx 160 to 200 on the HSV wheel)
const uint8_t MIN_HUE = 160;
const uint8_t MAX_HUE = 232;

uint8_t hueOffset = MIN_HUE;  // Used to animate the rainbow over time
uint8_t chasePos = 0;

float pulseT = 0;
float pulseSpeed = 0.12;

// For handling slowing down idle
static unsigned long lastUpdate = 0;
const unsigned long idleDelay = 50;

int activeHue = 0;

void active_nodes(long fade_brightness) {
  fill_solid(leds_knob_l, NUM_LEDS_KNOB, CRGB::Black);
  fill_solid(leds_knob_r, NUM_LEDS_KNOB, CRGB::Black);

  int MAX_VELOCITY = 2000;
  int brightnessL = (int)(min(velocity_average_l(), MAX_VELOCITY) * 255 / MAX_VELOCITY);
  int brightnessR = (int)(min(velocity_average_r(), MAX_VELOCITY) * 255 / MAX_VELOCITY);

  // Center
  for (int i = 0; i < NUM_LEDS_CENTER; i++) {
    leds_knob_l[i] = CHSV(255, 0, brightnessL);
    leds_knob_r[i] = CHSV(255, 0, brightnessR);
  }

  for (int i = 0; i < NUM_LEDS_RING; i++) {
    if (i % (NUM_LEDS_RING / NUM_NODES) == 3) {
      leds_knob_l[NUM_LEDS_CENTER + i] = CHSV(255, 0, brightnessL);
      leds_knob_r[NUM_LEDS_CENTER + i] = CHSV(255, 0, brightnessR);
    } else {
      int hue = activeHue + i * 10;
      int clawBrightnessL = (brightnessL / 2 + 122) * (fade_brightness / 255.0);
      int clawBrightnessR = (brightnessR / 2 + 122) * (fade_brightness / 255.0);
      leds_knob_l[NUM_LEDS_CENTER + i] = CHSV(hue, 255, clawBrightnessL);
      leds_knob_r[NUM_LEDS_CENTER + i] = CHSV(hue, 255, clawBrightnessR);
    }
  }
  activeHue = (activeHue + 1) % 255;
}

void knob_active(long potL, long potR) {
  // Fade out
  long brightness;
  if (idleTicker > IDLE_TICKS) {
    brightness = map((float)(idleTicker - IDLE_TICKS) / IDLE_TRANSITION_OUT_TIME * 255, 255, 0, 0, 255);
  } else {
    brightness = 255;
  }

  active_nodes(brightness);
}



void pulse_center(long dim) {
  // Calculate brightness using a sine wave
  uint8_t max_brightness = 255;
  uint8_t min_brightness = 0;

  // Get normalized sine (range 0–1)
  float s = (sin(pulseT) + 1.0) * 0.5;
  // Apply curve
  s = pow(s, 2.0);
  // Hard cutoff to avoid lingering dim glow
  if (s < 0.1) s = 0.0;
  // Scale to brightness range
  float brightness = s * (max_brightness - min_brightness) + min_brightness;

  // Center
  for (int i = 0; i < NUM_LEDS_CENTER; i++) {
    leds_knob_l[i] = CHSV(160, 255, (int)(brightness * dim / 255));
    leds_knob_r[i] = CHSV(160, 255, (int)(brightness * dim / 255));
  }

  pulseT += pulseSpeed;  // Controls speed of the wave — lower is slower
}

void color_chase_claws(long dim) {
  for (int i = 0; i < NUM_LEDS_RING; i++) {
    int opposite_i = NUM_LEDS_RING - 1 - i;
    uint8_t brightness = i % (NUM_LEDS_RING / NUM_NODES) == 3 ? 255 : 155;
    brightness = (int)(brightness * dim / 255);

    uint8_t hue = (hueOffset + i) * 3 % (MAX_HUE - MIN_HUE);

    leds_knob_l[NUM_LEDS_CENTER + i] = CHSV(MIN_HUE + hue, 255, brightness);
    leds_knob_r[NUM_LEDS_CENTER + opposite_i] = CHSV(MIN_HUE + hue, 255, brightness);
  }

  hueOffset++;
  if (hueOffset > MAX_HUE - MIN_HUE) hueOffset = 0;
}

void knob_idle() {
  if (millis() - lastUpdate >= idleDelay) {
    lastUpdate = millis();

    // Fade in
    long dim = min((idleTicker - IDLE_TICKS - IDLE_TRANSITION_OUT_TIME) / IDLE_TRANSITION_IN_TIME, 255);

    pulse_center(dim);
    color_chase_claws(dim);
  }
}