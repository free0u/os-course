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

int find_char(char* s, int from, int to, char c) {
    int i = from;
    while (i < to) {
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

int main(int argc, char ** argv)
{
    if (argc < 4) {
        char message[] = "Usage: client <ip> <port> <name file>\n";
        write_all(1, message, strlen(message));
        exit(1);
    }
    struct addrinfo hints, *res;
    int sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    getaddrinfo(argv[1], argv[2], &hints, &res);

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(sockfd, res->ai_addr, res->ai_addrlen);

    write_all(sockfd, argv[3], strlen(argv[3]) + 1);
    
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];

    int len = 0;
    while (len < 30) {
        int cnt = check("read err", read(sockfd, buffer + len, BUFFER_SIZE - len));
        if (cnt == 0) break;
        len += cnt;
    }
    
    const char * io_error = "i/o error\n";
    int cur_fs = 0;
    if (len > 3) {
        char * p = strstr(buffer, "err");
        if (p == buffer) {
            write_all(2, buffer, len);
        } else
        {
            p = strstr(buffer, "ok");
            if (p != buffer) {
                write_all(2, io_error, strlen(io_error));
                close(sockfd);
                return 0;
            }

            int p1 = find_char(buffer, 0, len, 0) + 1;
            int fs = atoi(buffer + p1);
            p1 = find_char(buffer, p1, len, 0) + 1;
            write_all(1, buffer + p1, len - p1);
            cur_fs += len - p1;

            while (true) {
                len = check("read err", read(sockfd, buffer, BUFFER_SIZE));
                if (len == 0) break;
                cur_fs += len;
                write_all(1, buffer, len); 
            }
            //printf("\n%d/%d\n", cur_fs, fs);
            if (cur_fs != fs) {
                write_all(2, io_error, strlen(io_error));
                close(sockfd);
                return 0;
            } 
        }
    } else {
        write_all(2, io_error, strlen(io_error));
    }

    close(sockfd);
    return 0;
}
