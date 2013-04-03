#include <stdio.h>

int _strlen(char* s) {
    int i = 0;
    while (s[i] != 0) {
        ++i;
    }
    return i;
}

int main(int argc, char* argv[]) {
    printf("%d args\n", argc);
    int i;
    for (i = 0; i < argc; ++i) {
       // printf("%s\n", argv[i]);
        printf("%d\n", _strlen(argv[i]));
    }
    return 0;
}
