#include <FastLED.h>

#define NUM_LEDS    83
#define LED_PIN     8
#define BUTTON_PIN  7
#define POT_PIN_1   A0
#define POT_PIN_2   A1
#define DELAY       10
#define WINDOW      30
#define LED_SPEED   3

// Variables to store the readings
int pot1Values[WINDOW] = {0};
int pot2Values[WINDOW] = {0};
int pot1Index = 0;
int pot2Index = 0;
int buttonState = 0;

// Variable for hue shifting
int startingHue = 0;

CRGB leds[NUM_LEDS];

void setup() {
  // Initialize serial communication at 9600 bits per second
  Serial.begin(9600);
  
  // Set the button pin as input
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
}

void loop() {
  // Read the pot values
  pot1Values[pot1Index] = analogRead(POT_PIN_1);
  pot2Values[pot2Index] = analogRead(POT_PIN_2);

  // Move index and wrap around if needed
  pot1Index = (pot1Index + 1) % WINDOW;
  pot2Index = (pot2Index + 1) % WINDOW;

  // Calculate the average value for each potentiometer
  long pot1Average = 0;
  long pot2Average = 0;

  for (int i = 0; i < WINDOW; i++) {
    pot1Average += pot1Values[i];
    pot2Average += pot2Values[i];
  }
  
  pot1Average = (pot1Average + WINDOW/2) / WINDOW;  // Adding half before dividing rounds to nearest
  pot2Average = (pot2Average + WINDOW/2) / WINDOW;

  // Read the state of the button (HIGH or LOW)
  buttonState = digitalRead(BUTTON_PIN);

  // Print the values to the serial monitor
  Serial.print(pot1Average);
  Serial.print(" ");
  Serial.print(pot2Average);
  Serial.print(" ");
  Serial.println(buttonState); // New line after button state

  //// LEDs
  // Map potentiometer values to LED groups
  int numGroupsPot1 = map(pot1Average, 0, 1023, 0, 41);  // First 41 groups
  int numGroupsPot2 = map(pot2Average, 0, 1023, 41, 0);  // Last 41 groups

  // Clear all LEDs
  FastLED.clear();

  // Turn on LEDs for the first potentiometer (from the front)
  for (int i = 0; i < numGroupsPot1; i++) {
    int hue = startingHue/LED_SPEED + map(i, 0, numGroupsPot1 * 3, 0, 255);  // Gradient from starting hue
    leds[i] = CHSV(hue, 255, 255);  // Rainbow color with full saturation and brightness
  }

  // Turn on LEDs for the second potentiometer (from the back)
  for (int i = NUM_LEDS - 1; i >= NUM_LEDS - numGroupsPot2; i--) {
    int hue = startingHue/LED_SPEED + map(NUM_LEDS - 1 - i, 0, numGroupsPot2 * 3, 0, 255);  // Same gradient from starting hue
    leds[i] = CHSV(hue, 255, 255);  // Rainbow color with full saturation and brightness
  }

  // Slowly shift the hue for the rainbow cycle
  startingHue += 1;  // Increment the hue to slowly cycle through the rainbow
  if (startingHue >= 255*LED_SPEED) {
    startingHue = 0;  // Wrap around when it reaches 255
  }

  // Ensure the middle group (group 42) stays off
  leds[41] = CRGB::Black;

  // Update the LED strip
  FastLED.show();

  delay(DELAY);
}