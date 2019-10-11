#include <iostream>

#include <Server.h>
#include <cassert>
#include <thread>

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
	std::cout << "server initialized\n";
}

void Server::Run()
{
	Listen();
	std::thread threadAccept { &Server::Accept, this };
	threadAccept.detach();
    while(!m_shouldClose)
    {
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
    return EXIT_SUCCESS;
}

int Server::Bind() const
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
    return EXIT_SUCCESS;
}

int Server::Listen() const
{
	if (listen(m_sock, 1) == SOCKET_ERROR)
	{
		perror("listen()");
		std::cout << "SOCKET_ERROR" << std::endl;
		std::cin.get();
		return errno;
	}
    return EXIT_SUCCESS;
}


int Server::Accept()
{
	SOCKADDR_IN csin = { 0 };
	int sinsize = sizeof csin;

	Client newClient;
	newClient.clientSocket = accept(m_sock, reinterpret_cast<SOCKADDR*>(&csin), &sinsize);

	if (newClient.clientSocket == INVALID_SOCKET)
	{
		perror("accept()");
		std::cout << "INVALID_SOCKET" << std::endl;
		std::cin.get();
		return errno;
	}
	int placement {0};
	while (!m_clients.emplace(m_clients.size() + placement, newClient).second)
		++placement;

	m_clients.find(m_clients.size() - 1)->second.id = m_clients.size() + placement - 1;

    std::thread { &Server::Accept, this }.detach();
	std::thread{ &Server::ReceiveMessage, this, std::ref(m_clients.find(m_clients.size() - 1)->second) }.detach();
    std::cout << "User logged in\n";
    return EXIT_SUCCESS;
}

void Server::ReceiveMessage(Client& p_client)
{
	bool shouldThreadStop{ false };
	while (!shouldThreadStop)
	{
		char buffer[1024]{'\0'};
		int n = 0;

		if ((n = recv(p_client.clientSocket, buffer, sizeof buffer - 1, 0)) < 0)
		{
			closesocket(p_client.clientSocket);
			std::cout << p_client.username + " Disconnected\n";

			m_clients.erase(m_clients.find(p_client.id));
			shouldThreadStop = true;
		}
		if (n < 0)
			n = 0;
		if (buffer[0] != -52 && buffer[n - 1] != NULL)
			buffer[n] = '\0';

		const std::string stringBuffer{ buffer };
		if (stringBuffer.find(": !Quit") != std::string::npos)
		{
			Send(p_client, "Disconnecting");

			closesocket(p_client.clientSocket);
			std::cout << (p_client.username + " Disconnected\n");

			m_clients.erase(m_clients.find(p_client.id));
			shouldThreadStop = true;
		}
		else if (stringBuffer.find(": !close") != std::string::npos)
		{
			m_shouldClose = true;
		}
		else if (stringBuffer.find("New client username is : ") != std::string::npos)
		{
			char name[25];
			sscanf_s(stringBuffer.c_str(), "New client username is : %s", &name, 25);

			if (*name != NULL)
				p_client.username = name;

			std::string Welcome{ "Welcome to the server, " + p_client.username + '\n' };
			Welcome += "There are currently " + std::to_string((m_clients.size() - 1)) + " people connected with you right now. \n";
			Welcome += "users connected : ";
			for (auto& client : m_clients)
				Welcome += '\t' + client.second.username;

			Send(p_client, Welcome);
		}
		else 
		{
		    std::string realMessage = stringBuffer.substr(p_client.username.size() + 3);
			BroadcastMessage('\r' + stringBuffer, p_client.id);
		    //std::cout << buffer << std::endl;
		}
	}
}

void Server::Send(const Client& p_client, const std::string& p_message) const
{
    if (send(p_client.clientSocket, p_message.c_str(), p_message.length(), 0) < 0)
    {
        perror("send()");
    }
}

void Server::BroadcastMessage(const std::string& p_message, const size_t p_origin)
{
    for (auto& client : m_clients)
        if (client.second.id != p_origin)
            Send(client.second, p_message);
}

int Server::Close()
{
	for (auto& sockets : m_clients)
		closesocket(sockets.second.clientSocket);
	std::cout << "close listen socket" << std::endl;
	closesocket(m_sock);
	std::cout << "close socket" << std::endl;
	WSACleanup();
	return EXIT_SUCCESS;
}
