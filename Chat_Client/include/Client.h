#pragma once

#include <WinSock2.h>
#include <string>

class Client
{
public:
    Client() = default;
    ~Client();

    void Init();

    void TryConnect();
    void Run();
private:
    int  InitLib();
    int  InitSocket();
    void SetUsername();

    void Connection(const std::string& p_address, unsigned p_port);
    void DecodeAddress() const;

    void Send();
    void Send(const std::string& p_message) const;
    void ReceiveMessage();

    void Close();

    SOCKET             m_serverSocket;
    SOCKADDR_IN        m_sin;
    std::string        m_username{};
    const std::string  m_defaultAddress{"127.0.0.1"};
    const unsigned int m_defaultPort{8765};
    bool               m_shouldClose{false};
    bool               m_isConnected{false};
    bool               m_shouldThreadStop{false};
};
