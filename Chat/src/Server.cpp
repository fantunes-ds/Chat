#include <iostream>

#include <Server.h>
#include <cassert>

#define PORT 8765

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
		if (ReceiveMessage() < 0)
			Accept();
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
    return EXIT_SUCCESS;
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
    return EXIT_SUCCESS;
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
    return EXIT_SUCCESS;
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
    return EXIT_SUCCESS;
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
    return EXIT_SUCCESS;
}

int Server::ReceiveMessage()
{
	char buffer[1024];
	int n = 0;

	if ((n = recv(m_csock, buffer, sizeof buffer - 1, 0)) < 0)
	{
		perror("User Disconnected");
		return -1;
		exit(errno);
	}
    if (buffer[n-1] != NULL)
	    buffer[n] = '\0';
	std::string stringBuffer{ buffer };

	if (stringBuffer.find_last_of(": Quit\0") == 1)
	{
		closesocket(m_csock);
		std::cout << ("User Disconnected");
		return -1;
	}

	if (stringBuffer.find_last_of(": !close\0") == 1)
		m_shouldClose = true;

    std::cout << buffer << std::endl;
    return EXIT_SUCCESS;
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
