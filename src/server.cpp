
#include "server.hpp"

#include <cstring>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>  // for close


// TODO: Configurable
#define PORT  "5001"


#include <stdio.h>


Server::Server() :
    m_Commands {},
    m_CommandsLock {},
    m_Ending {false}
{
}


bool Server::getCommand(Command& rCommand)
{
    std::lock_guard<std::mutex> guard {m_CommandsLock};
    const bool hasCommand = ! m_Commands.empty();
    if (hasCommand)
    {
        rCommand = m_Commands.front();
        m_Commands.pop();
    }
    return hasCommand;
}


// get sockaddr, IPv4 or IPv6:
static void* get_in_addr(sockaddr *sa)
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

void Server::start()
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

        // // https://stackoverflow.com/a/2939145
        // timeval tv;
        // tv.tv_sec = 5;
        // tv.tv_usec = 0;
        // if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv)) == -1)
        // {
        //     perror("setsockopt 2");
        //     return;
        // }

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
    while ( ! m_Ending)
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

        // Commands come in a stream of register assignments.
        //   [u16] - Register number
        //   [u16] - Register value
        //   ...
        //
        // const size_t MAX_ASSIGNMENTS = 1024;
        // const size_t messageSize = sizeof(uint8_t) + MAX_ASSIGNMENTS * sizeof(Command);
        // char buffer[messageSize];
        //
        // size_t bytesGotten = 0;


        const ssize_t MESSAGE_SIZE = sizeof(uint16_t) + sizeof(uint16_t);
        const size_t MESSAGE_BUFFER_SIZE = 128 * MESSAGE_SIZE;
        uint8_t recvBuffer[MESSAGE_BUFFER_SIZE];

        ssize_t bytesLeftover = 0;
        while (true)
        {

            // TODO: Just read pairs of uint16_t from the input stream.
            // Move any bytes that exceed the last multiple of four to
            // the start of the buffer and do it again.

            ssize_t bytesReceived = recv(newfd, recvBuffer + bytesLeftover, sizeof(recvBuffer) - bytesLeftover, 0);
            if (bytesReceived == -1)
            {
                perror("recv");
                break;
            }
            else if (bytesReceived == 0)
            {
                // Remote end closed the connection
                break;
            }

            // Consider any previously leftover bytes as part of the newly received bytes.
            bytesReceived += bytesLeftover;
            const ssize_t messageCount = bytesReceived / MESSAGE_SIZE;
            bytesLeftover = bytesReceived - (messageCount * MESSAGE_SIZE);

            // TODO: Better way? Should I create a packed struct for commands?
            // auto pU16Stream = reinterpret_cast<uint16_t*>(recvBuffer);
            // TODO: Use this in sizeof for buffer above, MESSAGE_SIZE, etc.
            struct CommandMessage {  // TODO: Does this interfere with the real "command" class?
                uint16_t registerNumber;
                uint16_t registerValue;
            };  // TODO: __attribute__((packed))
            auto pCommandStream = reinterpret_cast<CommandMessage*>(recvBuffer);

            printf("Got %zd commands \n", messageCount);
            for (ssize_t i = 0; i < messageCount; i++)
            {
                // TODO: Clean this up
                auto command = pCommandStream[i];
                const uint16_t registerNumber = ntohs(command.registerNumber);
                const uint16_t registerValue = ntohs(command.registerValue);

                // TODO: Find a better way
                if (registerValue == 0xffff && registerValue == 0xffff)
                {
                    printf("Got magic quit command \n");
                    m_Ending = true;
                }

                // TODO: Get this lock for inserting all commands at once?
                std::lock_guard<std::mutex> guard {m_CommandsLock};
                m_Commands.push({registerNumber, registerValue});
            }

            // Move any leftover bytes to the start of the buffer for next time.
            std::memcpy(recvBuffer, recvBuffer + (bytesReceived - bytesLeftover), bytesLeftover);

            // TODO: Error handling?
            send(newfd, "OK\n", 3, 0);

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

// void Server::end()
// {
//     m_Ending = false;
// }
