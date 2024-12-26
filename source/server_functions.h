#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

// Qurilmaga mos kutubxonalar
#if defined(ESP32)
  #include <ESP32WebServer.h>
#elif defined(ESP8266)
  #include <ESP8266WebServer.h>
#else
  #error "Ushbu kod faqat ESP32 yoki ESP8266 uchun mo'ljallangan!"
#endif

void handleRoot();
void handleHello();
void handle_get_all_GET();
void handle_add_POST();
void handle_update_POST();
void handle_delete_POST();
bool chaekAuth();


#endif // SERVER_FUNCTIONS_H
