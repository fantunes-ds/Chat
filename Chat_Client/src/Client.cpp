#include <iostream>
#include <cassert>

#include <Client.h>

#include <WS2tcpip.h>
#include <thread>


#define PORT 76475

Client::~Client()
{
	Close();
}

void Client::Init()
{
    assert(InitLib() >= 0);
	InitSocket();
	std::cout << "Client started\n";
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
	return EXIT_SUCCESS;
}

int Client::InitSocket()
{
	m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_serverSocket == INVALID_SOCKET)
	{
		perror("socket()");
		std::cout << "INVALID_SOCKET" << std::endl;
		std::cin.get();
		return errno;
	}
	return EXIT_SUCCESS;
}

void Client::SetUsername()
{
	std::string newUsername{};
	std::cout << "Enter Username" << '\n';
	//std::getline(std::cin, m_username)
	char test[1024];
	std::getline(std::cin, newUsername);
    std::cin.getline(test, 1024);
	m_username = test;

    const std::string formattedMessage = "New client username is : " + m_username;
    if (send(m_serverSocket, formattedMessage.c_str(), formattedMessage.length(), 0) < 0)
	{
		perror("send()");
	}
	std::thread{ &Client::ReceiveMessage, this }.detach();
}

void Client::Connection(const std::string& p_address, const unsigned int p_port)
{
	struct hostent* hostinfo = NULL;

	inet_pton(AF_INET, p_address.c_str(), &m_sin.sin_addr);/* On encode l'adresse dans la variable sin.sin_addr*/
	m_sin.sin_port = htons(p_port); /* on utilise htons pour le port */
	m_sin.sin_family = AF_INET;
}

void Client::DecodeAddress() const
{
	char str[15];
	inet_ntop(AF_INET, &(m_sin.sin_addr), str, 15);
}


void Client::TryConnect()
{
	std::cout << "Default address is " << m_defaultAddress << " send 1 to use it, or write a new address" << '\n';
	std::string address;
	std::cin >> address;

	if (address == "1")
	{
		address = m_defaultAddress;
		std::cout << "using " << m_defaultAddress << " as address\n";
	}
    std::cout << "Default port is " << m_defaultPort << " send 1 to use it, or write a new port" << '\n';
	int port;
	std::cin >> port;
	if (port == 1)
	{
		port = m_defaultPort;
		std::cout << "using " << m_defaultPort << " as port\n";
	}
	Connection(address, port);

    DecodeAddress();
	while (connect(m_serverSocket, reinterpret_cast<SOCKADDR*>(&m_sin), sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		perror("connect()");
	}
    std::cout << "Connected to server " << address << ':' << port << '\n';
	m_isConnected = true;
    m_shouldThreadStop = false;

    SetUsername();
	Send();
}

void Client::Run()
{
    while(!m_shouldClose)
    {
		Send();
    }
}

void Client::Send()
{
	std::string message;
	std::string formattedMessage;

	std::cout << m_username << " : ";

	std::getline(std::cin, message);

    // we keep sending our username, although the server has that information
    // because it restricts what the user can do. He can't run commands that are only server sided.
    // since his username is always included before the message, the server won't take it as a command
    // unless we prepared it to be.
	formattedMessage = m_username + " : " +  message;
    
    if (send(m_serverSocket, formattedMessage.c_str(), formattedMessage.length(), 0) < 0)
	{
		perror("send()");
		TryConnect();
	}
}

void Client::Send(const std::string& p_message) const
{
    const std::string formattedMessage = m_username + " : " + p_message + '\n';

	send(m_serverSocket, formattedMessage.c_str(), formattedMessage.length(), 0);
}

void Client::ReceiveMessage()
{
	while (!m_shouldThreadStop)
	{
		char buffer[1024];
		int n = 0;

		if ((n = recv(m_serverSocket, buffer, sizeof buffer - 1, 0)) < 0)
		{
			std::cout << ("[ERROR] : Couldn't Receive() message from server\n");
			m_shouldThreadStop = true;
            return;
		}

		if (buffer[n - 1] != NULL)
			buffer[n] = '\0';

		std::string stringBuffer{ buffer };
		if (stringBuffer == "Disconnecting")
		{
			m_shouldClose = true;
            m_shouldThreadStop = true;
			std::cout << "Disconnected. Press enter to close the application.\n";
			return;
		}

        if (stringBuffer == "Flush")
        {
            system("cls");
        }
        else 
        {
            std::cout << buffer << std::endl;
            std::cout << m_username + " : ";
        }
    }
}

void Client::Close()
{
	if (!m_shouldThreadStop)
		m_shouldThreadStop = true;
	closesocket(m_serverSocket);
	WSACleanup();
}

