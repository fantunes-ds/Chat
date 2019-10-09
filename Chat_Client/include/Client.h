#pragma once

#include <WinSock2.h>
#include <string>

class Client
{
public:
	Client() = default;
	~Client();

	void Init();

	void TryConnect(const std::string& p_address, unsigned int p_port);
	void Run();
private:
	int InitLib();
	int InitSocket();

	void Connection(const std::string& p_address, unsigned p_port);
	void DecodeAddress();

	void Send();

    void Close();

    SOCKET m_sock;
    SOCKADDR_IN m_sin;
    bool m_shouldClose {false};
};