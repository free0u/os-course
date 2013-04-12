#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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


int get_status(char* argv[])
{
    int pid = fork();
    if (pid == 0)
    {
        execvp(argv[0], argv);
        printf("oops\n");
    }

    int status = 0;
    waitpid(pid, &status, 0);

    return WEXITSTATUS(status);
}

int main(int argc, char* argv[]) {
    int k = 4 * 1024;
    char delim = '\n';

    int res_opt;
    while ((res_opt = getopt(argc, argv, "nzb::")) != -1)
    {
        switch (res_opt) {
            case 'n': 
                break; 
            case 'z': 
                delim = '\0';
                break;
            case 'b': 
                if (optarg) {
                    k = atoi(optarg);
                }
                break;
        };
    }
    int len_cmd = argc - optind + 1;

    char ** cmds;
    cmds = malloc(len_cmd * sizeof(char*));
    int i = 0;
    for (i = 0; i < len_cmd; ++i)
    {
        cmds[i] = argv[i + optind];
    } 
    cmds[len_cmd - 1] = NULL;
    printf("%d\n", get_status(cmds));
    return 0;
    

    char* buffer = malloc(k + 1);
    int result;

    int len = 0;
    int count;
    int eof_found = 0;
    while (1) {
        // invariant: buffer contains "len" start chars of new (and maybe next) strings

        // work with buffer[0..len - 1]
        int ind_endl = find_char(buffer, len, delim);
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
                    ind_endl = find_char(buffer, len, delim);
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
    free(cmds); 

    return 0;
}
