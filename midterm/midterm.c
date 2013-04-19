#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <vector>
#include <stdlib.h>

char* buffer;
char* delim1;
char* delim2;

void reverse(char* line, int len)
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

    for (int i = inds.size() - 1; i >= 0; --i)
    {
        int next_end = (i == inds.size() - 1 ? len : inds[i + 1]);
        int local_len = next_end - inds[i] - strlen(delim2); 
        write(1, buffer + inds[i] + strlen(delim2), local_len);
        write(1, delim2, strlen(delim2));
    }
    write(1, buffer, inds.empty() ? len : inds[0]);
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
            //write(1, buffer, pos); 
            char backup = buffer[pos];
            buffer[pos] = '\0';
            reverse(buffer, pos);
            buffer[pos] = backup;

            write(1, delim1, strlen(delim1));

            memmove(buffer, buffer + pos + strlen(delim1), len - strlen(delim1) - pos);
            len = len - strlen(delim1) - pos;
            buffer[len] = '\0';
        } else
        {
            if (eof_found)
            {
                if (len > 0)
                { 
                    //write(1, buffer, len);
                    buffer[len] = '\0';
                    reverse(buffer, len);
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
