#include <FastLED.h>
#include <ESP32Servo.h>
#include <WiFiManager.h>
#include <EEPROM.h>

// Pin definitions
#define SERVO_PIN          13
#define TRIGGER_PIN        12
#define SOUND_PIN          14
#define LED_PIN            15
#define NUM_LEDS           2

// Constants
#define MAX_POS           150
#define MIN_POS            30
#define MOVE_INTERVAL       5
#define SERVO_SPEED         1

// Global variables
Servo myservo;
CRGB leds[NUM_LEDS];
int pos = 0;    // Current servo position
volatile bool active = false;

// Settings
struct Settings {
  uint32_t eyeColor;
  // Add other settings here
} settings;

WiFiManager wifiManager;

void setup() 
{
  // Initialize FastLED
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  // Initialize pins
  pinMode(SOUND_PIN, OUTPUT);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  
  // Attach interrupt for trigger
  attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), triggerIsr, RISING);
  
  // Attach servo
  ESP32PWM::allocateTimer(0);
  myservo.setPeriodHertz(50);
  myservo.attach(SERVO_PIN, 500, 2400);
  
  // Initialize random seed
  randomSeed(analogRead(0));
  
  // Serial for debugging
  Serial.begin(115200);

  // Load settings from EEPROM
  EEPROM.begin(sizeof(Settings));
  EEPROM.get(0, settings);

  // Setup WiFiManager
  wifiManager.autoConnect("BirdConfig");

  // Setup custom parameters
  WiFiManagerParameter custom_eye_color("eye_color", "Eye Color (hex)", String(settings.eyeColor, HEX).c_str(), 7);
  wifiManager.addParameter(&custom_eye_color);

  // Set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
}

void loop() 
{
  // Your existing loop code here
  // ...

  // Handle WiFiManager
  wifiManager.process();
}

void triggerIsr() {
  active = true;
}

void birdMove(int dest) {
  // Your existing birdMove code here
}

void maybeTwitch() {
  // Your existing maybeTwitch code here
}

void maybeBlink() {
  if (random(2) == 0) {
    blink();
    if (random(5) == 0) {
      delay(200);
      blink();
    }
  }
}

void blink() {
  CRGB eyeColor = CRGB(settings.eyeColor);
  leds[0] = eyeColor;
  leds[1] = eyeColor;
  FastLED.show();
  delay(100);
  leds[0] = CRGB::Black;
  leds[1] = CRGB::Black;
  FastLED.show();
}

void makeSound() {
  // Your existing makeSound code here
}

void saveConfigCallback() {
  Serial.println("Should save config");
  strcpy(settings.eyeColor, custom_eye_color.getValue());
  EEPROM.put(0, settings);
  EEPROM.commit();
}
