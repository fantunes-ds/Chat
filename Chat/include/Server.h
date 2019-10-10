#pragma once

#include <WinSock2.h>

#include <string>
#include <unordered_map>

class Server
{
public:
	Server() = default;
	~Server();

	void Init();

	void Run();
private:
    struct Client
    {
		SOCKET clientSocket{};
		std::string username;
		size_t id{};
        bool isReceiving {false};
    };

    int InitLib();
    int InitSocket();
    int Bind() const;

	int Listen() const;
	int Accept();

    void ReceiveMessage(Client& p_client);
    void Send(const Client& p_client, const std::string& p_message) const;
    void BroadcastMessage(const std::string& p_message);
    void DisplayConnectedClients();

    int Close();


    SOCKET m_sock;
	std::unordered_map<size_t, Client> m_clients{};

    bool m_shouldClose {false};
    const size_t m_maxConnections = 2;
};
