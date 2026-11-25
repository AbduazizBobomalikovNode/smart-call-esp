#include "WString.h"
#include "esp32-hal.h"
#include "esp32-hal-gpio.h"
#include <string>
#include "HardwareSerial.h"
#include "utils.h"
#include <Arduino.h>
#include <SPIFFS.h>
#include <FS.h>
#include "variables.h"




typedef std::function<void()> callback_fun_t;

bool _delay(int millisekunt, callback_fun_t callback_fun) {
  // unsigned long: millis() 49 kundan keyin aylanib o'tganda ham to'g'ri ishlaydi
  unsigned long oldingi_vaqt = millis();
  while ((unsigned long)millisekunt > (millis() - oldingi_vaqt)) {
    if (callback_fun != nullptr) {
      callback_fun();  // Callback funksiyasini chaqirish
    }
    yield();  // watchdog / WiFi stek uchun vaqt berish
  }
  return false;
}

// delay() o'rniga: kutish paytida MQTT keepalive (client.loop) ishlab turadi,
// aks holda uzoq qo'ng'iroq (soniyalab) paytida broker ulanishni uzib qo'yadi.
static void keepAliveDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    if (client.connected()) client.loop();
    delay(5);
  }
}

//;
void listenForTime() {
  // Joriy vaqtni olish va chiqarish
  DateTime now = rtc.now();
  __day = now.dayOfTheWeek();
  __hour = now.hour();
  __minute = now.minute();

  // Yagona qatorda joriy vaqtni chiqarish
  // Serial.print("T Kun: ");
  // Serial.print(__day);
  // Serial.print(", Soat: ");
  // Serial.print(__hour);
  // Serial.print(", Daqiqa: ");
  // Serial.println(__minute);
}


bool includesDay(const char str[], const char substr[]) {
  // Serial.println(str);
  // Serial.println(substr);
  // Null tekshiruvi va bo'sh qatorlarni tekshirish
  if (str == NULL || substr == NULL) {
    Serial.println("Error: Null pointer argument passed to includesDay.");
    return false;
  }

  // Bo'sh qatorlarni tekshirish
  if (str[0] == '\0' || substr[0] == '\0') {
    Serial.println("Error: Empty string argument passed to includesDay.");
    return false;
  }

  // strstr funksiyasi orqali substringni qidirish
  return strstr(str, substr) != NULL;
}


void callParm(int call_delay, int call_duration, int count) {
  digitalWrite(LED_FAIL, HIGH);
  Serial.print("Call delay: ");
  Serial.print(call_delay);
  Serial.print(", Count: ");
  Serial.println(count);
  // `count > 0`: avval `!= 0` edi -- manfiy count (noto'g'ri JSON) cheksiz qo'ng'iroq qilardi
  while (count > 0) {
    count--;
    digitalWrite(CALL_PIN, HIGH);
    keepAliveDelay(call_duration);
    digitalWrite(CALL_PIN, LOW);
    keepAliveDelay(call_delay);
  }
  digitalWrite(LED_FAIL, LOW);
}

