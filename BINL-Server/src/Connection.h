/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Includes.h"

class Connection
{
public:
	Connection(ServerType* type, uint32_t IPAddress);
	~Connection();

	int CreateUDPSocket(int Broadcast, int ReUseAddr, uint16_t port);
	int Send(Packet* packet);
	int Listen();
	void Handle_DHCP_Request(Client* client, Packet* packet, ServerType* type);
	std::string ResolvePath(std::string& path);
	void Handle_Request(ServerType* type, sockaddr_in& remote, const char* buffer, size_t length);
	Client* Get_Client(ServerType* type, sockaddr_in& remote, Packet* packet);
	void Connection::RemoveClient(std::string& id);
	void Handle_RRQ_Request(Client* client, Packet* packet, ServerType* type);
	void Handle_ACK_Request(Client* client, Packet* packet, ServerType* type);
	void Handle_ERR_Request(Client* client, Packet* packet, ServerType* type);

private:
	uint32_t ServerIP;

	int SocketState;
	ServerType* serverType;

	int AddressFamily;
	int SocketType;
	int Protocol;
	
	SOCKET Conn;
	struct sockaddr_in listener;
	struct sockaddr_in remote;
	
	std::map<std::string, std::unique_ptr<Client>>* clients;
};
