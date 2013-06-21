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
#include <map>
#include <set>
#include <vector>
#include <string>

using namespace std;

#define MAX_CLIENTS 10

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


const int BUFFER_SIZE = 4096;
struct my_buffer {
    char* buf;
    int len;

    my_buffer() {} 

    void init() {
        len = 0;
        buf = (char*)malloc(BUFFER_SIZE);
        memset(buf, 0, BUFFER_SIZE);
    }

    void add(const char * s, int n) {
        memmove(buf + len, s, n);
        len += n;    
        buf[len++] = 0;
    }

    ~my_buffer() {
        if (buf) free(buf);
    }
};

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
    //close(1);
    //close(2);

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
    listen(sockfd, MAX_CLIENTS);

    addr_size = sizeof their_addr;

    pollfd fd[MAX_CLIENTS + 1];
    fd[0].fd = sockfd;
    fd[0].events = POLLIN;

    my_buffer out[MAX_CLIENTS + 1];
    //my_buffer in[MAX_CLIENTS + 1];


    map<string, set<int>> mm;
    int cnt_clients = 1;
    while (1) {
        check("poll err", poll(fd, cnt_clients, -1));

        for (int i = 1; i < cnt_clients; ++i) {
            if (fd[i].revents & (POLLERR | POLLHUP)) {
                fd[i].events = 0;
                fd[i].fd = -1;
                continue;
            }

            if (fd[i].revents & POLLOUT && out[i].len != 0) {
                int cnt = check("write err", write(fd[i].fd, out[i].buf, out[i].len));
                memmove(out[i].buf, out[i].buf + cnt, out[i].len - cnt);
                out[i].len -= cnt;
                if (out[i].len == 0) {
                    fd[i].events = POLLIN;
                }
            }

            if (fd[i].revents & POLLIN) {
                char buffer[100];
                int cnt = check("read err", read(fd[i].fd, buffer, 100));
                if (cnt == 0) {
                    if (fd[i].events & POLLOUT) fd[i].events = POLLOUT;
                    else fd[i].events = 0;
                }
                if (buffer[0] == 'l') {
                    out[i].add("signals:\n", 9);
                    for (auto it = mm.begin(); it != mm.end(); ++it) {
                        string s = it->first;
                        if (mm[s].find(i) != mm[s].end()) {
                            out[i].add(s.c_str(), s.size());
                            out[i].add("\n", 1);
                        }
                    }
                    fd[i].events |= POLLOUT;
                } else {
                    int pos = find_char(buffer, cnt, ' ') + 1;
                    string s(buffer + pos, cnt - pos);

                    if (buffer[0] == 's') {
                        mm[s].insert(i);
                    } else if (buffer[0] == 'u') {
                        mm[s].erase(i);
                        out[i].len = 0;
                    } else if (buffer[0] == 'e') {
                        for (auto it = mm[s].begin(); it != mm[s].end(); ++it) {
                            int ind = *it;
                            out[ind].add(buffer + pos, cnt - pos); 
                            fd[ind].events |= POLLOUT;
                        }
                    }
                }
            }
        }

        if (fd[0].revents & POLLIN) {
            int new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_size);
            fd[cnt_clients].fd = new_fd;
            fd[cnt_clients].events = POLLIN;
            out[cnt_clients].init();
            cnt_clients++;
        }
    }

    close(sockfd);
    return 0;
}
