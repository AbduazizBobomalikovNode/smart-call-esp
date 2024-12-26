#include <string>
#include "HardwareSerial.h"
#include "utils.h"
#include <Arduino.h>
#include <SPIFFS.h>
#include <FS.h>
#include "variables.h"



typedef std::function<void()> callback_fun_t;

bool _delay(int millisekunt, callback_fun_t callback_fun) {
  int oldingi_vaqt = millis();
  while ((millisekunt > (millis() - oldingi_vaqt))) {
    if (callback_fun != nullptr) {
      callback_fun();  // Callback funksiyasini chaqirish
    }
  }
  return false;
}


DynamicJsonDocument deserializeWithMemoryIncrease(const char* json_str) {
  size_t memorySize = 500;                                // Initial memory size
  const size_t maxMemorySize = ESP.getFreeHeap() - 5000;  // Maximum memory (free memory - buffer)
  Serial.println("deserializeWithMemoryIncrease :" + String(json_str));
  while (true) {
    // Create a document with current memory size
    DynamicJsonDocument jsonDoc(memorySize);

    // Try deserializing
    DeserializationError error = deserializeJson(jsonDoc, json_str);

    // If deserialization was successful
    if (!error) {
      Serial.println("Deserialization successful!");
      return jsonDoc;  // Return the JSON document
    }

    // If memory error and haven't reached max memory yet
    if (error == DeserializationError::NoMemory && memorySize < maxMemorySize) {
      memorySize += 500;  // Increase memory size
      Serial.print("Memory shortage! New memory size: ");
      Serial.println(memorySize);
    } else {
      // If other errors or reached max memory
      Serial.println("Deserialization failed or reached max memory limit!");
      return DynamicJsonDocument(0);  // Return an empty document
    }
  }
}

bool validateJson(const char* input) {
  return !deserializeWithMemoryIncrease(input).isNull();
}


bool validateFields(DynamicJsonDocument& json) {

  if (!json.containsKey("id") || !json["id"].is<int>()) return false;
  if (!json.containsKey("name") || !json["name"].is<String>() || json["name"].as<String>().length() < 3 || json["name"].as<String>().length() > 50) return false;
  if (!json.containsKey("time") || !json["time"].is<String>()) return false;
  if (!json.containsKey("days") || !json["days"].is<JsonArray>() || json["days"].size() < 1 || json["days"].size() > 7) return false;
  if (!json.containsKey("turn_off_after_work") || !json["turn_off_after_work"].is<bool>()) return false;
  if (!json.containsKey("call_duration") || !json["call_duration"].is<int>()) return false;
  if (!json.containsKey("call_delay") || !json["call_delay"].is<int>()) return false;
  if (!json.containsKey("count") || !json["count"].is<int>()) return false;
  if (!json.containsKey("isactive") || !json["isactive"].is<bool>()) return false;

  return true;
}


bool includesDay(const char str[], const char substr[]) {
  return strstr(str, substr) != NULL;
}

void callParm(int call_delay, int count) {
  Serial.print("Call delay: ");
  Serial.print(call_delay);
  Serial.print(", Count: ");
  Serial.println(count);
}


void check_calls() {
  File file = SPIFFS.open("/smart.txt", "r");
  if (!file) {
    Serial.println("Faylni o‘qish uchun ochib bo‘lmadi!");
    return;
  }

 

  while (file.available()) {
    String line = file.readStringUntil('\n');
    Serial.print("String line = file.readStringUntil('\n');  :");
    Serial.print(line);

    // JSON deserializatsiya qilish
    DynamicJsonDocument json = deserializeWithMemoryIncrease(line.c_str());
    if (!json.isNull()) {
      if (includesDay(json["days"], String(__day).c_str()) && json["hour"].as<int>() == __hour && json["minute"].as<int>() == __minute) {
        callParm(json["call_delay"].as<int>(), json["count"].as<int>());
        file.close();  // Topilgandan keyin faylni yopamiz
        return;
      }
    } else {
      Serial.println("JSON deserializatsiya xatoligi: " + line);
    }
  }
  file.close();
}