// Initialize variables and constants
int old__day, old__hour, old__minute;
void check_calls() {
  // Vaqtni yangilashni sinab ko‘ramiz
  listenForTime();

  // Eski vaqt bilan hozirgi vaqtni solishtiramiz
  if (old__day != __day || old__hour != __hour || old__minute != __minute) {
    // Vaqtni darhol eslab qolamiz: avval faqat mos kelganda yangilanardi, shuning
    // uchun mos kelmasa /smart.txt har loop() aylanishida qayta o'qilardi.
    old__day = __day;
    old__hour = __hour;
    old__minute = __minute;
    // Faylni ochishga harakat qilamiz
    File file = SPIFFS.open("/smart.txt", "r");
    if (!file) {
      Serial.println("Faylni o‘qish uchun ochib bo‘lmadi!");
      return;
    }

    // Fayldan har bir qatorni o‘qish
    while (file.available()) {
      String line = file.readStringUntil('\n');
      // Serial.print("String line = file.readStringUntil('n') check_calls;  :");
      // Serial.println(line);

      // JSON deserializatsiya qilishni sinash
      DynamicJsonDocument json(1024);  // JSON uchun hujjat hajmini belgilash
      DeserializationError error = deserializeJson(json, line.c_str());

      // Deserializatsiya xatosini tekshirish
      if (error) {
        Serial.print("JSON deserializatsiya xatoligi: ");
        Serial.println(error.c_str());
        continue;  // Keyingi qatorga o‘tish
      }
      String days_ = "";
      if (json.containsKey("days") && json["days"].is<JsonArray>()) {
        // Massivni olish
        JsonArray daysArray = json["days"].as<JsonArray>();

        // Massivdagi har bir elementni chiqarish
        for (JsonVariant day : daysArray) {
          // Serial.println(day.as<String>());
          days_ = days_ + ", " + day.as<String>();
        }
      }


      // JSON tarkibini tekshirish
      if (json.containsKey("days") && json["days"].is<JsonArray>()) {
        // Serial.println(json["days"].as<String>());
        // JSON tarkibidan vaqtni ajratish va soat va minutni bo'lish
        String timeString = json["time"].as<String>();

        // Soat va minutni bo'lish
        int separatorIndex = timeString.indexOf(':');  // ':' belgining indeksini topish
        String hour = "";                              // Soatni ajratish
        String minute = "";
        bool ch1 = false;
        bool ch2 = false;
        bool ch3 = false;
        if (separatorIndex != -1) {
          String hour = timeString.substring(0, separatorIndex);     // Soatni ajratish
          String minute = timeString.substring(separatorIndex + 1);  // Minutni ajratish

          // Natijani chiqarish
          // Serial.print("K Soat: ");
          // Serial.print(hour);
          // Serial.print(" Minut: ");
          // Serial.print(minute);
          ch1 = includesDay(days_.c_str(), String(__day).c_str());
          // Serial.print(" includesDayTrue: ");
          // Serial.print(ch1);
          ch2 = hour.toInt() == __hour;
          // Serial.print(" hour.toInt() == __hour: ");
          // Serial.println(ch2);
          ch3 = minute.toInt() == __minute;
          // Serial.print(" minute.toInt() == __minute: ");
          // Serial.println(ch3);
        } else {
          Serial.println("Vaqtni ajratishda xatolik: Format noto'g'ri.");
        }
        // isactive=false bo'lgan qo'ng'iroq chalinmasligi kerak (avval e'tiborga olinmagan)
        bool active = json["isactive"] | true;
        bool flag = ch1 && ch2 && ch3 && active;
        // Serial.print(" flag: ");
        // Serial.println(flag);
        if (flag) {
          Serial.println("Qo'ng'iroq parametrlarini sozlash...");
          // Qo‘ng‘iroqni sozlash
          callParm(json["call_delay"].as<int>() * 1000,
                   json["call_duration"].as<int>() * 1000,
                   json["count"].as<int>());

          // Eski vaqtni yangilash
          old__day = __day;
          old__hour = __hour;
          old__minute = __minute;

          // Faylni yopamiz
          file.close();
          return;  // Qo'ng'iroq topildi, funksiyani yakunlaymiz
        }
      } else {
        Serial.println("JSONda 'days' yoki boshqa kerakli qiymatlar yo'q!");
      }
    }

    // Faylni yopamiz
    file.close();
  }
}






// Xabarlarni eshitish funksiyasi
void listenForMessage() {
  if (radio.available()) {                                  // If a message is available
    // 33 bayt: radio 32 bayt yozadi, oxirgi bayt doim '\0' (avval terminatorsiz edi -> strcmp/println buffer'dan chiqib ketardi)
    char receivedMessage[33] = "";
    radio.read(&receivedMessage, 32);  // Read the message (payload = 32 bayt)

    // Print the received message
    Serial.print("Qabul qilindi: ");
    Serial.println(receivedMessage);

    // Use strcmp to compare the received message with "HA"
    if (strcmp(receivedMessage, "HA") == 0) {
      callParm(2000, 5000, 2);
    }
  } else {
    check_calls();
  }
}


DynamicJsonDocument deserializeWithMemoryIncrease(const char* json_str) {
  size_t memorySize = 500;  // Initial memory size
  // Xotira kam bo'lsa (<5000) unsigned ayirish aylanib ketardi -- tekshiramiz
  const size_t freeHeap = ESP.getFreeHeap();
  const size_t maxMemorySize = freeHeap > 5000 ? freeHeap - 5000 : 0;  // Maximum memory (free memory - buffer)
  // Mazmunni chop etmaymiz (sozlamalarda parollar bo'lishi mumkin) -- faqat uzunlik
  Serial.println("deserializeWithMemoryIncrease, len=" + String(json_str ? strlen(json_str) : 0));
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

  bool firstLine = true;  // Birinchi satrni tekshirish uchun flag
  while (file1.available()) {
    String line = file1.readStringUntil('\n');  // Yangi satrni o'qish
    line.trim();                                // Satrdan ortiqcha bo'sh joylarni olib tashlash

    if (line.length() > 0) {  // Bo'sh satrlarni o'tkazib yuborish
      if (!firstLine) {
        json_str += ",";  // Keyingi elementlarni vergul bilan ajratish
      }
      json_str += line;   // Satrdagi ma'lumotni JSONga qo'shish
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

    Serial.print("String line = file.readStringUntil('n');  deleteJsonById :");
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

    Serial.print("String line = file.readStringUntil('n');  deleteJsonByUdateId:");
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

    Serial.print("String line = file.readStringUntil('n');  deleteJsonByUdateId:");
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
        break;         // Loopsni to'xtatamiz, id topilganda boshqa qatorlarni o'qishga hojat yo'q
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
        break;         // Loopsni to'xtatamiz, id topilganda boshqa qatorlarni o'qishga hojat yo'q
      }
    }
  }

  file.close();

  return found;  // Natija qaytariladi
}
