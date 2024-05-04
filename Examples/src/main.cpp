#include "Networking.hpp"

enum HeaderID
{
	Hello = 0, Hi
};

int main()
{
	bool OnServer = false;

	{
		std::string type;
		std::cout << "What are you?\n";
		std::cin >> type;
		if (type == "Server")
			OnServer = true;
		else
			OnServer = false;
	}


	if (OnServer)
	{
		Server s(80);

		while (s.GetConnections()->size() == 0) {}

		while (s.GetConnections()->size() > 0)
		{
			for (auto& client : *s.GetConnections())
			{
				for (auto& msg : client->GetMessages())
				{
					switch (msg.header.id)
					{
					case HeaderID::Hello:
					{
						for (auto& c : msg.body)
							std::cout << c;
						std::cout << std::endl;
						Message returnMsg;
						returnMsg.header.id = HeaderID::Hi;
						for (int i = msg.body.size() - 1; i > -1; i--)
							returnMsg.body.push_back(msg.body[i]);
						returnMsg.header.size = returnMsg.body.size();
						client->Send(returnMsg);
					}
					default:
					{
					}
					}
				}
			}
		}
	}
	else
	{
		Client c("10.0.0.230", 80);

		while (!c.IsConnected()) {}

		Message initialMsg;
		initialMsg.header.id = HeaderID::Hello;
		char m[] = "Hello";
		initialMsg.body.resize(5);
		std::memcpy(initialMsg.body.data(), m, 5);
		initialMsg.header.size = initialMsg.body.size();
		c.GetConnection()->Send(initialMsg);

		while (c.IsConnected())
		{
			for (auto& msg : c.GetConnection()->GetMessages())
			{
				switch (msg.header.id)
				{
				case HeaderID::Hi:
				{
					for (auto& c : msg.body)
						std::cout << c;
					std::cout << std::endl;
					Message returnMsg;
					returnMsg.header.id = HeaderID::Hello;
					for (int i = msg.body.size() - 1; i > -1; i--)
						returnMsg.body.push_back(msg.body[i]);
					returnMsg.header.size = returnMsg.body.size();
					c.GetConnection()->Send(returnMsg);
				}
				}
			}
		}
	}

	return 0;
}