bool saveToFile(String str_json) {
  String str_json_ = removeNewlines(str_json);
  Serial.println("bool saveToFile(String str_json) : " + str_json);
  // JSON-ni deserializatsiya qilish
  DynamicJsonDocument json = deserializeWithMemoryIncrease(str_json.c_str());
  if (json.isNull()) {
    Serial.println("JSON-ni deserializatsiya qilishda xato.");
    return false;
  }

  // Faylni "append" rejimida ochish
  File file = SPIFFS.open("/smart.txt", "a");
  if (!file) {
    Serial.println("Faylni ochib bo'lmadi!");
    return false;
  }

  // JSON-ni faylga yozish
  file.println(str_json_);
  file.close();

  Serial.println("Malumotlar smart.txt fayliga muvaffaqiyatli yozildi.");
  return true;
}



String get_json() {
  File file1 = SPIFFS.open("/smart.txt", "r");
  if (!file1) {
    Serial.println("Faylni o‘qish uchun ochib bo‘lmadi!");
    return "";
  }
  Serial.println("smart.txt fayli o'qish uchun ochildi!");
  
  String json_str = "[";  // JSON massivining boshlanishi
  
  bool firstLine = true; // Birinchi satrni tekshirish uchun flag
  while (file1.available()) {
    String line = file1.readStringUntil('\n');  // Yangi satrni o'qish
    line.trim();  // Satrdan ortiqcha bo'sh joylarni olib tashlash
    
    if (line.length() > 0) {  // Bo'sh satrlarni o'tkazib yuborish
      if (!firstLine) {
        json_str += ",";  // Keyingi elementlarni vergul bilan ajratish
      }
      json_str += line;  // Satrdagi ma'lumotni JSONga qo'shish
      firstLine = false;  // Endi birinchi satr emas
    }
  }
  
  file1.close();
  
  json_str += "]";  // JSON massivining tugashi
  // Serial.println(json_str);
  
  return json_str;
}






bool deleteJsonById(int id) {
  File file = SPIFFS.open("/smart.txt", "r");
  if (!file) {
    Serial.println("Faylni o‘qish uchun ochib bo‘lmadi!");
    return false;
  }

  File tempFile = SPIFFS.open("/temp.txt", "w");
  if (!tempFile) {
    Serial.println("Vaqtinchalik faylni yaratish mumkin emas!");
    file.close();
    return false;
  }

  bool found = false;
  

  while (file.available()) {
    String line = file.readStringUntil('\n');

    Serial.print("String line = file.readStringUntil('\n');  :");
    Serial.println(line);
    
    DynamicJsonDocument json = deserializeWithMemoryIncrease(line.c_str());
    if (!json.isNull()) {
      if (json["id"].as<int>() != id) {
        tempFile.println(line);
      } else {
        found = true;  // Mark as found
      }
    }
  }

  file.close();
  tempFile.close();

  if (found) {
    SPIFFS.remove("/smart.txt");
    SPIFFS.rename("/temp.txt", "/smart.txt");
    return true;
  } else {
    SPIFFS.remove("/temp.txt");  // Remove the temp file if not found
    return false;
  }
}

