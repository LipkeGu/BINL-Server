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

Server::Server(uint16_t port, ServerType type, uint32_t ip)
{
	int retval = SOCKET_ERROR;
	uint8_t bcast = 0;

	this->servertype = type;

#ifdef _WIN32
	retval = WSAStartup(MAKEWORD(2, 2), &this->wsa);
#endif

#ifdef WITH_TFTP
	if (this->servertype != TYPE_TFTP)
	{
#endif
		this->Hostname = new char[64];
		ClearBuffer(this->Hostname, 64);
		gethostname(this->Hostname, 64);

		this->ServerIP = IP2Bytes(hostname_to_ip(this->Hostname));
		delete[] this->Hostname;
#ifdef WITH_TFTP
	}
#endif
	switch (this->servertype)
	{
	case TYPE_DHCP:
		bcast = 1;
		break;
	case TYPE_BINL:
		bcast = 1;
		break;
#ifdef WITH_TFTP
	case TYPE_TFTP:
		bcast = 0;
		break;
#endif
	}

	this->Srvsocket = new Connection(this->servertype, this->ServerIP);
	retval = this->Srvsocket->CreateUDPSocket(bcast, 1, port);
	
	if (retval != SOCKET_ERROR)
		retval = this->Srvsocket->Listen();
}

Server::~Server()
{
	delete this->Srvsocket;
}
