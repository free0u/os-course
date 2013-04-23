#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <vector>
#include <stdlib.h>
#include <wait.h>

char* buffer;
char* delim1;
char* delim2;

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

struct my_string
{
    char* s;
    int len;
    my_string(my_string const& o)
    {
        len = o.len;
        s = (char*)malloc(len + 1);
        memmove(s, o.s, len + 1);
    }
    my_string(char* buf, int _len)
    {
        len = _len;
        s = (char*)malloc(len + 1);
        memmove(s, buf, len);
        s[len] = '\0';
    }
    ~my_string()
    {
        if (s)
        {
            free(s);
        }
    }
};

void reverse(char* line, int len, int argc, char* argv[])
{
    std::vector<int> inds;
    int pos = 0;
    char* ind;
    while (pos < len)
    {
       ind = strstr(line + pos, delim2);
       if (ind)
       {
           pos = ind - line + strlen(delim2);
           inds.push_back(ind - line);
       } else
       {
           break;
       }
    }

    std::vector<my_string> fields;

    fields.push_back(my_string(buffer, inds.empty() ? len : inds[0]));
    for (int i = 0; i < inds.size(); i++)
    {
        int next_end = (i == inds.size() - 1 ? len : inds[i + 1]);
        int local_len = next_end - inds[i] - strlen(delim2);
        fields.push_back(my_string(buffer + inds[i] + strlen(delim2), local_len) );
    }

    char ** cmds;
    int len_cmd = argc - 3;
    len_cmd += fields.size();
    
    cmds = (char**)malloc((len_cmd + 1) * sizeof(char*) ); 
    cmds[len_cmd] = NULL;
    for (int i = 3; i < argc; i++)
    {
        cmds[i - 3] = argv[i];
    }

    int st = argc - 3;
    for (int i = 0; i < fields.size(); i++)
    {
        cmds[st + i] = fields[i].s;
    }
    int status = get_status(cmds);

    if (status)
    {
        write(1, buffer, inds.empty() ? len : inds[0]);
        for (int i = 0; i < inds.size(); i++)
        {
            int next_end = (i == inds.size() - 1 ? len : inds[i + 1]);
            int local_len = next_end - inds[i] - strlen(delim2);
            write(1, delim2, strlen(delim2));
            write(1, buffer + inds[i] + strlen(delim2), local_len);
        }

    } else
    {
        for (int i = inds.size() - 1; i >= 0; --i)
        {
            int next_end = (i == inds.size() - 1 ? len : inds[i + 1]);
            int local_len = next_end - inds[i] - strlen(delim2); 
            write(1, buffer + inds[i] + strlen(delim2), local_len);
            write(1, delim2, strlen(delim2) );
        }
        write(1, buffer, inds.empty() ? len : inds[0]);
   }

    free(cmds);
}

int main(int argc, char* argv[])
{
    delim1 = (char*)argv[1];
    delim2 = (char*)argv[2];

    int k = 4096;
    buffer = (char*)malloc(k + 1);

    int count, len, eof_found;
    len = 0;
    eof_found = 0;
    while (1)
    {
        count = read(0, buffer + len, k - len);
        if (k - len > 0 && count == 0)
        {
            eof_found = 1;
        }
        len += count;

        char* ind = strstr(buffer, delim1);
        if (ind) // delim1 found
        {
            int pos = ind - buffer;
            char backup = buffer[pos];
            buffer[pos] = '\0';
            reverse(buffer, pos, argc, argv);
            buffer[pos] = backup;

            write(1, delim1, strlen(delim1) );

            memmove(buffer, buffer + pos + strlen(delim1), len - strlen(delim1) - pos);
            len = len - strlen(delim1) - pos;
            buffer[len] = '\0';
        } else
        {
            if (eof_found)
            {
                if (len > 0)
                { 
                    buffer[len] = '\0';
                    reverse(buffer, len, argc, argv);
                }
                break;
            }
            if (k == len)
            {
                // crash
                return 1;
            }
        }
    }

    free(buffer);
    return 0;
}
