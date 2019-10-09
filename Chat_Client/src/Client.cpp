#include <iostream>
#include <cassert>

#include <Client.h>

#include <WS2tcpip.h>


#define PORT 76475

Client::~Client()
{
	Close();
}

void Client::Init()
{
    assert(InitLib() >= 0);
	InitSocket();
}

int Client::InitLib()
{
	WSADATA wsa;
	int err = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (err < 0)
	{
		std::cout << "WSAStartup failed !" << std::endl;
		std::cin.get();
		return EXIT_FAILURE;
	}
	std::cout << "init" << std::endl;
	return EXIT_SUCCESS;
}

int Client::InitSocket()
{
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET)
	{
		perror("socket()");
		std::cout << "INVALID_SOCKET" << std::endl;
		std::cin.get();
		return errno;
	}
	std::cout << "init socket" << std::endl;
	return EXIT_SUCCESS;
}

void Client::Connection(const std::string& p_address, unsigned int p_port)
{
	struct hostent* hostinfo = NULL;

	inet_pton(AF_INET, p_address.c_str(), &m_sin.sin_addr);/* On encode l'adresse dans la variable sin.sin_addr*/
	m_sin.sin_port = htons(p_port); /* on utilise htons pour le port */
	m_sin.sin_family = AF_INET;
}

void Client::DecodeAddress()
{
	char str[15];
	inet_ntop(AF_INET, &(m_sin.sin_addr), str, 15);
	std::cout << str << std::endl;
}

void Client::Run()
{
    while(!m_shouldClose)
    {
		Send();
    }
}

void Client::TryConnect(const std::string& p_address, unsigned int p_port)
{
	Connection(p_address, p_port);
	DecodeAddress();
	while (connect(m_sock, reinterpret_cast<SOCKADDR*>(&m_sin), sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		perror("connect()");
	}
}

void Client::Send()
{
	std::string p_message;
	std::getline(std::cin,p_message);

	if (send(m_sock, p_message.c_str(), p_message.length(), 0) < 0)
	{
		perror("send()");
	}

    if (p_message == "Quit")
    {
		m_shouldClose = true;
    }


	std::cout << "sending: " << p_message << std::endl;
}

void Client::Close()
{
	std::cout << "close listen" << std::endl;
	closesocket(m_sock);
	std::cout << "close ori" << std::endl;
	WSACleanup();
}

