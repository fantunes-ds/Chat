#include <iostream>
#include <cassert>
#include <thread>

#include <Server.h>

#define PORT 8765

Server::~Server()
{
	Close();
}

void Server::Init()
{
	assert(InitLib() >= 0);
	InitSocket();

	if (Bind() == EXIT_FAILURE)
	{
        std::cout << "Bind() failed\n";
        return;
	}
    std::cout << "Server initialized\n";
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

void Server::Run()
{
    if (Listen() == EXIT_FAILURE)
    {
        std::cout << "Listen() failed\n";
        return;
    }
    std::cout << "Server running\n";

    std::thread threadAccept{ &Server::AcceptNewClient, this };
	threadAccept.detach();

    while(!m_shouldClose)
    {
    }
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

int Server::AcceptNewClient()
{
	Client newClient;
	SOCKADDR_IN clientSocketIn = { 0 };
	int clientSocketInSize = sizeof clientSocketIn;

	newClient.clientSocket = accept(m_sock, reinterpret_cast<SOCKADDR*>(&clientSocketIn), &clientSocketInSize);

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

	Client& clientRef = m_clients.find(m_clients.size() - 1)->second;
    clientRef.id = (m_clients.size() - 1) + placement;

    std::thread { &Server::AcceptNewClient, this }.detach();
	std::thread{ &Server::ReceiveMessageFromClient, this, std::ref(clientRef) }.detach();
    std::cout << "New user logged in\n";
    return EXIT_SUCCESS;
}

void Server::ReceiveMessageFromClient(Client& p_client)
{
	bool shouldThreadStop{ false };
	while (!shouldThreadStop)
	{
        std::string stringBuffer = CatchMessageFromClient(p_client);
        if (stringBuffer == "ABORT")
        {
            shouldThreadStop = true;
            DisconnectClient(p_client);
        }
        if (CheckForExceptions(stringBuffer, p_client) == EXIT_FAILURE)
            shouldThreadStop = true;
        else 
            BroadcastMessage("\n\r" + stringBuffer, p_client.id);
        
	}
}

std::string Server::CatchMessageFromClient(const Client& p_client)
{
    char buffer[1024]{ '\0' };
    int n = 0;

    if ((n = recv(p_client.clientSocket, buffer, sizeof buffer - 1, 0)) < 0)
        return{"ABORT"};

    if (buffer[0] != -52 && buffer[n - 1] != NULL)
        buffer[n] = '\0';

    return { buffer };
}

void Server::DisconnectClient(const Client& p_client)
{
    closesocket(p_client.clientSocket);
    std::cout << p_client.username + " Disconnected\n";

    m_clients.erase(m_clients.find(p_client.id));
}

int Server::CheckForExceptions(std::string& p_stringBuffer, Client& p_client)
{
    if (p_stringBuffer.find(": !Quit") != std::string::npos)
    {
        Send(p_client, "Disconnecting");
        BroadcastMessage('\r' + p_client.username + " has left the server\n", p_client.id);
        DisconnectClient(p_client);
        return EXIT_FAILURE;
    }
    else if (p_stringBuffer.find(": !CloseRemote") != std::string::npos)
    {
        m_shouldClose = true;
    }
    else if (p_stringBuffer.find("New client username is : ") != std::string::npos)
    {
        char name[25];
        sscanf_s(p_stringBuffer.c_str(), "New client username is : %s", &name, 25);

        if (*name != NULL)
            p_client.username = name;
        
        std::cout << "User " << p_client.username << " joined the chat\n";

        Send(p_client, "Flush");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::string WelcomeMessage{ "\rWelcome to the server, " + p_client.username + '\n' };
        WelcomeMessage += "There are currently " + std::to_string((m_clients.size() - 1)) + " users connected with you right now. \n";
        WelcomeMessage += "users connected : ";
        for (auto& client : m_clients)
            WelcomeMessage += '\t' + client.second.username;

        WelcomeMessage += "\nWrite '!Quit' to quit anytime\n";

        Send(p_client, WelcomeMessage);
        BroadcastMessage('\n' + p_client.username + " has joined the chat\n", p_client.id);
    }
    return EXIT_SUCCESS;
}

void Server::Send(const Client& p_client, const std::string& p_message) const
{
    if (send(p_client.clientSocket, p_message.c_str(), p_message.length(), 0) < 0)
    {
        std::cout << "Message " << '\'' + p_message + '\'' << " couldn't been sent to" << p_client.username << std::endl;
    }
}

void Server::BroadcastMessage(const std::string& p_message, const size_t p_originID)
{
    for (auto& client : m_clients)
        if (client.second.id != p_originID)
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
