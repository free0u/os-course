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

void write_string(char* s, int len) {
    write(1, s, len);
    char endl = '\n';
    write(1, &endl, 1);
}
        
int main(int argc, char* argv[]) {
    int k = parse_int(argv[1]) + 1;

    char* buffer = malloc(k);
    int result;

    int len = 0;
    int count;
    int eof_found = 0;
    while (1) {
        // invariant: buffer contains "len" start chars of new (and maybe next) strings

        // work with buffer[0..len - 1]
        int ind_endl = find_char(buffer, len, '\n');
        if (ind_endl != -1) { // buffer[0..len - 1] contain endl
            write_string(buffer, ind_endl);
            write_string(buffer, ind_endl);
            if (ind_endl < len - 1) { // copy tail to begin (without endl)
                memmove(buffer, buffer + ind_endl + 1, len - ind_endl - 1);
                len = len - ind_endl - 1;
            } else 
            {
                len = 0;
            }
        } else // buffer[0..len - 1] don't contain endl
        {
            if (eof_found && len > 0) {
                write_string(buffer, len);
                write_string(buffer, len);
                break;
            }
            if (k == len) {
                // skip tail
                len = 0;
                while (1) {
                    count = read(0, buffer + len, k - len);
                    if (k != len && count == 0) { // EOF
                        eof_found = 1;
                        break;
                    }
                    len += count;
                    ind_endl = find_char(buffer, len, '\n');
                    if (ind_endl != -1) {
                        memmove(buffer, buffer + ind_endl + 1, len - ind_endl - 1);
                        len = len - ind_endl - 1;
                        break;
                    }
                }
            }
        }

        if (eof_found) {
            break;
        }
        count = read(0, buffer + len, k - len);
        if (k != len && count == 0) { // EOF
            eof_found = 1;
        }
        len += count;
    }

    free(buffer);
    return 0;
}
