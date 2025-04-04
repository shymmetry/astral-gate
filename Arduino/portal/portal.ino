#include <FastLED.h>
#include <Encoder.h>
#include "portal.h"
#include "knob.h"
#include "ring.h"

#define LED_RING_PIN              8
#define LED_KNOB_L_PIN            9
#define LED_KNOB_R_PIN            10
#define BUTTON_PIN                7
#define POT_PIN_L1                2
#define POT_PIN_L2                4
#define POT_PIN_R1                3
#define POT_PIN_R2                5
#define DELAY                     10
#define POT_CACHE_SIZE            10
#define VELOCITY_CACHE_SIZE       3

// Variables to store pot (encoder) readings
long potLCache[POT_CACHE_SIZE] = {0};
long potRCache[POT_CACHE_SIZE] = {0};
int potIndex = 0;

// Pot (encoder) velocity
long velocityLCache[VELOCITY_CACHE_SIZE] = {0};
long velocityRCache[VELOCITY_CACHE_SIZE] = {0};
int velocityIndex = 0;
unsigned long lastTime = 0;

int buttonState = 0;

// Timers
long idleTicker = IDLE_TICKS + IDLE_TRANSITION_OUT_TIME + 1;
long activeTicker = 0;

CRGB leds_ring[NUM_LEDS_RING];
CRGB leds_knob_l[NUM_LEDS_KNOB];
CRGB leds_knob_r[NUM_LEDS_KNOB];

Encoder myEncoderL(POT_PIN_L1, POT_PIN_L2);
Encoder myEncoderR(POT_PIN_R1, POT_PIN_R2);

void setup() {
  // Initialize serial communication at 9600 bits per second
  Serial.begin(9600);
  
  // Set the button pin as input
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  FastLED.addLeds<WS2812, LED_RING_PIN, GRB>(leds_ring, NUM_LEDS_RING);
  FastLED.addLeds<WS2812, LED_KNOB_L_PIN, GRB>(leds_knob_l, NUM_LEDS_KNOB);
  FastLED.addLeds<WS2812, LED_KNOB_R_PIN, GRB>(leds_knob_r, NUM_LEDS_KNOB);
  FastLED.clear();
  FastLED.show();

  lastTime = millis();
}

void loop() {
  // Read the pot values
  long potL = myEncoderL.read();
  long potR = myEncoderR.read();

  // Calculate velocities
  long currentTime = millis();
  unsigned long deltaTime = currentTime - lastTime;
  
  long deltaPosL = abs(potL - potLCache[last_cache_index()]);
  long deltaPosR = abs(potR - potRCache[last_cache_index()]);

  float velocityL = (float)deltaPosL / (deltaTime / 1000.0);
  float velocityR = (float)deltaPosR / (deltaTime / 1000.0);

  check_idle(potL, potR);

  // Read the state of the button (HIGH or LOW)
  buttonState = digitalRead(BUTTON_PIN);

  //print_state(potL, potR, buttonState);

  //// LEDs
  // Clear all LEDs
  if (is_idle()) {
    activeTicker = 0;
    ring_idle();
    knob_idle();
  } else {
    ring_active(potL, potR);
    knob_active(potL, potR);
    activeTicker++;
  }

  // Update the LED strip
  FastLED.show();

  //// Cleanup
  // Pot cache
  potLCache[potIndex] = potL;
  potRCache[potIndex] = potR;

  // Velocity cache
  velocityLCache[velocityIndex] = velocityL;
  velocityRCache[velocityIndex] = velocityR;

  potIndex = (potIndex + 1) % POT_CACHE_SIZE;
  velocityIndex = (velocityIndex + 1) % VELOCITY_CACHE_SIZE;
  lastTime = currentTime;

  delay(DELAY);
}

float velocity_average_l() {
  float sum = 0.0;
  int length = sizeof(velocityLCache) / sizeof(velocityLCache[0]); // 20 / 4 = 5
  for (int i = 0; i < length; ++i) {
    sum += velocityLCache[i];
  }

  return sum / length;
}

float velocity_average_r() {
  float sum = 0.0;
  int length = sizeof(velocityRCache) / sizeof(velocityRCache[0]); // 20 / 4 = 5
  for (int i = 0; i < length; ++i) {
    sum += velocityRCache[i];
  }

  return sum / length;
}

int last_cache_index() {
  int newIndex = potIndex - 1;
  // Perform a newIndex % POT_CACHE_SIZE accounting for negatives (C++ modulo doesn't behave like mathematical)
  return (newIndex % POT_CACHE_SIZE + POT_CACHE_SIZE) % POT_CACHE_SIZE;
}

void check_idle(long potL, long potR) {
  long cache_avgL = 0;
  long cache_avgR = 0;
  for (int i = 0; i < POT_CACHE_SIZE; i++) {
    cache_avgL += potLCache[i];
    cache_avgR += potRCache[i];
  }
  cache_avgL = cache_avgL / POT_CACHE_SIZE;
  cache_avgR = cache_avgR / POT_CACHE_SIZE;

  if (IDLE_SLACK > abs(potL - cache_avgL) && 
      IDLE_SLACK > abs(potR - cache_avgR)) {
    idleTicker++;
  } else {
    idleTicker = 0;
  }
}

bool is_idle() {
  return idleTicker > (IDLE_TICKS + IDLE_TRANSITION_OUT_TIME);
}

void print_state(long potL, long potR, int buttonState) {
  Serial.print(potL);
  Serial.print(" ");
  Serial.print(potR);
  Serial.print(" ");
  Serial.println(buttonState);
}