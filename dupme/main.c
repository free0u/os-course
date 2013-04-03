#include <string.h>
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
    int k = parse_int(argv[1]) + 1;

    char* buffer = malloc(k);
    int result;

    /*
    while (1) {
        int cc = read(0, buffer, k);
        printf("%d\n", cc);
    }
    return 0;
    */
    int len = 0;
    while (1) {
        // invariant: buffer contains "len" chars of new (and maybe next) strings
        if (len == k) { // buffer is full
            int ind_endl = -1;
            int i;
            for (i = 0; i < len; ++i) {
                if (buffer[i] == '\n') {
                    ind_endl = i;
                    break;
                }
            }
            if (ind_endl != -1) { // buffer contain endl
                write(1, buffer, ind_endl); printf("\n");
                write(1, buffer, ind_endl); printf("\n");
                if (ind_endl < len - 1) { // copy tail to begin (without endl)
                    memmove(buffer, buffer + ind_endl + 1, len - ind_endl - 1);
                    len = len - ind_endl - 1;
                } else 
                {
                    len = 0;
                }
            } else 
            {
            }
        }
        int count = read(0, buffer + len, k - len);
        printf("%d\n", count);
        len += count;
    }

    free(buffer);
    return 0;
}
