#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define a custom string structure
typedef struct {
    char *data;
    size_t length;
} String;

// Initialize a string
String* string_init(const char *source) {
    String *str = malloc(sizeof(String));
    if (!str) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    str->length = strlen(source);
    str->data = malloc((str->length + 1) * sizeof(char)); // +1 for null terminator
    if (!str->data) {
        perror("Memory allocation failed");
        free(str);
        exit(EXIT_FAILURE);
    }

    strcpy(str->data, source);

    return str;
}

// Free memory used by a string
void string_free(String *str) {
    free(str->data);
    free(str);
}

// Compare two strings
int string_compare(const String *str1, const String *str2) {
    return strcmp(str1->data, str2->data);
}