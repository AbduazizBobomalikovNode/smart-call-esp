#ifndef UTILS_H
#define UTILS_H


#include <ArduinoJson.h>

typedef std::function<void()> callback_fun_t;

bool _delay(int millisekunt, callback_fun_t callback_fun);

DynamicJsonDocument deserializeWithMemoryIncrease(const char* json_str);
bool validateFields(DynamicJsonDocument& json);
bool validateJson(const char* input);

void callParm(int call_delay, int count);
void check_calls();
bool includesDay(const char str[], const char substr[]);

String get_json();
bool saveToFile(String str_json);

bool checkJsonById(int id) ;
bool deleteJsonById(int id);
bool deleteJsonByUdateId(int id);
bool updateJsonById(int id, const String& newJson);
bool checkTypeById(int id);

String removeNewlines(String jsonStr);

#endif // UTILS_H
