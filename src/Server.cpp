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

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Includes.h"

Server::Server(uint16_t port, ServerType type)
{
	int retval = SOCKET_ERROR;
	uint8_t bcast = 0;

	this->servertype = type;

#ifdef _WIN32
	retval = WSAStartup(MAKEWORD(2, 2), &this->wsa);
#endif

#ifdef WITH_TFTP
	if (this->servertype != TFTP)
	{
#endif
		ClearBuffer(this->Hostname, sizeof(this->Hostname));
		gethostname(this->Hostname, sizeof(this->Hostname));

		this->ServerIP = IP2Bytes(hostname_to_ip(this->Hostname));
#ifdef WITH_TFTP
	}
#endif
	switch (this->servertype)
	{
	case DHCP:
		this->Logger = new EventLog("DHCP-Server");
		bcast = 1;
		break;
	case BINL:
		this->Logger = new EventLog("BINL-Server");
		bcast = 0;
		break;
#ifdef WITH_TFTP
	case TFTP:
		this->Logger = new EventLog("TFTP-Server");
		bcast = 0;
		break;
	case HTTP:
		this->Logger = new EventLog("HTTP-Server");
		break;
#endif
	default:
		return;
	}

	this->Srvsocket = new Connection(this->Logger, this->servertype, this->ServerIP);
	retval = this->Srvsocket->CreateUDPSocket(bcast, 1, port);
	
	if (retval == SOCKET_ERROR)
		this->Logger->Report(Error, "CreateSocket(): Can not create Socket!");
	else
		retval = this->Srvsocket->Listen();
}

Server::~Server()
{
	delete this->Logger;
	delete this->Srvsocket;
}
