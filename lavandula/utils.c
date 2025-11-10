#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/utils.h"

FileBuffer readFileBuffer(const char *path) {
    FileBuffer buffer = {0};

    FILE *fptr = fopen(path, "rb");
    if (!fptr) {
        printf("unable to open file: %s\n", path);
        return buffer;
    }

    if (fseek(fptr, 0, SEEK_END) != 0) {
        fclose(fptr);
        return buffer;
    }

    long sz = ftell(fptr);
    if (sz < 0) {
        fclose(fptr);
        return buffer;
    }

    rewind(fptr);

    buffer.data = malloc((size_t)sz + 1);
    if (!buffer.data) {
        fclose(fptr);
        return buffer;
    }

    size_t read = fread(buffer.data, 1, (size_t)sz, fptr);
    if (read != (size_t)sz) {
        fclose(fptr);
        free(buffer.data);
        buffer.data = NULL;
        buffer.length = 0;
        return buffer;
    }

    buffer.data[sz] = '\0';
    buffer.length = (size_t)sz;

    fclose(fptr);
    return buffer;
}

char *readFile(const char *path) {
    FileBuffer buffer = readFileBuffer(path);
    return buffer.data;
}
