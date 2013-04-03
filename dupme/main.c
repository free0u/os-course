#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int _strlen(char* s) {
    int i = 0;
    while (s[i] != 0) {
        ++i;
    }
    return i;
}

int parse_int(char* s) {
    int result = 0;
    int i = 0;
    while (s[i] != 0) {
        result = result * 10 + (s[i] - '0');
        ++i;
    }
    return result;
}

int main(int argc, char* argv[]) {
    int k = parse_int(argv[1]);

    char* buffer = malloc(k);
    int result;

    while (1) {
        result = read(0, buffer, 1);
        printf("%d\n", result);

        if (result == 0) {
            break;
        }
    }

    free(buffer);
    return 0;
}
