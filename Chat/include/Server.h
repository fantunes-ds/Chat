#pragma once

#include <WinSock2.h>

#include <string>
#include <unordered_map>

class Server
{
public:
    Server() = default;
    ~Server();

    // Initializes Lib and Socket
    void Init();

    // Keeps program running until it doesn't need to anymore.
    void Run();
private:
    struct Client
    {
        SOCKET      clientSocket{};
        std::string username;
        size_t      id{};
    };

    int InitLib();
    // Readies socket for connection
    int InitSocket();
    int Bind() const;

    // Readies socket to wait for connections
    int Listen() const;
    // Waits for a connection to accept it and link it to the server. Threaded.
    int AcceptNewClient();

	// Receives and treats messages. Threaded.
    void        ReceiveClientMessage(Client& p_client);
    // Reads Message from Client
    std::string CatchClientMessage(const Client& p_client);
    // Terminates connection and deletes all references from the client.
    void        DisconnectClient(const Client& p_client);
    // Verifies for exceptions on messages (disconnections, lsot signal, end)
    int         CheckForExceptions(std::string& p_stringBuffer, Client& p_client);
    // Sends a message to a very specific client.
    void        Send(const Client& p_client, const std::string& p_message) const;
    // Retransmits a caught message to everyone but the sender of the message.
    void        BroadcastMessage(const std::string& p_message,
                                 const size_t       p_originId);

    int Close();


    SOCKET                             m_sock;
    std::unordered_map<size_t, Client> m_clients{};

    bool         m_shouldClose{false};
    const size_t m_maxConnections = 2;
};
