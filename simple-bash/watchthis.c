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

int check(const char * message, int what)
{
    if (what < 0) {
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

void write_to_descr(char * buf, int len, int file)
{
    int pos = 0;
    while (pos < len)
    {
        pos += check("write failed", write(file, buf + pos, len - pos));
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
    close(fds[1]);

    int count;
    int p = open(file1, O_CREAT | O_WRONLY | O_TRUNC, S_IREAD | S_IWRITE);
    while (1)
    {
        count = check("read failed", read(fds[0], buffer, BUFFER_SIZE));
        if (count == 0) // eof
            break;

        write_to_descr(buffer, count, 1);
        write_to_descr(buffer, count, p);
    }

    int status;
    waitpid(pid, &status, 0);
}

int main(int argc, char* argv[]) 
{
    if (argc < 2) {
        char* message = "Usage: watchthis <time> <command> <command arg 0> ...\n";
        write(1, message, strlen(message));
        exit(1);
    }

    buffer = check_malloc("malloc error\n", BUFFER_SIZE);

    int interval = atoi(argv[1]);

    char ** cmds;
    cmds = (char**)check_malloc("malloc error\n", sizeof(char*) * (argc - 1));

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
            execlp("diff", "diff", "-u", file2, file1, NULL);
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

