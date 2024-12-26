#include "HardwareSerial.h"
#include "server_functions.h"
#include "pubsub_client.h"
#include "variables.h"
#include "variables.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <FS.h>
#include "utils.h"

void handleRoot() {
  if (chaekAuth()) { return; }
  server.send(200, "application/json", "{\"result\":\"chalindi!\"}");
}

void handleHello() {
  if (chaekAuth()) { return; }
  server.send(201, "application/json", "{\"result\":\"Avtorizatsiya mofaqiyatli amalga oshirildi!\"}");
}

void handle_get_all_GET() {
  if (chaekAuth()) { return; }
  String response = get_json();
  server.send(200, "application/json", response);
}


void printAllHeaders() {
  int headerCount = server.headers();  // Sarlavhalar sonini olish
  Serial.println("Barcha HTTP sarlavhalari:");

  for (int i = 0; i < headerCount; i++) {
    // Sarlavhaning nomini va qiymatini olish
    String headerName = server.headerName(i);
    String headerValue = server.header(i);

    // Chop etish
    Serial.print(headerName + ": ");
    Serial.println(headerValue);
  }

  if (headerCount == 0) {
    Serial.println("Hech qanday sarlavha mavjud emas.");
  }
}



bool chaekAuth() {
  printAllHeaders();
  if (!server.authenticate(device_login, device_pin)) {
    server.send(401, "application/json", "{\"result\":\"Avtorizatsiya qilinmagan!\"}");
    return true;
  }
  return false;
}



void handle_add_POST() {
  if (chaekAuth()) { return; }

  String body = server.arg("plain");

  // Check for excessive body length
  if (body.length() > 500000) {
    server.send(500, "application/json", "{\"result\":\"Xotira qayta ishlashga yetmadi!\"}");
    return;
  }

  // Check available heap memory for SRAM
  if (ESP.getFreeHeap() < 10000) {  // 10KB SRAM
    server.send(500, "application/json", "{\"result\":\"SRAM xotira yetmadi!\"}");
    return;
  }

  // Check available space in Flash for writing
  uint64_t totalBytes = SPIFFS.totalBytes();
  uint64_t usedBytes = SPIFFS.usedBytes();
  if (totalBytes - usedBytes < body.length()) {
    server.send(500, "application/json", "{\"result\":\"Saqlash uchun joy yetmaydi!\"}");
    return;
  }

  // Validate JSON format
  if (!validateJson(body.c_str())) {
    server.send(400, "application/json", "{\"result\":\"Noto‘g‘ri JSON formati!\"}");
    return;
  }

  // Deserialize JSON and validate fields using deserializeWithMemoryIncrease
  DynamicJsonDocument json = deserializeWithMemoryIncrease(body.c_str());
  // Serial.println("!validateFields(json) = ");
  // Serial.println(!validateFields(json));
  if (json.isNull() || !validateFields(json)) {
    server.send(400, "application/json", "{\"result\":\"Noto‘g‘ri yoki yetishmayotgan maydonlar!\"}");
    return;
  }

  if (checkJsonById(json["id"].as<int>())) {
    server.send(400, "application/json", "{\"message\":\"ushbu ID li ma'lumot oldindan mavjut\"}");
    return;
  }
  if (client.connected()) {
    // Xabarni yaratish
    String message = "{\"id\":\"" + String(user_mqtt) + "\", \"add_ring\":" + body + ", \"device_id\":\"" + String(mqtt_client_id) + "\"}";

    // Xabarni publish qilish
    if (client.publish(publish_topic, message.c_str())) {
      Serial.print(publish_topic);
      Serial.println(" ga  xabar yuborildi! " + message + " end");
    } else {
      Serial.print(publish_topic);
      Serial.println(" ga  xabar yuborishda xatolik!");
    }
  }

  // Try saving data to file
  if (saveToFile(body)) {
    server.send(200, "application/json", "{\"result\":\"Muvaffaqiyatli saqlandi!\"}");
  } else {
    server.send(500, "application/json", "{\"result\":\"Malumotlarni saqlashda xatolik!\"}");
  }
}


