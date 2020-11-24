#include "serverinfo.h"
#include "helpers.hh"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <cstdlib>
#include <cstring>
#include <strings.h>
#include <cstdio>
#include <cerrno>
#include <csignal>
#include <cassert>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
// stuff students might use
#include <syslog.h>
#include <semaphore.h>

const bool debugging = false;

struct proxy_config {
    int index = -1;
    int port = -1;
    const char* type = nullptr;
    unsigned delay = 0;
    size_t max_connections = 0;
    int listenfd = -1;

    int open_listen_socket();
    void accept_connections();
    void handle_client(int clientfd);
};


static const char* pong_host = PONG_HOST;
static int pong_port = PONG_PORT;
static struct addrinfo* pong_addrinfo;
static const int nproxies = PROXY_COUNT;
static const int proxy_start_port = PROXY_START_PORT;
static bool listen_all = false;


// open_listen_socket(port)
//    Open a socket for listening on `port`. The socket will accept
//    connections from any host, and has a listen queue of 100
//    connections.

int proxy_config::open_listen_socket() {
    // Create socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    int yes = 1;
    int r = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (r < 0) {
        perror("setsockopt");
        close(fd);
        return -1;
    }

    // Bind to port
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(this->port);
    if (listen_all) {
        address.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }
    r = bind(fd, (struct sockaddr*) &address, sizeof(address));
    if (r < 0) {
        perror("bind");
        close(fd);
        return -1;
    }

    // Actually start listening
    r = listen(fd, 100);
    if (r < 0) {
        perror("listen");
        close(fd);
        return -1;
    }

    this->listenfd = fd;
    return fd;
}

void proxy_config::accept_connections() {
    size_t nconnections = 0;

    while (true) {
        // when a client connects, spawn a new thread to handle it.
        socklen_t clientlen;
        struct sockaddr_in clientaddr;
        int cfd = accept(this->listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        if (cfd < 0) {
            perror("accept");
        }

        ++nconnections;
        if (this->max_connections != 0
            && nconnections >= this->max_connections) {
            fprintf(stderr, "Wrong proxy!!!\n");
        }

        std::thread t([this, cfd] () {
            this->handle_client(cfd);
        });
        t.detach();
    }
}

struct http_transfer_buffer {
    char buf[BUFSIZ];
    ssize_t len = 0;

    ssize_t clpos = 0;
    const char* const clmsg = "content-length:";
    const ssize_t cllen = 15;
    ssize_t content_length = -1;

    ssize_t ehpos = 0;
    const char* const ehmsg = "\r\n\r\n";
    const ssize_t ehlen = 4;

    const char* insertmsg;
    ssize_t insertlen = 0;

    http_transfer_buffer(const char* insertmsg_, ssize_t insertlen_)
        : insertmsg(insertmsg_), insertlen(insertlen_) {
    }
    ssize_t run(int fromfd, int tofd, unsigned delay);
};

static ssize_t my_read(int fd, char* buf, size_t sz) {
    ssize_t nr = read(fd, buf, sz);
    if (debugging && nr > 0) {
        fprintf(stderr, "read[%d] \"%.*s\"\n", fd, int(nr), buf);
    }
    return nr;
}

static ssize_t my_write(int fd, const char* buf, size_t sz) {
    ssize_t nw = write(fd, buf, sz);
    if (debugging && nw > 0) {
        fprintf(stderr, "write[%d] \"%.*s\"\n", fd, int(nw), buf);
    }
    return nw;
}

ssize_t http_transfer_buffer::run(int fromfd, int tofd, unsigned delay) {
    while (true) {
        // refresh buffer if necessary
        if (this->len == 0) {
            size_t capacity = sizeof(this->buf) - this->insertlen - this->ehlen;
            this->len = my_read(fromfd, this->buf, capacity);
            if (this->len == 0) {
                return 0;
            } else if (this->len == -1) {
                if (errno != EINTR && errno != EAGAIN) {
                    perror("read");
                    return -1;
                } else {
                    this->len = 0;
                    continue; // retry
                }
            } else if (delay > 0) {
                usleep(delay * 1000);
            }
        }

        // transfer headers, parsing `content-length`
        ssize_t pos = 0;
        for (; pos != this->len && this->ehpos != this->ehlen; ++pos) {
            char ch = this->buf[pos];
            if (ch == this->ehmsg[this->ehpos]) {
                ++this->ehpos;
            } else {
                this->ehpos = 0;
            }
            if (this->clpos == this->cllen) {
                if (ch >= '0' && ch <= '9') {
                    this->content_length = ch - '0'
                        + std::max(this->content_length, ssize_t(0)) * 10;
                } else if (ch != ' ' && ch != '\t') {
                    ++this->clpos;
                }
            } else if (this->clpos < this->cllen) {
                if (tolower((unsigned char) ch) == this->clmsg[this->clpos]) {
                    ++this->clpos;
                } else {
                    this->clpos = 0;
                }
            }
        }

        // add `this->insertmsg` after headers
        if (pos > 0 && this->insertlen > 0) {
            if (this->ehpos == this->ehlen && pos >= this->ehlen) {
                memmove(this->buf + this->insertlen + pos - this->ehlen,
                        this->buf + pos - this->ehlen,
                        this->len - (pos - this->ehlen));
                memcpy(this->buf + pos - this->ehlen, this->insertmsg, this->insertlen);
                this->len += this->insertlen;
                pos += this->insertlen;
            } else if (this->ehpos == this->ehlen) {
                memmove(this->buf + this->insertlen + this->ehlen,
                        this->buf + pos,
                        this->len - pos);
                memcpy(this->buf, this->insertmsg, this->insertlen);
                memcpy(this->buf + this->insertlen, this->ehmsg, this->ehlen);
                this->len += this->insertlen + this->ehlen - pos;
                pos = this->insertlen + this->ehlen;
            } else {
                pos -= this->ehpos;
            }
        }

        // include available message body if any
        if (this->ehpos == this->ehlen && this->content_length > 0) {
            ssize_t ncopy = std::min(this->len - pos, this->content_length);
            pos += ncopy;
            this->content_length -= ncopy;
        }

        // copy data
        ssize_t nw = my_write(tofd, this->buf, pos);
        assert(nw == pos);

        // shift remaining data down
        memmove(this->buf, this->buf + pos, this->len - pos);
        this->len -= pos;

        // report success if we transferred a full message
        if (this->ehpos == this->ehlen && this->content_length <= 0) {
            this->ehpos = this->clpos = 0;
            this->content_length = -1;
            return 1;
        }
    }
}

//forwards the client's requests to the server and sends the response back to the client
void proxy_config::handle_client(int cfd) {
    //printf("handle client at proxy %d\n", proxyid);

    // connect to server
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd < 0) {
        perror("server socket");
        exit(1);
    }

    int yes = 1;
    (void) setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    int r = connect(serverfd, pong_addrinfo->ai_addr, pong_addrinfo->ai_addrlen);
    if (r < 0) {
        perror("server connect");
        exit(1);
    }

    // transfers to server insert proxy header
    char proxybuf[512];
    size_t proxylen = sprintf(proxybuf, "\r\nX-CS61-Proxy: %s-%d;delay=%u",
                              this->type, this->port, this->delay);
    http_transfer_buffer to_server(proxybuf, proxylen);

    // transfers to client insert nothing
    http_transfer_buffer to_client("", 0);

    while (to_server.run(cfd, serverfd, 0) > 0
           && to_client.run(serverfd, cfd, this->delay) > 0) {
    }

    close(cfd);
    close(serverfd);
}


