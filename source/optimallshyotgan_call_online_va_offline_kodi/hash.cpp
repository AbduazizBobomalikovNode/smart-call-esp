#include "mbedtls/sha256.h"
#include <string.h>
#include <stdio.h>

void sha256_hash(const char* input, const char* salt, unsigned char output[32]) {
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);

    // Salt va inputni birlashtirish
    mbedtls_sha256_update(&ctx, (const unsigned char*)salt, strlen(salt));
    mbedtls_sha256_update(&ctx, (const unsigned char*)input, strlen(input));

    mbedtls_sha256_finish(&ctx, output);
    mbedtls_sha256_free(&ctx);
}

bool compare_passwords(const char* input_password, const char* salt, const unsigned char stored_hash[32]) {
    unsigned char calculated_hash[32];
    sha256_hash(input_password, salt, calculated_hash);

    // Hashlarni solishtirish
    for (int i = 0; i < 32; i++) {
        if (calculated_hash[i] != stored_hash[i]) {
            return false;  // Hashlar mos emas
        }
    }
    return true;  // Hashlar mos
}

void print_hash(unsigned char hash[32]) {
    for (int i = 0; i < 32; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

// void setup() {
//     Serial.begin(115200);

//     const char* password = "***REMOVED***";
//     const char* salt = "my_random_salt";
//     unsigned char stored_hash[32];

//     // Parolni hash qilish va saqlash (odatda bu saqlanadi)
//     sha256_hash(password, salt, stored_hash);

//     Serial.print("Saqlangan hashed parol: ");
//     print_hash(stored_hash);

//     // Foydalanuvchi tomonidan kiritilgan parolni tekshirish
//     const char* input_password = "***REMOVED***";  // Foydalanuvchi kiritgan parol
//     bool is_match = compare_passwords(input_password, salt, stored_hash);

//     if (is_match) {
//         Serial.println("Parol mos keldi!");
//     } else {
//         Serial.println("Parol mos kelmadi!");
//     }
// }


