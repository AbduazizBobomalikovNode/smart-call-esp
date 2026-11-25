// secrets.example.h  --  NAMUNA FAYL (commit qilinadi, haqiqiy qiymatlar YO'Q)
//
// Foydalanish:
//   1) Bu faylni shu papkada "secrets.h" nomi bilan nusxalang.
//   2) Qiymatlarni o'zingiznikiga almashtiring.
//   3) "secrets.h" .gitignore da -- uni hech qachon GitHub'ga yuklamang!
//
// Eslatma: bular faqat BOSHLANG'ICH qiymatlar. Qurilma ishlaganda
// /settings.txt (SPIFFS) va MQTT orqali kelgan sozlamalar ularni almashtiradi.
#ifndef SECRETS_H
#define SECRETS_H

// --- MQTT broker (port variables.cpp da, 1883) ---
#define SECRET_MQTT_SERVER     "mqtt.example.uz"   // broker manzili
#define SECRET_MQTT_USER       "YOUR_MQTT_USER"    // broker login
#define SECRET_MQTT_PASSWORD   "CHANGE_ME"         // broker parol / token
#define SECRET_MQTT_CLIENT_ID  "YOUR_DEVICE_ID"    // qurilma identifikatori
#define SECRET_USER_MQTT       "YOUR_USER_ID"      // foydalanuvchi ID (topic ichida ishlatiladi)
#define SECRET_USER_MQTT_DEVICE_ID "0"             // qurilma raqami (JSON'ga raqam sifatida yoziladi!)

// --- Uy WiFi (qurilma ulanadigan tarmoq) ---
#define SECRET_WIFI_SSID       "YOUR_WIFI_SSID"
#define SECRET_WIFI_PASSWORD   "CHANGE_ME"

// --- Qurilmaning o'z hotspoti (AP) ---
#define SECRET_HOTSPOT_SSID    "YOUR_DEVICE_NAME"  // hotspot nomi (qurilma nomi)
#define SECRET_HOTSPOT_PASSWORD "CHANGE_ME"        // kamida 8 belgi (WPA2)

// --- Qurilma veb-serveri (HTTP Basic) ---
#define SECRET_DEVICE_LOGIN    "admin"
#define SECRET_DEVICE_PIN      "0000"              // albatta o'zgartiring!

// --- nRF24 radio juftlash manzili (5 belgi; pult va qurilmada bir xil bo'lsin) ---
#define SECRET_RADIO_ADDRESS   "00001"

#endif  // SECRETS_H
