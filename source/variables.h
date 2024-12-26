#ifndef VARIABLES_H
#define VARIABLES_H
#define MQTT_MAX_PACKET_SIZE

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

#include <PubSubClient.h>

extern int update_id;

// Variables and constants
extern int __day, __hour, __minute;
extern ESP32WebServer server;
extern const char* mqtt_server;
extern const int mqtt_port;
extern const char* mqtt_user;
extern const char* mqtt_password;
extern const char* mqtt_client_id;
extern const char* user_mqtt;
extern const char* subscribe_topic;
extern const char* seting_topic;
extern const char* publish_topic;
extern WiFiClient espClient;
extern PubSubClient client;


extern const char* hostpot_and_device_username;
extern const char* hostpot_password;
extern const char* device_login;
extern const char* device_pin;
extern const char* wifi_ssid;
extern const char* wifi_password;


extern int check_ESP_restart_counter;
extern bool check_ESP_restart_counter_flag;



bool loadSettings();
bool saveSettings();
void printSettings();
#endif // VARIABLES_H
