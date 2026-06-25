#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

// Hardware Pin Definitions for ESP32
const int I2C_SDA = 21;    // Custom SDA Pin
const int I2C_SCL = 20;    // Custom SCL Pin
const int BUZZER_PIN = 42; // Piezo Buzzer Pin

// Button Pin Assignments (Active LOW with internal Pull-up Resistors)
const int BTN_START_PAUSE = 13; 
const int BTN_ADD = 4;         
const int BTN_SUB = 16;         
const int BTN_RESET = 46;       

Adafruit_7segment display = Adafruit_7segment();

// System State Variables
int totalSeconds = 0; 
bool isRunning = false;

unsigned long previousMillis = 0;
const long interval = 1000; 

// Debounce settings for stable button reading
unsigned long lastDebounceTime = 0;
const int debounceDelay = 200; 

void setup() {
  Serial.begin(115200);
  
  // Initialize hardware pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BTN_START_PAUSE, INPUT_PULLUP);
  pinMode(BTN_ADD, INPUT_PULLUP);
  pinMode(BTN_SUB, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);
  
  // Start I2C and Display
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!display.begin(0x70)) {
    Serial.println("HT16K33 Display not found!");
    while (1);
  }
  
  display.setBrightness(15); 
  updateDisplay();
  
  Serial.println("\n--- ESP32 Stopwatch System Ready ---");
}

void loop() {
  handleButtons();

  // Non-blocking timer clock tick
  unsigned long currentMillis = millis();
  if (isRunning && (currentMillis - previousMillis >= interval)) {
    previousMillis = currentMillis;

    if (totalSeconds > 0) {
      totalSeconds--;
      updateDisplay();

      if (totalSeconds == 0) {
        isRunning = false;
        triggerTimeoutAlarm();
      }
    }
  }
}

void handleButtons() {
  if ((millis() - lastDebounceTime) < debounceDelay) return;

  // Start / Pause Control
  if (digitalRead(BTN_START_PAUSE) == LOW) {
    isRunning = !isRunning;
    Serial.print("Timer State Changed. Running: "); Serial.println(isRunning);
    lastDebounceTime = millis();
  }
  
  // Add Time (+1 Minute)
  else if (digitalRead(BTN_ADD) == LOW) {
    totalSeconds += 60;
    if (totalSeconds > 5999) totalSeconds = 5999; // Cap at 99m 59s
    updateDisplay();
    Serial.print("Added 1 Min. Total: "); Serial.println(totalSeconds);
    lastDebounceTime = millis();
  }
  
  // Subtract Time (-1 Minute)
  else if (digitalRead(BTN_SUB) == LOW) {
    if (totalSeconds >= 60) totalSeconds -= 60;
    else totalSeconds = 0;
    updateDisplay();
    Serial.print("Subtracted 1 Min. Total: "); Serial.println(totalSeconds);
    lastDebounceTime = millis();
  }
  
  // Reset System
  else if (digitalRead(BTN_RESET) == LOW) {
    isRunning = false;
    totalSeconds = 0;
    updateDisplay();
    Serial.println("System Reset.");
    lastDebounceTime = millis();
  }
}

void updateDisplay() {
  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;
  
  display.print(minutes * 100 + seconds, DEC);
  display.drawColon(true);
  display.writeDisplay();
}

void triggerTimeoutAlarm() {
  Serial.println("TIMEOUT! Alarm sounding...");
  for (int i = 0; i < 4; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}
