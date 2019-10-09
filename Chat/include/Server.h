#pragma once

#include <WinSock2.h>

#include <string>

class Server
{
public:
	Server() = default;
	~Server();

	void Init();

	void Run();
private:
    int InitLib();
    int InitSocket();
    int Bind();

	int Listen();
	int Accept();

	int ReceiveMessage();
    void BroadcastMessage(const std::string& p_message);

    int Close();

    SOCKET m_sock;
	SOCKET m_csock;
    bool m_shouldClose {false};
};