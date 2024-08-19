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

// Expanded Settings
struct Settings {
  uint32_t eyeColor;
  int blinkProbability;
  int twitchProbability;
  int soundProbability;
  int minSoundDuration;
  int maxSoundDuration;
} settings;

WiFiManager wifiManager;
WiFiManagerParameter custom_eye_color("eye_color", "Eye Color (hex)", "", 7);
WiFiManagerParameter custom_blink_prob("blink_prob", "Blink Probability (%)", "", 3);
WiFiManagerParameter custom_twitch_prob("twitch_prob", "Twitch Probability (%)", "", 3);
WiFiManagerParameter custom_sound_prob("sound_prob", "Sound Probability (%)", "", 3);
WiFiManagerParameter custom_min_sound_duration("min_sound", "Min Sound Duration (ms)", "", 4);
WiFiManagerParameter custom_max_sound_duration("max_sound", "Max Sound Duration (ms)", "", 4);

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
  wifiManager.addParameter(&custom_eye_color);
  wifiManager.addParameter(&custom_blink_prob);
  wifiManager.addParameter(&custom_twitch_prob);
  wifiManager.addParameter(&custom_sound_prob);
  wifiManager.addParameter(&custom_min_sound_duration);
  wifiManager.addParameter(&custom_max_sound_duration);

  // Set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // Attempt to connect to WiFi or start config portal
  if (!wifiManager.autoConnect("BirdConfig")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
  }

  // If you get here you have connected to the WiFi
  Serial.println("Connected to WiFi");
}

void loop() 
{
  if (active) {
    for (int i = 0; i < 10; i++) {
      int target = random(MIN_POS, MAX_POS);
      
      birdMove(target);
      maybeBlink();
      maybeTwitch();
      makeSound();
      
      delay(500);
    }
    
    turnOffEyes();
    active = false;
  }

  // Handle WiFiManager
  wifiManager.process();
}

void triggerIsr() {
  active = true;
}

void birdMove(int dest) {
  // Existing birdMove code...
}

void maybeTwitch() {
  if (random(100) < settings.twitchProbability) {
    int timesTwitch = random(3, 6);
    for (int i = 0; i < timesTwitch; i++) {
      int currentPos = myservo.read();
      int twitchAmount = random(-10, 11);
      int target = constrain(currentPos + twitchAmount, MIN_POS, MAX_POS);
      birdMove(target);
    }
  }
}

void maybeBlink() {
  if (random(100) < settings.blinkProbability) {
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
  turnOffEyes();
}

void turnOffEyes() {
  leds[0] = CRGB::Black;
  leds[1] = CRGB::Black;
  FastLED.show();
}

void makeSound() {
  if (random(100) < settings.soundProbability) {
    digitalWrite(SOUND_PIN, HIGH);
    delay(random(settings.minSoundDuration, settings.maxSoundDuration + 1));
    digitalWrite(SOUND_PIN, LOW);
  }
}

void saveConfigCallback() {
  Serial.println("Saving configuration");
  
  settings.eyeColor = strtoul(custom_eye_color.getValue(), NULL, 16);
  settings.blinkProbability = atoi(custom_blink_prob.getValue());
  settings.twitchProbability = atoi(custom_twitch_prob.getValue());
  settings.soundProbability = atoi(custom_sound_prob.getValue());
  settings.minSoundDuration = atoi(custom_min_sound_duration.getValue());
  settings.maxSoundDuration = atoi(custom_max_sound_duration.getValue());

  EEPROM.put(0, settings);
  EEPROM.commit();

  Serial.println("Configuration saved");
}
