#ifndef KNOB_H
#define KNOB_H

#include <FastLED.h>
#include "portal.h"

extern long idleTicker;
extern long activeTicker;
extern float velocity_average_l();
extern float velocity_average_r();
extern CRGB leds_knob_l[NUM_LEDS_KNOB];
extern CRGB leds_knob_r[NUM_LEDS_KNOB];

void knob_idle();
void knob_active(long potL, long potR);

#endif