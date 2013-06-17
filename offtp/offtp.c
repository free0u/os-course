#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <fcntl.h>
#include <pty.h>
#include <fstream>
#include <sys/poll.h>

int child_pid = 0;

int check(const char * message, int what) {
    if (what < 0) {
        perror(message);
        _exit(1);
    }
    return what;
}

void handler(int foo) {
    (void)foo;
    if (child_pid)
        kill(-child_pid, SIGINT);
}

int find_char(char* s, int len, char c) {
    int i = 0;
    while (i < len) {
        if (s[i] == c) return i;
        ++i;
    }
    return -1;
}

int write_all(int file, const char * buf, int cnt) {
    int st = 0;
    while(st < cnt) {
        int sz = check("write", write(file, buf + st, cnt));
        st += sz;
    } 
    return 0;
}

int get_file_size(const char * name) {
    struct stat st;
    stat(name, &st);
    return st.st_size;
}

void itoa(int value, char * buf) {
    sprintf(buf, "%d", value);
}

int main(void)
{
    child_pid = fork();
    if (child_pid) { // parent
        printf("daemon started: pid = %d\n", child_pid);
        signal(SIGINT, handler);
        int status;
        waitpid(child_pid, &status, 0);
        printf("daemon stoped\n");
        return 0;
    }
    // child -- daemon
    setsid();
    close(0);
    close(1);
    close(2);

    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    getaddrinfo(NULL, "8823", &hints, &res);

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    int status = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &status, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    } 
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, 10);

    addr_size = sizeof their_addr;

    while (1) {
        // Waiting for accept
        int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

        int pid = fork();
        if (pid == 0) { //child
            close(sockfd);
            const int BUFFER_SIZE = 4096;
            char buffer[BUFFER_SIZE];

            int pos = 0;
            while (true) {
                int cnt = check("read err", read(new_fd, buffer + pos, BUFFER_SIZE - pos));
                if (cnt == 0) break;
                pos += cnt;
                if (find_char(buffer, pos, 0) != -1) break;
            } 

            char buf_size[20];
            itoa(get_file_size(buffer), buf_size);

            int f = open(buffer, O_RDONLY, S_IREAD | S_IWRITE);
            if (f == -1) {
                char * err = strerror(errno);
                write_all(new_fd, "err: ", 5);
                write_all(new_fd, err, strlen(err));
                close(new_fd);
                return 0;
            }
            write_all(new_fd, "ok\0", 3);
            write_all(new_fd, buf_size, strlen(buf_size) + 1);
            
            while (true) {
                int cnt = check("read err", read(f, buffer, BUFFER_SIZE));
                if (cnt == 0) break;
                write_all(new_fd, buffer, cnt);
            }
            close(f);

            close(new_fd);
            return 0;
        } else
        {
            close(new_fd);
        }
    }
    close(sockfd);
    return 0;
}
