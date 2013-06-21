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

enum state {READ, SKIP};

int main(int argc, char ** argv)
{
    if (argc < 3) {
        char message[] = "Usage: client <ip> <port>\n";
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

    int pid = fork();
    if (pid == 0) { // child
        char buffer[4096];
        char endl = '\n';
        while (true) {
            int cnt = check("read err", read(sockfd, buffer, 4096));
            if (cnt == 0) break;
            write_all(1, buffer, cnt);
            write_all(1, &endl, 1);
        }
        return 0;
    }
    int k = 4096;
    char buffer[k + 1]; 
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
                char c = buffer[pos_r];
                buffer[pos_r] = 0;
                write_all(sockfd, buffer + pos_l, pos_r - pos_l + 1);
                buffer[pos_r] = c;

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
    
            if (eof_found && len > 0) {
                buffer[len] = 0;
                write_all(sockfd, buffer, len + 1);
            }
        }
    }

    close(sockfd);
    return 0;
}
