#include <iostream>
#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

struct MessageHeader
{
	uint32_t id;
	uint32_t size;
};

struct Message
{
	Message(MessageHeader h = {}, std::vector<char> b = {})
		:header(h), body(b) {}
	MessageHeader header;
	std::vector<char> body;
};

class Connection
{
public:
	Connection(asio::ip::tcp::socket socket);
	
	void Send(const Message& msg);

	std::vector<Message> GetMessages();

	bool IsOpen();
private:
	void ReadHeader();
	
	void ReadBody();
 
private:
	
	asio::ip::tcp::socket m_Socket;

	bool inLock = false;
	bool outLock = false;

	std::vector<Message> m_MessagesIn;
	Message tempMsg;
};


class Server
{
public:
	Server(uint32_t port);

	~Server();

	void StartAccept();

	std::vector<Connection*>* GetConnections();
private:
	asio::io_context m_Context;
	asio::ip::tcp::acceptor m_Acceptor;
	std::vector<Connection*> m_Connections;
	bool acceptLock = false;
	std::thread m_Thread;
};


class Client
{
public:
	Client(const std::string& host, uint32_t port);

	~Client();

	Connection* GetConnection();

	bool IsConnected();
private:
	asio::io_context m_Context;
	Connection* m_Connection;
	std::thread m_Thread;
	bool m_Connected = false;
};