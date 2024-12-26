#ifndef VARIABLES_H
#define VARIABLES_H
#define MQTT_MAX_PACKET_SIZE


#include <WiFi.h>
#include <ESP32WebServer.h>

#include <PubSubClient.h>

#include <RTClib.h>
#include <nRF24L01.h>
#include <RF24.h>

extern const int CE_PIN;
extern const int CSN_PIN;
extern const int LED_SUCCESS;
extern const int LED_FAIL;
extern const int CALL_PIN;
extern const byte address[6];  // Massiv sifatida deklaratsiya


extern RTC_DS1307 rtc;
extern RF24 radio;

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
extern const char* user_mqtt_device_id;
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
#endif  // VARIABLES_H
