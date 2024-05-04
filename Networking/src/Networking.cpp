#include "Networking.hpp"

Connection::Connection(asio::ip::tcp::socket socket)
	: m_Socket(std::move(socket))
{
	ReadHeader();
}

void Connection::Send(const Message& msg)
{
	while (outLock) {}
	outLock = true;
	asio::async_write(m_Socket, asio::buffer(&msg.header, sizeof(MessageHeader)), [this](asio::error_code ec, size_t length)
		{
			if (ec)
				std::cout << "Write Header Error: " << ec.message() << std::endl;
		});
	asio::async_write(m_Socket, asio::buffer(msg.body.data(), msg.header.size), [this](asio::error_code ec, size_t length)
		{
			if (!ec)
				outLock = false;
			else
				std::cout << "Write Body Error: " << ec.message() << std::endl;
		});
}

std::vector<Message> Connection::GetMessages()
{
	while (inLock) {}
	if (m_MessagesIn.size() == 0)
		return std::vector<Message>();
	inLock = true;
	std::vector<Message> messages;
	messages.resize(m_MessagesIn.size());
	for (int i = 0; i < m_MessagesIn.size(); i++)
		messages[i] = m_MessagesIn[i];
	m_MessagesIn.clear();
	inLock = false;
	return messages;
}

bool Connection::IsOpen()
{
	return m_Socket.is_open();
}

void Connection::ReadHeader()
{
	asio::async_read(m_Socket, asio::buffer(&tempMsg.header, sizeof(MessageHeader)), [this](asio::error_code ec, size_t length)
		{
			if (!ec)
			{
				while (inLock) {}
				inLock = true;
				tempMsg.body.clear();
				tempMsg.body.resize(tempMsg.header.size);
				if (tempMsg.header.size)
				{
					ReadBody();
				}
				else
				{
					m_MessagesIn.push_back(tempMsg);
					inLock = false;
					ReadHeader();
				}
			}
			else
			{
				std::cout << "Read Header Error: " << ec.message() << std::endl;
			}
		});
}

void Connection::ReadBody()
{
	asio::async_read(m_Socket, asio::buffer(tempMsg.body.data(), tempMsg.header.size), [this](asio::error_code ec, size_t length)
		{
			if (!ec)
			{
				m_MessagesIn.push_back(tempMsg);
				inLock = false;
				ReadHeader();
			}
			else
			{
				std::cout << "Read Body Error: " << ec.message() << std::endl;
			}
		});
}

Server::Server(uint32_t port)
	: m_Acceptor(m_Context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{
	try
	{
		StartAccept();
		m_Thread = std::thread([this]() { m_Context.run(); });
	}
	catch (std::exception& e)
	{
		std::cout << "[Server] exception: " << e.what() << std::endl;
	}
}

Server::~Server()
{
	m_Context.stop();
	if (m_Thread.joinable())
		m_Thread.join();
}

void Server::StartAccept()
{
	m_Acceptor.async_accept([this](asio::error_code ec, asio::ip::tcp::socket socket)
		{
			if (!ec)
			{
				while (acceptLock) {}
				acceptLock = true;
				std::cout << "Client connected: " << socket.remote_endpoint().address().to_string();
				m_Connections.push_back(new Connection(std::move(socket)));
				acceptLock = false;
			}
			else
			{
				std::cout << "Error accepting connection: " << ec.message() << std::endl;
			}
			StartAccept();
		});
}

std::vector<Connection*>* Server::GetConnections()
{
	std::vector<uint32_t> deleteThese;
	for (int i = 0; i < m_Connections.size(); i++)
	{
		if (!m_Connections[i]->IsOpen())
			deleteThese.push_back(i);
	}
	for (auto& i : deleteThese)
		m_Connections.erase(m_Connections.begin() + i);

	while (acceptLock) {}
	return &m_Connections;
}

Client::Client(const std::string& host, uint32_t port)
{
	asio::ip::tcp::socket socket(m_Context);
	asio::ip::tcp::resolver resolver(m_Context);
	asio::async_connect(socket, resolver.resolve(host, std::to_string(port)), [this](asio::error_code ec, asio::ip::tcp::endpoint endpoint)
		{
			if (!ec)
			{
				std::cout << "Connected" << std::endl;
				m_Connected = true;
			}
			else
			{
				std::cout << "Failed Connection: " << ec.message() << std::endl;
			}
		});
	m_Connection = new Connection(std::move(socket));
	m_Thread = std::thread([this]() { m_Context.run(); });
}

Client::~Client()
{
	m_Context.stop();
	if (m_Thread.joinable())
		m_Thread.join();

	delete m_Connection;
}

Connection* Client::GetConnection()
{
	while (!m_Connected) {}
	return m_Connection;
}

bool Client::IsConnected()
{
	return m_Connected;
}