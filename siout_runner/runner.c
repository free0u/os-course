#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <vector>
#include <stdlib.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>

std::vector<int> pids;

int run_command(char* from, char* to, char* argv[])
{
    int pid = fork();
    if (pid == 0)
    {
        int file_from = open(from, O_RDONLY, S_IREAD | S_IWRITE);
        int file_to = open(to, O_CREAT | O_WRONLY | O_TRUNC, S_IREAD | S_IWRITE);
        
        dup2(file_from, 0);
        close(file_from);
        dup2(file_to, 1);
        close(file_to);

        execvp(argv[0], argv);
        exit(255);
    }
    pids.push_back(pid);
    return 0;
//
//    int status = 0;
//    waitpid(pid, &status, 0);
//    
//    if (WIFEXITED(status)) {
//        if (WEXITSTATUS(status)) {
//            return 1;
//        }
//    }
//    return 0;
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

int my_strstr(char* in, int len, int from, int cnt_zero)
{
    for (int i = from; i < len; i++)
    {
        if (in[i] == '\0')
        {
            if (i == len - 1)
            {
                if (cnt_zero == 1)
                    return i;
                else
                    return -1;
            } else
            {
                if (cnt_zero == 1 && in[i + 1] != '\0')
                    return i;
                if (cnt_zero == 2 && in[i + 1] == '\0')
                    return i;
            }
        }
    }
    return -1;
}

void parse_command(char* cmd, int len)
{
    std::vector<my_string> fields;

    int ind_st = 0;
    while (true)
    {
        int p = my_strstr(cmd, len, ind_st, 1);
        if (p == -1)
        {
            fields.push_back(my_string(cmd + ind_st, len - ind_st));
            break;
        }

        fields.push_back(my_string(cmd + ind_st, p - ind_st));
        ind_st = p + 1;
    }

    len = fields.size();
    my_string from = fields[0];
    my_string to = fields[len - 1];

    char** argv = (char**)malloc((len - 1) * sizeof(char*));
    for (int i = 1; i < len - 1; i++)
    {
        argv[i - 1] = fields[i].s;
    }

    run_command(from.s, to.s, argv);
    free(argv);
}

int main(int argc, char* argv[])
{
    char* name_of_input = argv[1];

    int k = 4096;
    char* buffer = (char*)malloc(k + 1);

    int file_input = open(name_of_input, O_RDONLY, S_IREAD | S_IWRITE);

    int count, len, eof_found;
    len = 0;
    eof_found = 0;
    while (1)
    {
        count = read(file_input, buffer + len, k - len);
        if (k - len > 0 && count == 0)
        {
            eof_found = 1;
        }
        len += count;

        int ind = my_strstr(buffer, len, 0, 2);
        if (ind != -1) // delim1 found
        {
            parse_command(buffer, ind);

            memmove(buffer, buffer + ind + 2, len - ind - 2);
            len = len - ind - 2;
        } else
        {
            if (eof_found)
            {
                if (len > 0)
                { 
                    parse_command(buffer, len);
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

    int status;
    for (int i = 0; i < pids.size(); i++)
    {
        waitpid(pids[i], &status, 0);
    }
    close(file_input);
    free(buffer);
    return 0;
}
