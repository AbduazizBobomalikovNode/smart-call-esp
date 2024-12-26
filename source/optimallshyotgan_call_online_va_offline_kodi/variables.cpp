#include "variables.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "utils.h"



ESP32WebServer server(80);


#include <RTClib.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const int CE_PIN = 4;
const int CSN_PIN = 5;

const int LED_SUCCESS = 14;
const int LED_FAIL = 27;

const int CALL_PIN = 32;
const byte address[6] = "00005";

RTC_DS1307 rtc;
RF24 radio(CE_PIN, CSN_PIN);

// Initialize variables and constants
int __day, __hour, __minute;

int update_id = 0;

// MQTT broker details
const char* mqtt_server = "abdusoft.uz";  // e.g., "192.168.1.100"
const int mqtt_port = 1883;
const char* mqtt_user = "***REMOVED***";
const char* mqtt_password = "***REMOVED***";  // JWT token
const char* mqtt_client_id = "demo-NjMKHkO810gkfhh";
const char* user_mqtt = "55087719";
const char* user_mqtt_device_id = "99436911";
String subscribe_topic_str = String("/index/") + user_mqtt + "/ring";
const char* subscribe_topic = strdup(subscribe_topic_str.c_str());

String seting_topic_str = String("/index/") + user_mqtt + "/seting";
const char* seting_topic = strdup(seting_topic_str.c_str());

const char* publish_topic = "/get";

// Boshqa strdup qilingan char* xotira bloklarini ham ozod qilishni unutmang.


WiFiClient espClient;
PubSubClient client(espClient);


const char* wifi_ssid = "***REMOVED***";       //1
const char* wifi_password = "***REMOVED***";  //2

//hostpot_ssid  sifatida
const char* hostpot_and_device_username = "abdusoft_device";  // 3 qurilma nomi  device  wi
//hostpot_password  sifatida
const char* hostpot_password = "***REMOVED***";  // 6 password user


//auth login  sifatida
const char* device_login = "abdusoft";  //5 login user
//auth password  sifatida
const char* device_pin = "***REMOVED***";  //4 qurilma pin kodi device

//espni  o'chirib yoqish uchun
int check_ESP_restart_counter = 0;
bool check_ESP_restart_counter_flag = false;




void printSettings() {
  Serial.print("MALUMOTLAT  IDsi : ");
  Serial.println(update_id);
  Serial.println("====== Configuration Settings ======");

  Serial.println("\n--- MQTT Settings ---");
  Serial.print("Server: ");
  Serial.println(mqtt_server);

  Serial.print("Port: ");
  Serial.println(mqtt_port);

  Serial.print("User: ");
  Serial.println(mqtt_user);

  Serial.print("Password: ");
  Serial.println(mqtt_password);

  Serial.print("Client ID: ");
  Serial.println(mqtt_client_id);

  Serial.print("Subscribe Topic: ");
  Serial.println(subscribe_topic);

  Serial.print("Setting Topic: ");
  Serial.println(seting_topic);

  Serial.print("Publish Topic: ");
  Serial.println(publish_topic);

  Serial.println("\n--- WiFi Settings ---");
  Serial.print("SSID: ");
  Serial.println(wifi_ssid);

  Serial.print("Password: ");
  Serial.println(wifi_password);

  Serial.println("\n--- Hotspot and Device Settings ---");
  Serial.print("Username: ");
  Serial.println(hostpot_and_device_username);

  Serial.print("Hotspot Password: ");
  Serial.println(hostpot_password);

  Serial.println("\n--- Authentication Settings ---");
  Serial.print("Device Login: ");
  Serial.println(device_login);

  Serial.print("Device PIN: ");
  Serial.println(device_pin);

  Serial.println("====================================");
}

