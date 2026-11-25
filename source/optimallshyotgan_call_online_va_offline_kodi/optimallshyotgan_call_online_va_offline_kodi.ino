
#include <WiFi.h>
#include <ESP32WebServer.h>



#include <ArduinoJson.h>
#include <Wire.h>
#include <FS.h>
#include <SPIFFS.h>
#include <PubSubClient.h>

#include "variables.h"
#include "server_functions.h"
#include "pubsub_client.h"
#include "utils.h"



void setup() {
  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {
    Serial.println("Fayl tizimini o‘rnatib bo‘lmadi");
    while (1)
      ;
  } else {

    // saveSettings();
    if (!loadSettings()) {
      Serial.println("Standart qiymatlardan foydalangan holda sozlamalarni yuklab bo‘lmadi.");
    } else {
      Serial.println("Standart qiymatlar o'rnatildi!");
    }
    // SPIFFS.remove("/smart.txt");
    Serial.println("Fayl tizimi o'rnatilgan");
  }
  // RTC sozlamalari
  if (!rtc.begin()) {
    Serial.println("RTC moduli topilmadi");
    while (1)
      ;  // Tizimni to‘xtatish
  }
  // // Joriy vaqtni sozlash (faqat kerak bo'lsa)
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // nRF24 sozlamalari
  radio.begin();
  radio.openReadingPipe(1, address);  // Qabul qilish kanali
  radio.setPALevel(RF24_PA_HIGH);     // Quvvat darajasi
  radio.startListening();             // Qabul qilish rejimiga o‘tkazish

  // LED sozlamalari
  pinMode(LED_SUCCESS, OUTPUT);
  pinMode(LED_FAIL, OUTPUT);
  pinMode(CALL_PIN, OUTPUT);

  digitalWrite(LED_SUCCESS, LOW);  // LEDlarni o‘chirish
  digitalWrite(LED_FAIL, LOW);     // LEDlarni o‘chirish
  digitalWrite(CALL_PIN, LOW);     // releni  o‘chirish

  // Serial.println("SPIFFS formatlash boshlandi...");
  // if (SPIFFS.format()) {
  //   Serial.println("SPIFFS muvaffaqiyatli formatlandi!");
  // } else {
  //   Serial.println("SPIFFS formatlashda xato!");
  // }
  WiFi.mode(WIFI_AP);

  Serial.println();
  Serial.print("Attempting to connect to: ");
  Serial.println(wifi_ssid);

  WiFi.softAP(hostpot_and_device_username, hostpot_password);

  Serial.println("Hotspot created");
  Serial.print("SSID: ");
  Serial.println(hostpot_and_device_username);
  Serial.print("Password: ***");  // parol Serial'ga chiqarilmaydi
  Serial.print(" (len=");
  Serial.print(hostpot_password ? strlen(hostpot_password) : 0);
  Serial.println(")");

  server.on("/call", HTTP_GET, handleRoot);
  server.on("/pin", HTTP_GET, handleHello);

  server.on("/get", HTTP_GET, handle_get_all_GET);
  server.on("/add", HTTP_POST, handle_add_POST);
  server.onNotFound([]() {
    if (server.uri().startsWith("/update/")) {
      handle_update_POST();
    } else if (server.uri().startsWith("/delete/")) {
      handle_delete_POST();
    } else {
      server.send(404, "application/json", "{\"message\":\"Not Found\"}");
    }
    printSettings();
  });

  // server.on("/update", HTTP_PUT, handle_update_POST);
  // server.on("/delete", HTTP_DELETE, handle_delete_POST);
  server.begin();

  WiFi.begin(wifi_ssid, wifi_password);

  Serial.println("");
  Serial.println("Created!");
  Serial.print("Local IP Address: ");

  Serial.println(WiFi.softAPIP());
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_SUCCESS, HIGH);
    _delay(500, []() {
      listenForMessage();
      server.handleClient();
    });
    Serial.print(".");
    digitalWrite(LED_SUCCESS, LOW);
  }

  Serial.println("");
  Serial.println("Connected!");
  Serial.print("Public IP Address: ");
  Serial.println(WiFi.localIP());


  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setBufferSize(512);
  while (!client.connected()) {
    reconnect([]() {
      listenForMessage();
      server.handleClient();
    });
  }

  // Xabarni yaratish
  String message = "{\"id\":\"" + String(user_mqtt) + "\", \"update_id\":" + String(update_id) + ", \"device_id\":\"" + String(mqtt_client_id) + "\"}";


  // Xabarni publish qilish
  if (client.publish(publish_topic, message.c_str())) {
    Serial.print(publish_topic);
    Serial.println(" ga  xabar yuborildi! " + message + " end");
  } else {
    Serial.print(publish_topic);
    Serial.println(" ga  xabar yuborishda xatolik!");
  }
  client.loop();
}


// unsigned long: int millis() ~24.8 kundan keyin to'lib ketardi va hisoblagich to'xtardi
unsigned long old_time_checker = 0;
void loop() {
  listenForMessage();
  server.handleClient();
  if (!client.connected()) {
    reconnect([]() {
      listenForMessage();
      server.handleClient();
    });
  }
  client.loop();

  if (check_ESP_restart_counter == 30) {
    ESP.restart();  // Restart the ESP device after the counter reaches 10
  }
  if (check_ESP_restart_counter_flag && old_time_checker + 1000 < millis()) {
    old_time_checker = millis();  // Update the last time checked
    check_ESP_restart_counter++;  // Increment the counter every second
  }
}
