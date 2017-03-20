#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string_buffer.h"
#include "default_definitions.h"

bool string_buffer_read_line(FILE* input_file, StringBuffer* buf) {
    bool result = (!feof(input_file) && fgets(buf->buffer_data, buf->avaible_buffer_space, input_file));

    if(!result) {
        return result;
    }

    uint string_len = strlen(buf->buffer_data);

    bool end = 0;
    while(!end && buf->buffer_data[string_len - 1] != '\n') {
        buf->buffer_size *= 2;
        uint avaible_buffer_space = buf->buffer_size - string_len;
        buf->buffer_data = realloc(buf->buffer_data, buf->buffer_size);

        end = !(!feof(input_file) && fgets(buf->buffer_data + string_len, avaible_buffer_space, input_file));
        string_len = strlen(buf->buffer_data);
    }

    if(string_len && buf->buffer_data[string_len - 1] == '\n') {
        buf->buffer_data[string_len - 1] = '\0';
    }

    return result;
}

void string_buffer_init(StringBuffer* buf) {
    buf->buffer_data = (char*) malloc(DEFAULT_STRING_BUFFER_SIZE * sizeof(char));
    buf->avaible_buffer_space = DEFAULT_STRING_BUFFER_SIZE;
    buf->buffer_size = 0;
}
