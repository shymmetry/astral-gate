#include <FastLED.h>
#include <Encoder.h>

#define NUM_LEDS                  250
#define LED_PIN                   8
#define BUTTON_PIN                7
#define POT_PIN_1A                2
#define POT_PIN_1B                4
#define POT_PIN_2A                3
#define POT_PIN_2B                5
#define DELAY                     10
#define POT_CACHE_SIZE            50
#define IDLE_SLACK                5
#define IDLE_TICKS                50
#define IDLE_TRANSITION_OUT_TIME  100
#define IDLE_TRANSITION_IN_TIME   5

// Shooting Stars Animation
#define SPEED       10 // Inverse
#define STAR_FADE   200 // (0-255 where 255 is no fade)

// Variables to store the readings
int pot1Cache[POT_CACHE_SIZE] = {0};
int pot2Cache[POT_CACHE_SIZE] = {0};
int pot1Index = 0;
int pot2Index = 0;
int buttonState = 0;

// Timers
long idle_ticker = 0;
long active_ticker = 0;

// Variable for hue shifting
int startingHue = 0;

CRGB leds[NUM_LEDS];

Encoder myEncoder1(POT_PIN_1A, POT_PIN_1B);
Encoder myEncoder2(POT_PIN_2A, POT_PIN_2B);

void setup() {
  // Initialize serial communication at 9600 bits per second
  Serial.begin(9600);
  
  // Set the button pin as input
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
}

void loop() {
  // Read the pot values
  long pot1 = myEncoder1.read();
  long pot2 = myEncoder2.read();

  check_idle(pot1, pot2);

  // Read the state of the button (HIGH or LOW)
  buttonState = digitalRead(BUTTON_PIN);

  print_state(pot1, pot2, buttonState);

  //// LEDs
  // Clear all LEDs
  if (is_idle(pot1, pot2)) {
    active_ticker = 0;
    rainbow_cycle();
  } else {
    shooting_stars(pot1, pot2);
    active_ticker++;
  }

  // Update the LED strip
  FastLED.show();

  //// Cleanup
  // Pot cache
  pot1Cache[pot1Index] = pot1;
  pot2Cache[pot2Index] = pot2;

  pot1Index = (pot1Index + 1) % POT_CACHE_SIZE;
  pot2Index = (pot2Index + 1) % POT_CACHE_SIZE;

  delay(DELAY);
}

bool check_idle(long pot1, long pot2) {
  long cache_avg1 = 0;
  long cache_avg2 = 0;
  for (int i = 0; i < POT_CACHE_SIZE; i++) {
    cache_avg1 += pot1Cache[i];
    cache_avg2 += pot2Cache[i];
  }
  cache_avg1 = cache_avg1 / POT_CACHE_SIZE;
  cache_avg2 = cache_avg2 / POT_CACHE_SIZE;

  if (IDLE_SLACK > abs(pot1 - cache_avg1) && 
      IDLE_SLACK > abs(pot2 - cache_avg2)) {
    idle_ticker++;
  } else {
    idle_ticker = 0;
  }
}

bool is_idle(long pot1, long pot2) {
  return idle_ticker > (IDLE_TICKS + IDLE_TRANSITION_OUT_TIME);
}

void print_state(long position1, long position2, int buttonState) {
  // Print the values to the serial monitor
  Serial.print(position1);
  Serial.print(" ");
  Serial.print(position2);
  Serial.print(" ");
  Serial.println(buttonState); // New line after button state
}

void rainbow_cycle() {
  long brightness = min((idle_ticker - IDLE_TICKS - IDLE_TRANSITION_OUT_TIME) / IDLE_TRANSITION_IN_TIME, 255);

  // Turn on LEDs for the first potentiometer (from the front)
  for (int i = 0; i < NUM_LEDS/2; i++) {
    int hue = startingHue + map(i, 0, NUM_LEDS/2 * 2, 0, 255);  // Gradient from starting hue
    leds[i] = CHSV(hue, 255, brightness);  // Rainbow color with full saturation and brightness
  }

  // Turn on LEDs for the second potentiometer (from the back)
  for (int i = NUM_LEDS - 1; i >= NUM_LEDS/2; i--) {
    int hue = startingHue + map(NUM_LEDS - 1 - i, 0, NUM_LEDS/2 * 2, 0, 255);  // Same gradient from starting hue
    leds[i] = CHSV(hue, 255, brightness);  // Rainbow color with full saturation and brightness
  }

  // Slowly shift the hue for the rainbow cycle
  startingHue += 1;  // Increment the hue to slowly cycle through the rainbow
  if (startingHue >= 255) {
    startingHue = 0;  // Wrap around when it reaches 255
  }
}

bool group_on(int light_index, long pot_pos) {
  long pos = ((pot_pos % (NUM_LEDS * SPEED)) + (NUM_LEDS * SPEED)) % (NUM_LEDS * SPEED); // Handle negatives

  // Reduce to postion in LED index range
  pos = (pos / SPEED) % NUM_LEDS;

  return light_index == pos || ((light_index + NUM_LEDS / 2) % NUM_LEDS) == pos;
}

void shooting_stars(long pot_1, long pot_2) {
  if (active_ticker == 0) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }

  // Fade out
  long brightness;
  if (idle_ticker > IDLE_TICKS) {
    brightness = map((float)(idle_ticker - IDLE_TICKS) / IDLE_TRANSITION_OUT_TIME * 255, 255, 0, 0, 255);
  } else {
    brightness = 255;
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(STAR_FADE);  // 230/255 ≈ 0.9 (reduces brightness by ~10%)
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    bool group_1 = group_on(i, pot_1);
    bool group_2 = group_on(i, pot_2);
    long brightness_blue = 0; long brightness_green = 0;
    if (group_1) {
      leds[i].b = brightness;
    } 
    if (group_2) {
      leds[i].g = brightness;
    }
  }
}