#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 4096

int check(const char * message, int what)
{
    if (what < 0) 
    {
        perror(message);
        _exit(1);
    }
    return what;
}

void* check_malloc(const char * message, int k)
{
    void* p = malloc(k);
    if (p == NULL)
    {
        write(1, message, strlen(message));
        exit(1);
    }
    return p;
}

int find_char(char* s, int from, int to, char c)
{
    int i = from;
    while (i < to)
    {
        if (s[i] == c) return i;
        ++i;
    }
    return -1;
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

void write_string(char* s, int len)
{
    int pos = 0;
    while (pos < len)
    {
        pos += check("write", write(1, s + pos, len - pos));
    }
    char endl = '\n';
    write(1, &endl, 1);
}
        
enum state {READ, SKIP};

int main(int argc, char* argv[]) {
    int k = BUFFER_SIZE;
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
    cmds = check_malloc("malloc error\n", (len_cmd + 1) * sizeof(char*));
    cmds[len_cmd] = NULL;

    int i = 0;
    for (i = 0; i < len_cmd; ++i)
    {
        cmds[i] = argv[i + optind];
    } 

    char* buffer = check_malloc("malloc error\n", k + 1);

    int len = 0;
    int eof_found = 0;
    enum state st = READ;
    while (!eof_found)
    {
        int cnt = check("read", read(0, buffer + len, k - len));
        if (cnt == 0)
           eof_found = 1;
        len += cnt;

        if (st == SKIP)
        {
            int pos = find_char(buffer, 0, len, delim); 
            if (pos == -1)
            {
                len = 0;
                continue;
            }
            ++pos;
            memmove(buffer, buffer + pos, len - pos);
            len -= pos;
            st = READ;
        }

        if (st == READ) {
            int pos_l = 0;
            int pos_r = find_char(buffer, pos_l, len, delim);
            while (pos_r != -1)
            {
                char save = buffer[pos_r - pos_l];
                buffer[pos_r - pos_l] = 0;

                cmds[len_cmd - 1] = buffer + pos_l;
                if (!get_status(cmds))
                {
                    write_string(buffer + pos_l, pos_r - pos_l);
                }
                buffer[pos_r - pos_l] = save;

                pos_l = pos_r + 1;
                pos_r = find_char(buffer, pos_l, len, delim);
            }
    
            memmove(buffer, buffer + pos_l, len - pos_l);
            len -= pos_l;
    
            if (len == k) 
            {
                st = SKIP;
                len = 0;
            }
    
            if (eof_found && len > 0)
            {
                buffer[len] = 0;
                cmds[len_cmd - 1] = buffer;
                if (!get_status(cmds))
                {
                    write_string(buffer, len);
                }
            }
        }
    }
    
    free(buffer);
    free(cmds); 

    return 0;
}
