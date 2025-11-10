#ifndef utils_h
#define utils_h

typedef struct {
    char  *data;
    size_t length;
} FileBuffer;

FileBuffer readFileBuffer(const char *filename);
char *readFile(const char *filename);

#endif
