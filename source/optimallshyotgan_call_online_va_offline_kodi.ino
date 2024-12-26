// Qurilmaga mos kutubxonalar
#if defined(ESP32)
  #include <WiFi.h>
  #include <ESP32WebServer.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
#else
  #error "Ushbu kod faqat ESP32 yoki ESP8266 uchun mo'ljallangan!"
#endif


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
  Serial.print("Password: ");
  Serial.println(hostpot_password);

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
    _delay(500, []() {
      server.handleClient();
    });
    Serial.print(".");
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


int old_time_checker = 0;
void loop() {
  server.handleClient();
  if (!client.connected()) {
    reconnect([]() {
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
