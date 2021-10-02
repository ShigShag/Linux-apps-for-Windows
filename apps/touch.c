#include <windows.h>

int main(int argc, char *argv[])
{
    if(argc > 1)
    {
        for(int i = 1;i < argc;i++)
        {
            CreateFileA(argv[i], GENERIC_ALL, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        }
    }
    return 0;
}