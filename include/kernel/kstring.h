#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
int strncmp(const char *left, const char *right, size_t count);

size_t strlen(const char *str);
void reverse(char str[], int length);
int get_num_length(uint64_t num);
void uint64_to_string(uint64_t num, char *str);
void uint64_to_hex_string(uint64_t num, char *str);
void uint64_to_binary_string(uint64_t num, char *buf);
void uint64_to_hex_string_padded(uint64_t num, char *str);