#pragma once

#include <WinSock2.h>
#include <string>

class Client
{
public:
    Client() = default;
    ~Client();

    // Initializes Lib and Socket
    void Init();

    // Tries to connect. In case of failure, tries again on next input.
    void TryConnect();
    // Triggers Send() on repeat until program ends. Keeps program running.
    void Run();
private:
    int  InitLib();
    // Readies socket for connection
    int  InitSocket();
    // Changes Username and sends it to the server
    void SetUsername();

    /**
     * @brief Connects client to address p_address and port p_port
     * @param p_address address to connect to
     * @param p_port port to connect to
     */
    void Connect(const std::string& p_address, const unsigned int p_port);
    void DecodeAddress() const;

    // Sends an input chosen by the user
    void Send();
    // Sends a message to the server with specific request. User has no control of this.
    void Send(const std::string& p_message) const;

    //Receives and treats messages. Threaded.
    void ReceiveMessage();

    void Close();

    SOCKET            m_serverSocket;
    SOCKADDR_IN       m_sin;
    std::string       m_username{};
    const std::string m_defaultAddress{"127.0.0.1"};
    const int         m_defaultPort{8765};
    bool              m_shouldClose{false};
    bool              m_isConnected{false};
    bool              m_shouldThreadStop{false};
};
