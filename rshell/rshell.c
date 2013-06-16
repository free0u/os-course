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

        int pid = fork();
        if (pid == 0) { // child
            close(sockfd);
            // tty
            int master, slave;
            char buf[4096];
            if (openpty(&master, &slave, buf, NULL, NULL) == -1) {
                close(new_fd);
                perror("openpty error");
                return 0;
            }
            
            if (fork() == 0) { // child
                close(master);
                close(new_fd);
                dup2(slave, 0);
                dup2(slave, 1);
                dup2(slave, 2);
                setsid();
                int term = open(buf, O_RDWR);
                if (term) close(term);
                execl("/bin/zsh", "/bin/zsh", NULL);
            } else // parent
            {
                close(slave);
                int buf_size = 4096;
                char buf1[buf_size];
                char buf2[buf_size];
                int cnt1 = 0, cnt2 = 0;

                pollfd fd[2];
                fd[0].fd = master;
                fd[1].fd = new_fd;
               
                while(1) {
                    fd[1].events = (cnt1 > 0 ? POLLOUT : 0);
                    fd[0].events = (cnt1 < buf_size ? POLLIN : 0);

                    fd[0].events |= (cnt2 > 0 ? POLLOUT : 0);
                    fd[1].events |= (cnt2 < buf_size ? POLLIN : 0);
                    int ret = poll(fd, 2, -1);

                    if (ret == -1) {
                        _exit(1);
                    }

                    if (fd[0].revents & (POLLERR | POLLHUP)) {
                        break;
                    }
                    if (fd[1].revents & (POLLERR | POLLHUP)) {
                        break;
                    }

                    if (fd[0].revents & POLLIN) {
                        int sz = read(master, buf1 + cnt1, buf_size - cnt1);
                        if (sz == -1) _exit(2);
                        cnt1 += sz;
                    }
                    if (fd[1].revents & POLLOUT) {
                        int sz = write(new_fd, buf1, cnt1);
                        if (sz == -1) _exit(1);
                        memmove(buf1, buf1 + sz, buf_size - sz);
                        cnt1 -= sz;
                    }

                    if (fd[1].revents & POLLIN) {
                        int sz = read(new_fd, buf2 + cnt2, buf_size - cnt2);
                        if (sz == -1) _exit(2);
                        cnt2 += sz;
                    }
                    if (fd[0].revents & POLLOUT) {
                        int sz = write(master, buf2, cnt2);
                        if (sz == -1) _exit(1);
                        memmove(buf2, buf2 + sz, buf_size - sz);
                        cnt2 -= sz;
                    }
                } 
                if (cnt1 > 0) {
                    write_all(new_fd, buf1, cnt1);
                }
                if (cnt2 > 0) {
                    write_all(master, buf2, cnt2);
                }
                return 0;
            }
        } else { // parent
            close(new_fd);
        }
    }
    close(sockfd);
    return 0;
}
