#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096

char* file1 = "/tmp/out1.txt";
char* file2 = "/tmp/out2.txt";

char *buffer;

int check(const char * comment, int what)
{
    if (what < 0) {
        perror(comment);
        _exit(1);
    }
    return what;
}

void write_to_descr(char * buf, int len, int file)
{
    int pos = 0;
    while (len > 0)
    {
        int count = check("write failed", write(file, buf + pos, len));
        pos += count;
        len -= count;
    }
}

void run_cmd(char* argv[])
{
    int fds[2];
    pipe(fds);
    int pid = fork();
    if (pid == 0) // child
    {
        dup2(fds[1], 1);
        close(fds[0]);
        close(fds[1]);
               
        execvp(argv[0], argv);
        _exit(255);
    }

    int count, len;
    len = 0;
    close(fds[1]);
    while (1)
    {
        count = check("read failed", read(fds[0], buffer + len, BUFFER_SIZE - len));
        if (BUFFER_SIZE - len == 0) // buffer is full
            break;
        if (count == 0) // eof
            break;
        len += count;
    }
    write_to_descr(buffer, len, 1);

    int p = open(file1, O_CREAT | O_WRONLY | O_TRUNC, S_IREAD | S_IWRITE);
    write_to_descr(buffer, len, p);

    int status;
    waitpid(pid, &status, 0);
}

int main(int argc, char* argv[]) 
{
    if (argc < 2) {
        char* message = "Usage: watchthis <time> <command> <command arg 0> ...\n";
        write(1, message, strlen(message));
        return 0;
    }

    buffer = malloc(BUFFER_SIZE);

    int interval = atoi(argv[1]);

    char ** cmds;
    cmds = (char**)malloc(sizeof(char*) * (argc - 1));

    int i = 0;
    for (i = 2; i < argc; ++i)
    {
        cmds[i - 2] = argv[i];
    }
    cmds[argc - 2] = NULL;

    while (1)
    {
        int pid = fork();
        if (!pid)
        {
            execlp("clear", "clear", NULL);
        }
        waitpid(pid, NULL, 0);

        run_cmd(cmds);
        
        write(1, "\nDiff:\n\n", 8);
        pid = fork();
        if (!pid)
        {
            execlp("diff", "diff", "-u", file1, file2, NULL);
        }
        waitpid(pid, NULL, 0);

        char* tmp = file1;
        file1 = file2;
        file2 = tmp;
        
        sleep(interval);
    }

    free(cmds);
    free(buffer);
    return 0;
}

