#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    while (i < to && s[i] != 0)
    {
        if (s[i] == c) return i;
        ++i;
    }
    return -1;
}

void write_twice(char* s, int len)
{
    char endl = '\n';
    int i;
    for (i = 0; i < 2; ++i)
    {
        int pos = 0;
        while (pos < len)
        {
            pos += check("write", write(1, s + pos, len - pos));
        }
        write(1, &endl, 1);
    }
}
        
enum state {READ, SKIP};

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        char* message = "Usage: echo input | ./main <buffer size>\n";
        write(1, message, strlen(message));
        exit(1);
    }

    int k = atoi(argv[1]) + 1;

    char* buffer = check_malloc("malloc error\n", k);
    if (buffer == NULL)
    {
        char* message = "malloc error\n";
        write(1, message, strlen(message));
        exit(1);
    }

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
            int pos = find_char(buffer, 0, len, '\n'); 
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
            int pos_r = find_char(buffer, pos_l, len, '\n');
            while (pos_r != -1)
            {
                write_twice(buffer + pos_l, pos_r - pos_l);
                pos_l = pos_r + 1;
                pos_r = find_char(buffer, pos_l, len, '\n');
            }
    
            memmove(buffer, buffer + pos_l, len - pos_l);
            len -= pos_l;
    
            if (len == k) 
            {
                st = SKIP;
                len = 0;
            }
    
            if (eof_found && len > 0)
                write_twice(buffer, len);
        }
    }

    free(buffer);
    return 0;
}