bool deleteJsonByUdateId(int id) {
  File file = SPIFFS.open("/smart.txt", "r");
  if (!file) {
    Serial.println("Faylni o‘qish uchun ochib bo‘lmadi!");
    return false;
  }

  File tempFile = SPIFFS.open("/temp.txt", "w");
  if (!tempFile) {
    Serial.println("Vaqtinchalik faylni yaratish mumkin emas!");
    file.close();
    return false;
  }

  bool found = false;
  

  while (file.available()) {
    String line = file.readStringUntil('\n');

    Serial.print("String line = file.readStringUntil('\n');  :");
    Serial.println(line);
    
    DynamicJsonDocument json = deserializeWithMemoryIncrease(line.c_str());
    if (!json.isNull()) {
      if (json["type"].as<String>() != "ofline") {
        tempFile.println(line);
      } else {
        found = true;  // Mark as found
      }
    }
  }

  file.close();
  tempFile.close();

  if (found) {
    SPIFFS.remove("/smart.txt");
    SPIFFS.rename("/temp.txt", "/smart.txt");
    return true;
  } else {
    SPIFFS.remove("/temp.txt");  // Remove the temp file if not found
    return false;
  }
}





bool updateJsonById(int id, const String& newJson) {
  String newJson_ = removeNewlines(newJson);
  File file = SPIFFS.open("/smart.txt", "r");
  if (!file) {
    Serial.println("Faylni o‘qish uchun ochib bo‘lmadi!");
    return false;
  }

  File tempFile = SPIFFS.open("/temp.txt", "w");
  if (!tempFile) {
    Serial.println("Vaqtinchalik faylni yaratish mumkin emas!");
    file.close();
    return false;
  }

  bool updated = false;
  

  while (file.available()) {
    String line = file.readStringUntil('\n');

    Serial.print("String line = file.readStringUntil('\n');  :");
    Serial.println(line);
    

    DynamicJsonDocument json = deserializeWithMemoryIncrease(line.c_str());
    if (!json.isNull()) {
      if (json["id"].as<int>() == id) {
        tempFile.println(newJson_);
        updated = true;  // Mark as updated
      } else {
        tempFile.println(line);
      }
    }
  }

  file.close();
  tempFile.close();

  if (updated) {
    SPIFFS.remove("/smart.txt");
    SPIFFS.rename("/temp.txt", "/smart.txt");
    return true;
  } else {
    SPIFFS.remove("/temp.txt");  // Remove the temp file if not updated
    return false;
  }
}

String removeNewlines(String jsonStr) {
  String result;
  for (unsigned int i = 0; i < jsonStr.length(); i++) {
    if (jsonStr[i] != '\n') {  // Skip newline characters
      result += jsonStr[i];
    }
  }
  Serial.print("String removeNewlines(String jsonStr)");
  Serial.println(result);
  return result;
}

bool checkJsonById(int id) {
  File file = SPIFFS.open("/smart.txt", "r");
  if (!file) {
    Serial.println("Faylni o‘qish uchun ochib bo‘lmadi!");
    return false;  // Fayl ochilmasa, false qaytariladi
  }

  bool found = false;

  while (file.available()) {
    String line = file.readStringUntil('\n');
    
    DynamicJsonDocument json = deserializeWithMemoryIncrease(line.c_str());
    if (!json.isNull()) {
      if (json["id"].as<int>() == id) {
        found = true;  // Agar id topilsa, found true bo'ladi
        break;  // Loopsni to'xtatamiz, id topilganda boshqa qatorlarni o'qishga hojat yo'q
      }
    }
  }

  file.close();

  return found;  // Natija qaytariladi
}

bool checkTypeById(int id) {
  File file = SPIFFS.open("/smart.txt", "r");
  if (!file) {
    Serial.println("Faylni o‘qish uchun ochib bo‘lmadi!");
    return false;  // Fayl ochilmasa, false qaytariladi
  }

  bool found = false;

  while (file.available()) {
    String line = file.readStringUntil('\n');
    
    DynamicJsonDocument json = deserializeWithMemoryIncrease(line.c_str());
    if (!json.isNull()) {
      if (json["id"].as<int>() == id && json["type"].as<String>() == "ofline") {
        found = true;  // Agar id topilsa, found true bo'ladi
        break;  // Loopsni to'xtatamiz, id topilganda boshqa qatorlarni o'qishga hojat yo'q
      }
    }
  }
  
  file.close();

  return found;  // Natija qaytariladi
}
