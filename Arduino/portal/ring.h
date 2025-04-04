#ifndef RING_H
#define RING_H

#include <FastLED.h>
#include "portal.h"

extern long idleTicker;
extern long activeTicker;
extern CRGB leds_ring[NUM_LEDS_RING];

void ring_idle();
void ring_active(long pot_1, long pot_2);

#endif