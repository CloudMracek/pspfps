#include <stdio.h>
#include "filesystem.h"

long getFileSize(const char* filename) {
    FILE* file = fopen(filename, "rb");  // Open the file in binary mode

    if (file == NULL) {
        perror("Error opening file");
        return -1;  // Error opening file
    }

    fseek(file, 0, SEEK_END);  // Move the file pointer to the end of the file
    long size = ftell(file);   // Get the position of the file pointer, which is the file size
    fclose(file);              // Close the file

    return size;
}