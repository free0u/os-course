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

int find_char(char* s, int len, char c) {
    int i = 0;
    while (i < len && s[i] != 0) {
        if (s[i] == c) return i;
        ++i;
    }
    return -1;
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
    int count;
    while (1) {
        // invariant: buffer contains "len" start chars of new (and maybe next) strings

        int ind_endl = find_char(buffer, len, '\n');
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
        } else // buffer don't contain endl => we should skip tail
        {
        }

        count = read(0, buffer + len, k - len);
        len += count;
    }

    free(buffer);
    return 0;
}
