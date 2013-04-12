#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int find_char(char* s, int len, char c) {
    int i = 0;
    while (i < len) {
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
        exit(255);
    }

    int status = 0;
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status)) {
        if (WEXITSTATUS(status)) {
            return 1;
        }
    }
    return 0;
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
    cmds = malloc((len_cmd + 1) * sizeof(char*));
    cmds[len_cmd] = NULL;

    int i = 0;
    for (i = 0; i < len_cmd; ++i)
    {
        cmds[i] = argv[i + optind];
    } 

    k++;
    char* buffer = malloc(k);
    char* buffer2 = malloc(k);
    int result;

    int len = 0;
    int count;
    int eof_found = 0;
    while (1) {
        count = read(0, buffer + len, k - len);
        if (k - len > 0 && count == 0) { // EOF
            eof_found = 1;
        }
        len += count;
        // invariant: buffer contains "len" start chars of new (and maybe next) strings
        // work with buffer[0..len - 1]

        int ind_endl = find_char(buffer, len, delim);
        if (ind_endl != -1) { // buffer[0..len - 1] contain endl
            char char_save = buffer[ind_endl];
            buffer[ind_endl] = '\0';

            cmds[len_cmd - 1] = buffer;
            if (!get_status(cmds)) {
                write_string(buffer, ind_endl);
            }
            buffer[ind_endl] = char_save;
            
            if (ind_endl < len - 1) { // copy tail to begin (without endl)
                memmove(buffer, buffer + ind_endl + 1, len - ind_endl - 1);
                len = len - ind_endl - 1;
            } else 
            {
                len = 0;
            }
        } else // buffer[0..len - 1] don't contain endl
        {
            if (eof_found) {
                if (len > 0) {
                    char char_save = buffer[ind_endl];
                    buffer[ind_endl] = '\0';

                    cmds[len_cmd - 1] = buffer;
                    if (!get_status(cmds)) {
                        write_string(buffer, len);
                    }
                    buffer[ind_endl] = char_save;
                }
                break;
            }
            if (k == len) {
                // crash
                return 1;
                break;
            }
        }
    }

    free(buffer);
    free(buffer2);
    free(cmds); 

    return 0;
}