void handle_update_POST() {
  if (chaekAuth()) { return; }

  String path = server.uri();  // Get the full request URI
  Serial.println("update path: " + path);

  // Check if the path starts with "/update/"
  if (path.startsWith("/update/")) {

    String body = server.arg("plain");

    if (body.length() > 500000) {
      server.send(500, "application/json", "{\"result\":\"Xotira qayta ishlashga yetmadi!\"}");
      return;
    }

    // Check available heap memory for SRAM
    if (ESP.getFreeHeap() < 10000) {  // 10KB SRAM
      server.send(500, "application/json", "{\"result\":\"SRAM xotira yetmadi!\"}");
      return;
    }

    // Check available space in Flash for writing
    uint64_t totalBytes = SPIFFS.totalBytes();
    uint64_t usedBytes = SPIFFS.usedBytes();
    if (totalBytes - usedBytes < body.length()) {
      server.send(500, "application/json", "{\"result\":\"Saqlash uchun joy yetmaydi!\"}");
      return;
    }

    if (!validateJson(body.c_str())) {
      server.send(400, "application/json", "{\"result\":\"Noto‘g‘ri JSON formati!\"}");
      return;
    }

    DynamicJsonDocument json = deserializeWithMemoryIncrease(body.c_str());
    Serial.println("!validateFields(json) = ");
    Serial.println(!validateFields(json));
    if (json.isNull() || !validateFields(json)) {
      server.send(400, "application/json", "{\"result\":\"Noto‘g‘ri yoki yetishmayotgan maydonlar!\"}");
      return;
    }

    // Extract the 'id' after "/update/"
    String id = path.substring(8);  // Get the substring after "/update/"

    Serial.println("update ID: " + id);  // Debug print the extracted id

    if (!checkJsonById(id.toInt())) {
      server.send(400, "application/json", "{\"message\":\"ushbu ID li ma'lumot mavjut emas\"}");
      return;
    }

    if (client.connected()) {
      // Xabarni yaratish
      String message = "{\"id\":\"" + String(user_mqtt) + "\", \"update_ring\":" + body + ", \"device_id\":\"" + String(mqtt_client_id) + "\"}";

      // Xabarni publish qilish
      if (client.publish(publish_topic, message.c_str())) {
        Serial.print(publish_topic);
        Serial.println(" ga  xabar yuborildi! " + message + " end");
      } else {
        Serial.print(publish_topic);
        Serial.println(" ga  xabar yuborishda xatolik!");
      }
    }

    if (updateJsonById(id.toInt(), body)) {
      Serial.println(id + " - idli chalinish yangilandi!");
    }
    // Respond to the client
    server.send(200, "application/json", "{\"message\":\"Updated item with id " + id + "\"}");
  } else {
    // If no valid id is provided, return a bad request response
    server.send(400, "application/json", "{\"message\":\"Bad Request - ID not provided\"}");
  }
}


void handle_delete_POST() {
  if (chaekAuth()) { return; }

  String path = server.uri();  // Get the full request URI
  Serial.println("delete path: " + path);
  if (path.startsWith("/delete/")) {
    String id = path.substring(8);  // Get the substring after "/update/"

    // Now 'id' contains the value '123' in this case
    Serial.println("delete ID: " + id);

    if (!checkJsonById(id.toInt())) {
      server.send(400, "application/json", "{\"message\":\"ushbu ID li ma'lumot mavjut emas\"}");
      return;
    }
    bool ofline = checkTypeById(id.toInt());
    if (deleteJsonById(id.toInt())) {
      Serial.println(id + " - idli chalinish o'chirildi!");
    }
    if (ofline) {
      server.send(200, "application/json", "{\"type\":\"ofline\",\"xabar\":\"ID " + id + " bilan element o'chirildi\"}");
    } else {
      server.send(200, "application/json", "{\"type\":\"online\",\"xabar\":\"ID " + id + " bilan element o'chirildi\"}");
    }
    // Mijozga javob qaytarish
  } else {
    server.send(400, "application/json", "{\"xabar\":\"Xato so'rov - ID ko'rsatilmagan\"}");
  }
}
