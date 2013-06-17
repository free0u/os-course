#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
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

void handler(int foo) {
    (void)foo;
    if (child_pid)
        kill(-child_pid, SIGINT);
}

int write_all(int file, char * buf, int cnt) {
    int st = 0;
    while(cnt > 0) {
        int sz = write(file, buf + st, cnt);
        if (sz == -1) return -1;
        st += sz;
        cnt -= sz;
    } 
    return 0;
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

        char buffer[4096];
        while (true) {
            int cnt = read(new_fd, buffer, 4096);
            if (cnt < 1) break;
            write(new_fd, buffer, cnt);
        }
    }
    close(sockfd);
    return 0;
}