// usage()
//    Explain how proxypong61 should be run.
static void usage() {
    fprintf(stderr, "Usage: ./proxypong61 [-h HOST] [-p PORT] [-a]\n");
    exit(1);
}


int main(int argc, char** argv) {
    // parse arguments
    int ch;
    while ((ch = getopt(argc, argv, "ah:p:")) != -1) {
        if (ch == 'h') {
            pong_host = optarg;
        } else if (ch == 'p') {
            pong_port = strtol(optarg, nullptr, 0);
            if (pong_port <= 0 || pong_port > 65535) {
                usage();
            }
        } else if (ch == 'a') {
            listen_all = true;
        } else {
            usage();
        }
    }

    // look up the server
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    std::string portstr = std::to_string(pong_port);
    int r = getaddrinfo(pong_host, portstr.c_str(), &hints, &pong_addrinfo);
    if (r != 0) {
        fprintf(stderr, "problem looking up %s: %s\n",
                pong_host, gai_strerror(r));
        exit(1);
    }

    // choose which proxy is fast
    int fast_proxy = random_int(3, 4 * nproxies - 1) / 4;
    fprintf(stderr, "Best proxy is port %d\n", proxy_start_port + fast_proxy);

    // start the proxies
    std::thread ths[nproxies];
    for (int i = 0; i != nproxies; ++i) {
        proxy_config cfg;
        cfg.index = i;
        cfg.port = proxy_start_port + i;
        if (i == fast_proxy) {
            cfg.type = "Fast";
            cfg.delay = random_int(0, 10);
            cfg.max_connections = 0;
        } else {
            cfg.type = "Slow";
            cfg.delay = random_int(50, 200);
            cfg.max_connections = 3;
        }

        if (cfg.open_listen_socket() < 0) {
            exit(1);
        }

        ths[i] = std::thread([cfg] () mutable {
            cfg.accept_connections();
        });
    }

    for (int i = 0; i != nproxies; ++i) {
        ths[i].join();
    }

    return 0;
}
