

#include <cstring>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <unistd.h>  // for close

#define PORT  "5001"


#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>


#include <string>
#include <queue>




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




// std::string as stand-in for config update or key event
static std::queue<std::string> commands;
static std::mutex queueLock;

void tcpServerTask()
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
        // return 1;
        return;
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
            perror("setsockopt 1");
            // return 1;
            return;
        }

        // https://stackoverflow.com/a/2939145
        timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv)) == -1)
        {
            perror("setsockopt 2");
            return;
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
        // return 1;
        return;
    }

    if (listen(sockfd, 0) == -1)
    {
        perror("listen");
        // return 1;
        return;
    }

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

        // If recv times out, drop the connection
        while (true)
        {
            char buffer[33] = {0};
            ssize_t recvLen = recv(newfd, buffer, sizeof(buffer)-1, 0);  // TODO: MSG_WAITALL
            buffer[recvLen] = '\0';

            if (recvLen == -1)
            {
                perror("recv");
                break;
            }
            else if (recvLen == 0)
            {
                // Remote closed the connection, apparently
                break;
            }

            // TODO: Can I just block until there's something?
            // Should I use flags = MSG_WAITALL ?
            // if (recvLen <= 0) continue;

            char buffer2[64] = {0};
            // snprintf(buffer2, sizeof(buffer2)-1, "%s says (%zd) \"%s\"", s, recvLen, buffer);
            snprintf(buffer2, sizeof(buffer2)-1,
                "(len=%zd) \"%s\"", recvLen, buffer);
            std::string commandString {buffer2};
            {
                std::lock_guard<std::mutex> guard {queueLock};
                commands.push(commandString);
            }

            send(newfd, buffer, recvLen, 0);

            // TODO: Only break connection on QUIT message or similar (or timeout)
            // close(newfd);
            // break;
        }


        // if (send(newfd, "Hello, world!!\n", 15, 0) == -1)
        // {
        //     perror("send");
        // }
        // close(newfd);

    }
}







int main()
{

    std::thread tcpServerThread {tcpServerTask};

    int i = 0;
    while (true)
    {
        i += 5;
        {
            std::lock_guard<std::mutex> guard {queueLock};
            if (commands.empty())
            {
                std::cout << "Still waiting (" << i << ")" << std::endl;
            }
            else
            {
                // move from queue?
                std::string command = commands.front();
                commands.pop();
                std::cout << "Command: [" << command << "]" << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }


}