bool loadSettings() {
  // printSettings();
  // Fayl tizimini ishga tushirish
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount file system.");
    return false;
  }

  // Sozlamalar faylini o'qish uchun ochish
  File file = SPIFFS.open("/settings.txt", "r");
  if (!file) {
    Serial.println("Settings file does not exist, using default values.");
    return false;
  }

  String json_string = file.readString();

  Serial.println("read ");
  Serial.println(json_string);

  DynamicJsonDocument doc = deserializeWithMemoryIncrease(json_string.c_str());

  // JSON hujjatidagi ma'lumotlarni o'zgaruvchilarga yuklash
  if (doc.containsKey("update_id")) {
    update_id = doc["update_id"].as<int>();
  }
  if (doc.containsKey("mqtt_server")) {
    mqtt_server = strdup(doc["mqtt_server"].as<const char*>());
  }
  if (doc.containsKey("mqtt_user")) {
    mqtt_user = strdup(doc["mqtt_user"].as<const char*>());
  }
  if (doc.containsKey("mqtt_password")) {
    mqtt_password = strdup(doc["mqtt_password"].as<const char*>());
  }
  if (doc.containsKey("mqtt_client_id")) {
    mqtt_client_id = strdup(doc["mqtt_client_id"].as<const char*>());
  }
  if (doc.containsKey("subscribe_topic")) {
    subscribe_topic = strdup(doc["subscribe_topic"].as<const char*>());
  }
  if (doc.containsKey("seting_topic")) {
    seting_topic = strdup(doc["seting_topic"].as<const char*>());
  }
  if (doc.containsKey("publish_topic")) {
    publish_topic = strdup(doc["publish_topic"].as<const char*>());
  }
  if (doc.containsKey("hostpot_and_device_username")) {
    hostpot_and_device_username = strdup(doc["hostpot_and_device_username"].as<const char*>());
  }
  if (doc.containsKey("hostpot_password")) {
    hostpot_password = strdup(doc["hostpot_password"].as<const char*>());
  }
  if (doc.containsKey("device_login")) {
    device_login = strdup(doc["device_login"].as<const char*>());
  }
  if (doc.containsKey("device_pin")) {
    device_pin = strdup(doc["device_pin"].as<const char*>());
  }
  if (doc.containsKey("wifi_ssid")) {
    wifi_ssid = strdup(doc["wifi_ssid"].as<const char*>());
  }
  if (doc.containsKey("wifi_password")) {
    wifi_password = strdup(doc["wifi_password"].as<const char*>());
  }
  // Faylni yopish
  printSettings();
  file.close();
  return true;
}

bool saveSettings() {
  // printSettings();
  DynamicJsonDocument doc(1024);

  // JSON hujjatiga konfiguratsiya sozlamalarini qo'shish
  doc["update_id"] = update_id;
  doc["mqtt_server"] = mqtt_server;
  doc["mqtt_user"] = mqtt_user;
  doc["mqtt_password"] = mqtt_password;
  doc["mqtt_client_id"] = mqtt_client_id;
  doc["subscribe_topic"] = subscribe_topic;
  doc["seting_topic"] = seting_topic;
  doc["publish_topic"] = publish_topic;

  doc["hostpot_and_device_username"] = hostpot_and_device_username;
  doc["hostpot_password"] = hostpot_password;

  doc["device_login"] = device_login;
  doc["device_pin"] = device_pin;

  doc["wifi_ssid"] = wifi_ssid;
  doc["wifi_password"] = wifi_password;

  // Sozlamalar faylini yozish uchun ochish
  File file = SPIFFS.open("/settings.txt", "w");
  if (!file) {
    Serial.println("Failed to open settings file for writing.");
    return false;
  }

  // JSON hujjatini faylga yozish
  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write settings to file.");
    file.close();
    return false;
  }


  Serial.println("Settings saved.");
  // printSettings();
  // Faylni yopish
  file.close();
  check_ESP_restart_counter_flag = true;
  check_ESP_restart_counter = 0;
  return true;
}
