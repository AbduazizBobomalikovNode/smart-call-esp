#ifndef HASH_H
#define HASH_H

void sha256_hash(const char* input, const char* salt, unsigned char output[32]);
bool compare_passwords(const char* input_password, const char* salt, const unsigned char stored_hash[32]);
void print_hash(unsigned char hash[32]);

#endif
