#ifndef BUFFERED_READER_H
#define BUFFERED_READER_H 1

#include <stdio.h>
#include "default_definitions.h"

#define DEFAULT_STRING_BUFFER_SIZE 1024

typedef struct {
    char* buffer_data;
    uint buffer_size;
    uint avaible_buffer_space;
} StringBuffer;

bool string_buffer_read_line(FILE*, StringBuffer*);
void string_buffer_init(StringBuffer*);

#endif
