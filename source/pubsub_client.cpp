#include <Arduino.h>
#include "HardwareSerial.h"
#include "pubsub_client.h"
#include "variables.h"
#include "utils.h"


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Yangi ma'lumot kirib keldi [");
  Serial.print(topic);
  Serial.print("] ");
  String data;
  for (int i = 0; i < length; i++) {
    data += (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();
  // Serial.println(String(topic) + "   " + subscribe_topic);

  if (String(topic) == subscribe_topic) {
    DynamicJsonDocument json = deserializeWithMemoryIncrease(data.c_str());
    if (json.isNull()) {
      Serial.println("JSON deserializatsiya xatosi!");
      return;
    }
    int update_id_check = json["update_id"].as<int>();
    String method = json["method"].as<String>();
    update_id = update_id_check;
    saveSettings();
    if (method == "add") {
      String json_data = json["data"].as<String>();
      DynamicJsonDocument data = deserializeWithMemoryIncrease(json_data.c_str());

      if (!validateFields(data) && saveToFile(json_data)) {
        Serial.println("Qurilmaga yangi ma'lumot qo'shildi!");
      } else {
        Serial.println("malumot   saqlashda qandaydir xatolik mavjut");
      }
    } else if (method == "update") {
      // JsonObject  = json["data"].as<JsonObject>();
      String json_data = json["data"].as<String>();
      DynamicJsonDocument data = deserializeWithMemoryIncrease(json_data.c_str());
      if (!validateFields(data) && updateJsonById(data["id"].as<int>(), json_data)) {
        Serial.println(String(data["id"].as<int>()) + " - idli chalinish yangilandi!");
      } else {
        Serial.println("malumot   tahrirlashda qandaydir xatolik mavjut");
      }
    } else if (method == "delete") {
      JsonObject data = json["data"].as<JsonObject>();
      if (deleteJsonById(data["id"].as<int>())) {
        Serial.println(String(data["id"].as<int>()) + " - idli chalinish o'chirildi!");
        // Xabarni yaratish
        String message = "{\"delete_id\":\"" + String(user_mqtt) + "\", \"device_id\":\"" + String(mqtt_client_id) + "\"}";


        // Xabarni publish qilish
        if (client.publish(publish_topic, message.c_str())) {
          Serial.print(publish_topic);
          Serial.println(" ga  xabar yuborildi! " + message + " end");
        } else {
          Serial.print(publish_topic);
          Serial.println(" ga  xabar yuborishda xatolik!");
        }

      } else {
        Serial.println("malumot   o'chirishda qandaydir xatolik mavjut");
      }
    } else if (method == "check") {
      if (json["device"].as<String>() == mqtt_client_id) {
        
        String json_data = json["data"].as<String>();
        DynamicJsonDocument data = deserializeWithMemoryIncrease(json_data.c_str());
        if (checkJsonById(data["id"].as<int>())) {
          if (!validateFields(data) && updateJsonById(data["id"].as<int>(), json_data)) {
            Serial.println(String(data["id"].as<int>()) + " - idli chalinish yangilandi!");
          } else {
            Serial.println("malumot   tahrirlashda qandaydir xatolik mavjut");
          }
        } else {
          if (!validateFields(data) && saveToFile(json_data)) {
            Serial.println("Qurilmaga yangi ma'lumot qo'shildi!");
          } else {
            Serial.println("malumot   saqlashda qandaydir xatolik mavjut");
          }
        }
      }
    }else {
      Serial.println("Noma'lum usul!");
    }
  }
  if (String(topic) == seting_topic) {
    DynamicJsonDocument json = deserializeWithMemoryIncrease(data.c_str());
    if (json.isNull()) {
      Serial.println("JSON deserializatsiya xatosi!");
      return;
    }

    String type = json["type"].as<String>();
    if (type == "user") {
      String json_data = json["data"].as<String>();
      DynamicJsonDocument userData = deserializeWithMemoryIncrease(json_data.c_str());

      if (!userData.isNull()) {
        if (userData.containsKey("device_login") && userData["device_login"].as<String>().length() > 0) {
          device_login = strdup(userData["device_login"].as<String>().c_str());
        } else {
          Serial.println("Login ma'lumoti bo'sh!");
        }

        if (userData.containsKey("hostpot_password") && userData["hostpot_password"].as<String>().length() > 0) {
          hostpot_password = strdup(userData["hostpot_password"].as<String>().c_str());
        } else {
          Serial.println("Hotspot paroli bo'sh!");
        }
        saveSettings();
      } else {
        Serial.println("User ma'lumotlar noto'g'ri yoki bo'sh!");
      }
    }

    if (type == "device" && json["device"].as<String>() == mqtt_client_id) {
      String json_data = json["data"].as<String>();
      DynamicJsonDocument deviceData = deserializeWithMemoryIncrease(json_data.c_str());

      if (!deviceData.isNull()) {
        if (deviceData.containsKey("name") && deviceData["name"].as<String>().length() > 0) {
          hostpot_and_device_username = strdup(deviceData["name"].as<String>().c_str());
        } else {
          Serial.println("Qurilma nomi bo'sh!");
        }

        if (deviceData.containsKey("pin") && deviceData["pin"].as<String>().length() > 0) {
          device_pin = strdup(deviceData["pin"].as<String>().c_str());
        } else {
          Serial.println("Qurilma PIN kod bo'sh!");
        }

        if (deviceData.containsKey("wifi_name") && deviceData["wifi_name"].as<String>().length() > 0) {
          wifi_ssid = strdup(deviceData["wifi_name"].as<String>().c_str());
        } else {
          Serial.println("WiFi nomi bo'sh!");
        }

        if (deviceData.containsKey("wifi_password") && deviceData["wifi_password"].as<String>().length() > 0) {
          wifi_password = strdup(deviceData["wifi_password"].as<String>().c_str());
        } else {
          Serial.println("WiFi paroli bo'sh!");
        }
        saveSettings();
      } else {
        Serial.println("Device ma'lumotlar noto'g'ri yoki bo'sh!");
      }
    }
  }
}

void reconnect(callback_fun_t callback_fun) {
  while (!client.connected()) {
    Serial.print("MQTT-ga ulanish...");
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_password)) {
      Serial.println("MQTT-ga muvaffaqiyatli ulanildi!");
      client.subscribe(subscribe_topic);
      client.subscribe(seting_topic);
      deleteJsonByUdateId(1);
    } else {
      Serial.print("MQTT-ga ulanishda xatolik: ");
      Serial.println(client.state());
      Serial.println("15 sekunddan keyin qayta urinib ko'ramiz");
      _delay(15000, callback_fun);
      // delay(15000);
    }
  }
}
