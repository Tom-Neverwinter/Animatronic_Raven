#include <Servo.h>

// Pin definitions
#define SERVO_PIN          3
#define TRIGGER_PIN        2
#define LED_PIN           13
#define EYE1_PIN           7
#define EYE2_PIN           8
#define SOUND_PIN          4

// Constants
#define REFRESH_PERIOD_MS  20
#define MAX_POS           150
#define MIN_POS            30
#define MOVE_INTERVAL       5
#define SERVO_SPEED         1

// Global variables
Servo myservo;
int pos = 0;    // Current servo position
volatile bool active = false;

// Function prototypes
void triggerIsr();
void birdMove(int dest);
void maybeTwitch();
void maybeBlink();
void blink();
void makeSound();

void setup() 
{
  // Initialize pins
  pinMode(SOUND_PIN, OUTPUT);
  digitalWrite(SOUND_PIN, LOW);
  pinMode(EYE1_PIN, OUTPUT);
  pinMode(EYE2_PIN, OUTPUT);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  
  // Attach interrupt for trigger
  attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), triggerIsr, RISING);
  
  // Attach servo
  myservo.attach(SERVO_PIN);
  
  // Initialize random seed
  randomSeed(analogRead(0));
  
  // Serial for debugging (optional)
  Serial.begin(9600);
}

void loop() 
{
  if (active) {
    for (int i = 0; i < 10; i++) {
      int target = random(MIN_POS, MAX_POS);
      
      // Perform bird actions
      birdMove(target);
      maybeBlink();
      maybeTwitch();
      makeSound();
      
      delay(500);
    }
    
    // Reset eyes and active state
    digitalWrite(EYE1_PIN, LOW);
    digitalWrite(EYE2_PIN, LOW);
    active = false;
  }
}

// Interrupt Service Routine for trigger
void triggerIsr() {
  active = true;
}

// Move the bird (servo) to a target position
void birdMove(int dest) {
  int start = myservo.read();
  int direction = (dest > start) ? 1 : -1;
  
  for (pos = start; pos != dest; pos += direction * SERVO_SPEED) {
    myservo.write(pos);
    delay(MOVE_INTERVAL);
  }
  myservo.write(dest); // Ensure final position is reached
}

// Random twitching behavior
void maybeTwitch() {
  if (random(3) == 0) { // 1 in 3 chance to twitch
    int timesTwitch = random(3, 6);
    for (int i = 0; i < timesTwitch; i++) {
      int currentPos = myservo.read();
      int twitchAmount = random(-10, 11);
      int target = constrain(currentPos + twitchAmount, MIN_POS, MAX_POS);
      birdMove(target);
    }
  }
}

// Random blinking behavior
void maybeBlink() {
  if (random(2) == 0) { // 50% chance to blink
    blink();
    if (random(5) == 0) { // 20% chance to blink twice
      delay(200);
      blink();
    }
  }
}

// Blink eyes
void blink() {
  digitalWrite(EYE1_PIN, HIGH);
  digitalWrite(EYE2_PIN, HIGH);
  delay(100);
  digitalWrite(EYE1_PIN, LOW);
  digitalWrite(EYE2_PIN, LOW);
}

// Make sound
void makeSound() {
  if (random(10) == 0) { // 10% chance to make sound
    digitalWrite(SOUND_PIN, HIGH);
    delay(random(100, 500)); // Random duration sound
    digitalWrite(SOUND_PIN, LOW);
  }
}
