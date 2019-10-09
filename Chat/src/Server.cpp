#include <iostream>

#include <Server.h>
#include <cassert>

#define PORT 76475

Server::~Server()
{
	Close();
}

void Server::Init()
{
	assert(InitLib() >= 0);
	InitSocket();
	Bind();
}

void Server::Run()
{
	Listen();
	Accept();
    while(!m_shouldClose)
    {
		ReceiveMessage();
    }
}

int Server::InitLib()
{
	WSADATA wsa;
    const int err = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (err < 0)
	{
		std::cout << "WSAStartup failed !" << std::endl;
		std::cin.get();
		return EXIT_FAILURE;
	}
	std::cout << "init" << std::endl;
}

int Server::InitSocket()
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
}

int Server::Bind()
{
	SOCKADDR_IN sin{ 0 };
	sin.sin_addr.s_addr = htonl(INADDR_ANY); 
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);

	if (bind(m_sock, reinterpret_cast<SOCKADDR*>(&sin), sizeof sin) == SOCKET_ERROR)
	{
		perror("bind()");
		std::cout << "SOCKET_ERROR" << std::endl;
		std::cin.get();
		return errno;
	}
	std::cout << "Bind" << std::endl;
}

int Server::Listen()
{
	if (listen(m_sock, 1) == SOCKET_ERROR)
	{
		perror("listen()");
		std::cout << "SOCKET_ERROR" << std::endl;
		std::cin.get();
		return errno;
	}
	std::cout << "Listen" << std::endl;
}


int Server::Accept()
{
	SOCKADDR_IN csin = { 0 };

	int sinsize = sizeof csin;
	m_csock = accept(m_sock, reinterpret_cast<SOCKADDR*>(&csin), &sinsize);


	if (m_csock == INVALID_SOCKET)
	{
		perror("accept()");
		std::cout << "INVALID_SOCKET" << std::endl;
		std::cin.get();
		return errno;
	}
	std::cout << "Accept" << std::endl;
}

int Server::ReceiveMessage()
{
	char buffer[1024];
	int n = 0;

	if ((n = recv(m_csock, buffer, sizeof buffer - 1, 0)) < 0)
	{
		perror("recv()");
		exit(errno);
	}

	std::cout << n << std::endl;
	buffer[n] = '\0';
	if (strncmp(buffer, "!close",6) == 0)
		m_shouldClose = true;
	std::cout << buffer << std::endl;
}

void Server::BroadcastMessage(const std::string& p_message)
{
    //todo implementation;
}

int Server::Close()
{
	closesocket(m_csock);
	std::cout << "close listen socket" << std::endl;
	closesocket(m_sock);
	std::cout << "close socket" << std::endl;
	WSACleanup();
	return EXIT_SUCCESS;
}
