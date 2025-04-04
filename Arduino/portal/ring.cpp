#include <Arduino.h>
#include "portal.h"
#include "ring.h"

// Shooting Stars Animation
#define NUM_STARS   5
#define SPEED       30 // Inverse
#define STAR_FADE   200 // (0-255 where 255 is no fade)

// Variable for hue shifting
int startingHue = 0;

void rainbow_cycle() {
  // Fade in
  long brightness = min((idleTicker - IDLE_TICKS - IDLE_TRANSITION_OUT_TIME) / IDLE_TRANSITION_IN_TIME, 255);

  // Turn on LEDs for the first potentiometer (from the front)
  for (int i = 0; i < NUM_LEDS_RING/2; i++) {
    int hue = startingHue + map(i, 0, NUM_LEDS_RING/2 * 2, 0, 255);  // Gradient from starting hue
    leds_ring[i] = CHSV(hue, 255, brightness);  // Rainbow color with full saturation and brightness
  }

  // Turn on LEDs for the second potentiometer (from the back)
  for (int i = NUM_LEDS_RING - 1; i >= NUM_LEDS_RING/2; i--) {
    int hue = startingHue + map(NUM_LEDS_RING - 1 - i, 0, NUM_LEDS_RING/2 * 2, 0, 255);  // Same gradient from starting hue
    leds_ring[i] = CHSV(hue, 255, brightness);  // Rainbow color with full saturation and brightness
  }

  // Slowly shift the hue for the rainbow cycle
  startingHue += 1;  // Increment the hue to slowly cycle through the rainbow
  if (startingHue >= 255) {
    startingHue = 0;  // Wrap around when it reaches 255
  }
}

void ring_idle() {
  rainbow_cycle();
}

bool group_on(int light_index, long pot_pos) {
  long pos = ((pot_pos % (NUM_LEDS_RING * SPEED)) + (NUM_LEDS_RING * SPEED)) % (NUM_LEDS_RING * SPEED); // Handle negatives

  // Reduce to postion in LED index range
  pos = (pos / SPEED) % NUM_LEDS_RING;

  bool on = false;
  for (int i = 0; i < NUM_STARS; i++) {
    if ((light_index + i * NUM_LEDS_RING / NUM_STARS) % NUM_LEDS_RING == pos) {
      on = true;
      continue;
    }
  }
  return on;
}

void shooting_stars(long pot_1, long pot_2) {
  if (activeTicker == 0) {
    fill_solid(leds_ring, NUM_LEDS_RING, CRGB::Black);
  }

  // Fade out
  long brightness;
  if (idleTicker > IDLE_TICKS) {
    brightness = map((float)(idleTicker - IDLE_TICKS) / IDLE_TRANSITION_OUT_TIME * 255, 255, 0, 0, 255);
  } else {
    brightness = 255;
  }

  for (int i = 0; i < NUM_LEDS_RING; i++) {
    leds_ring[i].nscale8(STAR_FADE);  // 230/255 ≈ 0.9 (reduces brightness by ~10%)
  }

  for (int i = 0; i < NUM_LEDS_RING; i++) {
    bool group_1 = group_on(i, pot_1);
    bool group_2 = group_on(i, pot_2);
    long brightness_blue = 0; long brightness_green = 0;
    if (group_1) {
      leds_ring[i].b = brightness;
    } 
    if (group_2) {
      leds_ring[i].g = brightness;
    }
  }
}

void ring_active(long pot_1, long pot_2) {
  shooting_stars(pot_1, pot_2);
}