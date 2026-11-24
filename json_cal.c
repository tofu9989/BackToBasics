#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"

static const char *calculate(const cJSON *cal, double *result)
{
    if (!cJSON_IsObject(cal)) {
        return "cal must be a JSON object";
    }

    const char *op = cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(cal, "op"));
    double a = cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(cal, "a"));
    double b = cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(cal, "b"));

    if (op == NULL || strlen(op) != 1 || !isfinite(a) || !isfinite(b)) {
        return "cal requires an operator and two finite numbers (a and b)";
    }

    switch (op[0]) {
    case '+':
        *result = a + b;
        break;
    case '-':
        *result = a - b;
        break;
    case '*':
        *result = a * b;
        break;
    case '/':
        if (b == 0) {
            return "cannot divide by zero";
        }
        *result = a / b;
        break;
    default:
        return "supported operators are +, -, *, /";
    }

    return isfinite(*result) ? NULL : "result is out of range";
}

int main(int argc, char **argv)
{
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [data.json]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *path = argc == 2 ? argv[1] : "data.json";
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        perror(path);
        return EXIT_FAILURE;
    }

    char input[16 * 1024 + 1];
    size_t size = fread(input, 1, sizeof input, file);
    int read_failed = ferror(file);
    fclose(file);
    if (read_failed || size == sizeof input || memchr(input, '\0', size)) {
        fprintf(stderr, "Expected a text file of at most 16 KiB: %s\n", path);
        return EXIT_FAILURE;
    }

    input[size] = '\0';
    cJSON *root = cJSON_ParseWithLengthOpts(input, size + 1, NULL, 1);
    if (!cJSON_IsObject(root)) {
        fprintf(stderr, "Expected a JSON object in %s\n", path);
        cJSON_Delete(root);
        return EXIT_FAILURE;
    }

    double result;
    const char *error = calculate(cJSON_GetObjectItemCaseSensitive(root, "cal"), &result);
    char *output = NULL;
    if (error == NULL) {
        cJSON_DeleteItemFromObjectCaseSensitive(root, "result");
        if (cJSON_AddNumberToObject(root, "result", result) != NULL) {
            output = cJSON_Print(root);
        }
        if (output == NULL) {
            error = "could not create output JSON";
        }
    }

    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
    } else {
        puts(output);
        cJSON_free(output);
    }
    cJSON_Delete(root);
    return error == NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}
