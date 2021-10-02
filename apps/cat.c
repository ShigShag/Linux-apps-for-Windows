#include <stdio.h>
#include <stdlib.h>

void hexdump_file(char* path)
{
    FILE* fptr;
    unsigned long f_size;
    unsigned long bytes_read = 1;
    unsigned char buffer[1024];

    fopen_s(&fptr, path, "rb");
    if (fptr == NULL) return;

    fseek(fptr, 0, SEEK_END);
    f_size = ftell(fptr);
    rewind(fptr);

    while((bytes_read = fread(buffer, sizeof(unsigned char), sizeof(buffer), fptr)) > 0)
    { 
        fwrite(buffer, sizeof(unsigned char), bytes_read, stdout);
    }
    
    fclose(fptr);
}

int main(int argc, char *argv[])
{
    if(argc > 1)
    {
        unsigned char *buffer;
        int size;

        for(int i = 1;i < argc;i++) 
        {
            hexdump_file(argv[1]);
        }
    }
    return 0;
}