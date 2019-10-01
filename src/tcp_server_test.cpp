
#include <cstring>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <unistd.h>  // for close


#define PORT  "5001"
#define BACKLOG 10


// get sockaddr, IPv4 or IPv6:
void* get_in_addr(sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        auto sa_sin = reinterpret_cast<sockaddr_in*>(sa);
        return &(sa_sin->sin_addr);
    }
    else
    {
        auto sa_sin6 = reinterpret_cast<sockaddr_in6*>(sa);
        return &(sa_sin6->sin6_addr);
    }
}



int main()
{
    addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  // use my IP

    addrinfo* pServInfo;
    int rv = getaddrinfo(NULL, PORT, &hints, &pServInfo);
    if (rv != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    char s[INET6_ADDRSTRLEN];
    int sockfd;
    addrinfo* p;
    for (p = pServInfo; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
        {
            perror("server: socket");
            continue;
        }

        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            perror("setsockopt");
            return 1;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }
    freeaddrinfo(pServInfo);

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind \n");
        return 1;
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        return 1;
    }


    // sigaction sa;
    // sa.sa_handler = sigchld_handler;

    printf("server: waiting for connections...\n");

    sockaddr_storage theirAddr;
    while (true)
    {
        socklen_t theirAddrLen = sizeof(theirAddr);
        int newfd = accept(sockfd, reinterpret_cast<sockaddr*>(&theirAddr), &theirAddrLen);
        if (newfd == -1)
        {
            perror("accept");
            continue;
        }

        inet_ntop(theirAddr.ss_family,
            get_in_addr(reinterpret_cast<sockaddr*>(&theirAddr)),
            s, sizeof(s));
        printf("server: got connection from %s \n", s);

        if (send(newfd, "Hello, world!!\n", 15, 0) == -1)
        {
            perror("send");
        }
        close(newfd);

        break;
    }

    return 0;
}
