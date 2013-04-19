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
char *name_files[] = {"/tmp/out1.txt", "/tmp/out2.txt"};
char *buffer;

void write_to_descr(char * buf, int len, int file)
{
    int pos = 0;
    while (len > 0)
    {
        int count = write(file, buf + pos, len);
        if (count < 0)
        {
            exit(255);
        }
        pos += count;
        len -= count;
    }
}
void run_cmd(char* argv[], int pos)
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
        exit(255);
    }

    int count, len;
    len = 0;
    close(fds[1]);
    while (1)
    {
        count = read(fds[0], buffer + len, 4096 - len);
        if (4096 - len != 0 && count == 0) break; // eof
        if (4096 - len == 0) break; // buffer is full
        len += count;
    }
    write_to_descr(buffer, len, 1);

    int p = open(name_files[pos], O_CREAT | O_WRONLY | O_TRUNC, S_IREAD | S_IWRITE);
    write_to_descr(buffer, len, p);

    int status;
    waitpid(pid, &status, 0);
}

int main(int argc, char* argv[]) 
{
    buffer = malloc(4096);

    int interval = atoi(argv[1]);

    char ** cmds;
    cmds = malloc(argc - 1);

    int i = 0;
    for (i = 2; i < argc; ++i)
    {
        cmds[i - 2] = argv[i];
    }
    cmds[argc - 2] = NULL;

    int state = 0;
    while (1)
    {
        int pid = fork();
        if (!pid)
        {
            execlp("clear", "clear", NULL);
        }
        waitpid(pid, NULL, 0);

        run_cmd(cmds, state);
        
        write(1, "\nDiff:\n\n", 8);
        pid = fork();
        if (!pid)
        {
            execlp("diff", "diff", "-u", name_files[0], name_files[1], NULL);
        }
        waitpid(pid, NULL, 0);

        state = !state;
        sleep(1);
    }

    free(cmds);
    free(buffer);
    return 0;
}

