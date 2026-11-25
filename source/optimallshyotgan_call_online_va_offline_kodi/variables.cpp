#include "variables.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "utils.h"

// Maxfiy boshlang'ich qiymatlar. secrets.example.h dan secrets.h nusxalang.
#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "secrets.h topilmadi: secrets.example.h ni secrets.h deb nusxalang va to'ldiring"
#endif



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
const byte address[6] = SECRET_RADIO_ADDRESS;  // secrets.h dan

RTC_DS1307 rtc;
RF24 radio(CE_PIN, CSN_PIN);

// Initialize variables and constants
int __day, __hour, __minute;

int update_id = 0;

// MQTT broker details
const char* mqtt_server = SECRET_MQTT_SERVER;  // secrets.h dan
const int mqtt_port = 1883;
const char* mqtt_user = SECRET_MQTT_USER;
const char* mqtt_password = SECRET_MQTT_PASSWORD;
const char* mqtt_client_id = SECRET_MQTT_CLIENT_ID;
const char* user_mqtt = SECRET_USER_MQTT;
const char* user_mqtt_device_id = SECRET_USER_MQTT_DEVICE_ID;
String subscribe_topic_str = String("/index/") + user_mqtt + "/ring";
const char* subscribe_topic = strdup(subscribe_topic_str.c_str());

String seting_topic_str = String("/index/") + user_mqtt + "/seting";
const char* seting_topic = strdup(seting_topic_str.c_str());

const char* publish_topic = "/get";

// Boshqa strdup qilingan char* xotira bloklarini ham ozod qilishni unutmang.


WiFiClient espClient;
PubSubClient client(espClient);


const char* wifi_ssid = SECRET_WIFI_SSID;
const char* wifi_password = SECRET_WIFI_PASSWORD;

//hostpot_ssid  sifatida
const char* hostpot_and_device_username = SECRET_HOTSPOT_SSID;  // qurilma nomi / hotspot SSID
//hostpot_password  sifatida
const char* hostpot_password = SECRET_HOTSPOT_PASSWORD;  // hotspot paroli


//auth login  sifatida
const char* device_login = SECRET_DEVICE_LOGIN;  // HTTP auth login
//auth password  sifatida
const char* device_pin = SECRET_DEVICE_PIN;  // HTTP auth paroli (PIN)

//espni  o'chirib yoqish uchun
int check_ESP_restart_counter = 0;
bool check_ESP_restart_counter_flag = false;




// Maxfiy qiymatlarni Serial'ga chiqarmaslik: faqat uzunligi ko'rsatiladi.
static void printMasked(const char* secret) {
  Serial.print("*** (len=");
  Serial.print(secret ? strlen(secret) : 0);
  Serial.println(")");
}

// strdup bilan olingan xotirani kuzatish: sozlama qayta o'qilganda/yangilanganda
// eski nusxa free() qilinadi (avval har safar oqib ketardi). Literal va boshlang'ich
// qiymatlar ro'yxatda yo'q -- ular hech qachon free() qilinmaydi.
static const char* owned_strings[16];

void setConfigStr(const char*& slot, const char* value) {
  if (value == nullptr) return;  // JSON'da kalit bor, lekin satr emas -- strdup(NULL) crash bo'lardi
  char* copy = strdup(value);
  if (copy == nullptr) return;   // xotira yetmadi -- eski qiymat qoladi
  for (auto& p : owned_strings) {
    if (p != nullptr && p == slot) {  // eski qiymat bizniki edi -- bo'shatamiz
      free((void*)p);
      p = copy;
      slot = copy;
      return;
    }
  }
  for (auto& p : owned_strings) {
    if (p == nullptr) { p = copy; break; }
  }
  slot = copy;
}

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
  printMasked(mqtt_password);

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
  printMasked(wifi_password);

  Serial.println("\n--- Hotspot and Device Settings ---");
  Serial.print("Username: ");
  Serial.println(hostpot_and_device_username);

  Serial.print("Hotspot Password: ");
  printMasked(hostpot_password);

  Serial.println("\n--- Authentication Settings ---");
  Serial.print("Device Login: ");
  Serial.println(device_login);

  Serial.print("Device PIN: ");
  printMasked(device_pin);

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

  // Fayl mazmunini chop etmaymiz: unda parollar bor.
  Serial.print("settings.txt o'qildi, bayt: ");
  Serial.println(json_string.length());

  DynamicJsonDocument doc = deserializeWithMemoryIncrease(json_string.c_str());

  // JSON hujjatidagi ma'lumotlarni o'zgaruvchilarga yuklash
  if (doc.containsKey("update_id")) {
    update_id = doc["update_id"].as<int>();
  }
  if (doc.containsKey("mqtt_server")) {
    setConfigStr(mqtt_server, doc["mqtt_server"].as<const char*>());
  }
  if (doc.containsKey("mqtt_user")) {
    setConfigStr(mqtt_user, doc["mqtt_user"].as<const char*>());
  }
  if (doc.containsKey("mqtt_password")) {
    setConfigStr(mqtt_password, doc["mqtt_password"].as<const char*>());
  }
  if (doc.containsKey("mqtt_client_id")) {
    setConfigStr(mqtt_client_id, doc["mqtt_client_id"].as<const char*>());
  }
  if (doc.containsKey("subscribe_topic")) {
    setConfigStr(subscribe_topic, doc["subscribe_topic"].as<const char*>());
  }
  if (doc.containsKey("seting_topic")) {
    setConfigStr(seting_topic, doc["seting_topic"].as<const char*>());
  }
  if (doc.containsKey("publish_topic")) {
    setConfigStr(publish_topic, doc["publish_topic"].as<const char*>());
  }
  if (doc.containsKey("hostpot_and_device_username")) {
    setConfigStr(hostpot_and_device_username, doc["hostpot_and_device_username"].as<const char*>());
  }
  if (doc.containsKey("hostpot_password")) {
    setConfigStr(hostpot_password, doc["hostpot_password"].as<const char*>());
  }
  if (doc.containsKey("device_login")) {
    setConfigStr(device_login, doc["device_login"].as<const char*>());
  }
  if (doc.containsKey("device_pin")) {
    setConfigStr(device_pin, doc["device_pin"].as<const char*>());
  }
  if (doc.containsKey("wifi_ssid")) {
    setConfigStr(wifi_ssid, doc["wifi_ssid"].as<const char*>());
  }
  if (doc.containsKey("wifi_password")) {
    setConfigStr(wifi_password, doc["wifi_password"].as<const char*>());
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

  // Hujjat to'lib ketgan bo'lsa, yarim sozlamani saqlab qo'ymaymiz
  if (doc.overflowed()) {
    Serial.println("Settings JSON hujjati to'lib ketdi, saqlanmadi.");
    return false;
  }

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
