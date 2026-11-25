#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <esp_sleep.h>  // ESP32 uyqu kutubxonasi
#include <stdlib.h>     // Tasodifiy sonlar uchun
#include "secrets.h"      // secrets.example.h dan nusxalang

#define CE_PIN 0
#define CSN_PIN 1
#define btn1 2

#define W1 9   // 9-pinni LED
#define W2 10  // 10-pinni LED

RF24 radio(CE_PIN, CSN_PIN);
// Radio juftlash manzili secrets.h da (asosiy qurilmadagi bilan bir xil bo'lsin)
const byte address[6] = SECRET_RADIO_ADDRESS;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 500;

bool lastButtonState = HIGH;
bool buttonPressed = false;

void setup() {
  pinMode(btn1, INPUT_PULLUP);
  pinMode(W1, OUTPUT);
  pinMode(W2, OUTPUT);

  Serial.begin(115200);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_HIGH);
  radio.stopListening();

  // ESP32 uyqudan uyg'onish uchun GPIO2 pinini sozlash
  esp_deep_sleep_enable_gpio_wakeup(1ULL << GPIO_NUM_2, ESP_GPIO_WAKEUP_GPIO_HIGH);
  // Agar ESP32 uyqudan uyg'ongan bo'lsa, bu yerga kirishadi
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1) {
    Serial.println("Woke up from external GPIO interrupt.");
  }
  // digitalWrite(W1, HIGH);
  // digitalWrite(W2, LOW);
  // delay(2000);
  // digitalWrite(W1, LOW);
  // digitalWrite(W2, HIGH);
  // delay(2000);
  // digitalWrite(W1, LOW);
  // digitalWrite(W2, HIGH);
}

// Animatsion effektlar
void blinkLEDs(int led1, int led2, int times, int delayTime) {
  for (int i = 0; i < times; i++) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    delay(delayTime);
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    delay(delayTime);
  }
}

void fadeLEDs(int led1, int led2, int delayTime) {
  for (int i = 0; i <= 255; i += 5) {
    analogWrite(led1, i);
    analogWrite(led2, 255 - i);
    delay(delayTime);
  }
  for (int i = 255; i >= 0; i -= 5) {
    analogWrite(led1, i);
    analogWrite(led2, 255 - i);
    delay(delayTime);
  }
}

void alternateLEDs(int led1, int led2, int times, int delayTime) {
  for (int i = 0; i < times; i++) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
    delay(delayTime);
    digitalWrite(led1, LOW);
    digitalWrite(led2, HIGH);
    delay(delayTime);
  }
}

void randomBlinking(int led1, int led2, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(led1, random(0, 2));
    digitalWrite(led2, random(0, 2));
    delay(random(100, 500));
  }
}


int counter = 0;

void loop() {
  int currentState = digitalRead(btn1);

  if (currentState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    counter++;
    if (currentState == HIGH && !buttonPressed) {
      Serial.println("Button Pressed!");
      counter = 0;
      // Tasodifiy animatsiyani ishga tushirish
      alternateLEDs(W1, W2, 5, 300);

      const char text[] = "HA";
      bool success = radio.write(&text, sizeof(text));

      if (success) {
        digitalWrite(W2, HIGH);
        delay(1000);
        digitalWrite(W2, LOW);
      } else {
        for (int i = 0; i < 2; i++) {
          fadeLEDs(W1, W2, 20);
        }
      }

      buttonPressed = true;
      delay(100);
    } else if (currentState == HIGH && buttonPressed) {
      buttonPressed = false;
      
    }
  }

  lastButtonState = currentState;

  if (counter >= 20) {
    Serial.println("Going to sleep...");
    esp_deep_sleep_start();
    counter = 0;
  }
}
