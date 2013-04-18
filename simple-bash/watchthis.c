#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

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

void run_cmd(char* argv[])
{
    int pid = fork();
    if (pid == 0) // child
    {
        //int p = open("out.txt", O_CREAT | O_WRONLY | O_TRUNC, S_IREAD | S_IWRITE);
       
        execvp(argv[0], argv);
        exit(255);
    }
    int status;
    waitpid(pid, &status, 0);
}

int main(int argc, char* argv[]) 
{
    int interval = atoi(argv[1]);

    char ** cmds;
    cmds = malloc(argc - 1);

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
        sleep(1);
    }

    free(cmds);
    return 0;
}

