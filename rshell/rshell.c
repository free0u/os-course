#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int child_pid = 0;

void handler(int foo) {
    if (child_pid)
        kill(-child_pid, SIGINT);
}

int main(void)
{
//    child_pid = fork();
//    if (child_pid == 0) {
//        setsid();
//    } else
//    {
//        signal(SIGINT, &handler);
//    } 

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

    while (true) {
        printf("Waiting for accept\n");
        int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

        int pid = fork();
        if (pid == 0) {
            close(sockfd);

            dup2(new_fd, 0);
            dup2(new_fd, 1);
            dup2(new_fd, 2);
    
            printf("Hello, world\n");

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
