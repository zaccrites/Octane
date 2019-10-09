
#ifndef SERVER_HPP
#define SERVER_HPP

#include <stdint.h>
#include <queue>
#include <mutex>


// Command to set a register value
// TODO: Other kinds of commands?
struct Command
{
    uint16_t registerNumber;
    uint16_t registerValue;
};


class Server
{
public:

    Server();

    void start();
    // void end();

    bool getCommand(Command& rCommand);

    bool quitRequested() const
    {
        return m_Ending;
    }

private:
    std::queue<Command> m_Commands;
    std::mutex m_CommandsLock;

    bool m_Ending;

};


#endif
