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

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Includes.h"

Connection::Connection(ServerType type, uint32_t IPAddress)
{
	this->serverType = type;
	this->remotesocklen = sizeof(this->remote);
	this->clients = new std::map<std::string, Client*>();
	this->ServerIP = IPAddress;
	this->requestID = 1;
	this->AddressFamily = AF_INET;
	this->Protocol = IPPROTO_UDP;
	this->SocketType = SOCK_DGRAM;
	this->Conn = SOCKET_ERROR;
	this->SocketState = SOCKET_ERROR;
}

int Connection::CreateUDPSocket(int Broadcast, int ReUseAddr, uint16_t port)
{
	this->Conn = socket(this->AddressFamily, this->SocketType, this->Protocol);

	if (this->Conn != SOCKET_ERROR)
	{
		if (this->serverType == TYPE_DHCP)
		{
			this->SocketState = setsockopt(this->Conn, SOL_SOCKET,
				SO_REUSEADDR, (const char*)&ReUseAddr, sizeof(ReUseAddr));

			if (this->SocketState == SOCKET_ERROR)
				return this->SocketState;

			this->SocketState = setsockopt(this->Conn, SOL_SOCKET,
				SO_BROADCAST, (const char*)&Broadcast, sizeof(Broadcast));

			if (this->SocketState == SOCKET_ERROR)
				return this->SocketState;
		}

		ClearBuffer(&this->listener, sizeof(this->listener));
		ClearBuffer(&this->remote, sizeof(this->remote));

		this->listener.sin_family = AF_INET;
		this->listener.sin_addr.s_addr = INADDR_ANY;
		this->listener.sin_port = htons(port);

		this->SocketState = bind(this->Conn, (struct sockaddr*)
			&this->listener, sizeof(this->listener));

		return this->SocketState;
	}
	else
		return SOCKET_ERROR;
}

int Connection::Listen()
{
	char buffer[16384];
	while (this->SocketState != SOCKET_ERROR)
	{
		ClearBuffer(buffer, sizeof buffer);

		this->remotesocklen = sizeof(this->remote);

		this->SocketState = recvfrom(this->Conn, buffer, sizeof buffer, 0, (struct sockaddr *)&this->remote, &this->remotesocklen);
		if (this->SocketState == SOCKET_ERROR)
			continue;

		this->Handle_Request(this->serverType, remote, buffer, this->SocketState);
	}

	return this->SocketState;
}

Client* Connection::Get_Client(ServerType type, sockaddr_in& remote, Packet* packet)
{
	auto addr = new std::string(inet_ntoa(this->remote.sin_addr));
	if (clients->find(*addr) == clients->end())
	{
		if (this->serverType != TYPE_TFTP)
		{
			auto hwaddr = new char[packet->GetBuffer()[BOOTP_OFFSET_MACLEN] + 12];
			ClearBuffer(hwaddr, packet->GetBuffer()[BOOTP_OFFSET_MACLEN] + 12);

			uint8_t hwadr[6];

			memcpy(&hwadr, &packet->GetBuffer()[BOOTP_OFFSET_MACADDR],
				packet->GetBuffer()[BOOTP_OFFSET_MACLEN]);

			sprintf(hwaddr, "%02X:%02X:%02X:%02X:%02X:%02X", hwadr[0],
				hwadr[1], hwadr[2], hwadr[3], hwadr[4], hwadr[5]);

			clients->insert(std::pair<std::string, Client*>(*addr, new Client(this->serverType, addr, hwaddr)));
			delete[] hwaddr;
		}
		else
			clients->insert(std::pair<std::string, Client*>(*addr, new Client(this->serverType, addr, NULL)));
	}

	return clients->find(*addr)->second;
}

void Connection::Handle_Request(ServerType type, sockaddr_in& remote, const char* buffer, size_t length)
{
	auto packet = new Packet(this->serverType, length);
	memcpy(&packet->GetBuffer()[0], buffer, length);

	auto c = this->Get_Client(this->serverType, remote, packet);

	switch (GetClientType(c, packet, this->serverType))
	{
	case TYPE_DHCP:
	case TYPE_BINL:
		if (FindVendorOpt(packet->GetBuffer(), packet->GetLength(), DHCP_VENDORCLASS_PXE))
		{
			packet->Parse();
			c->DHCP_CLIENT.NextServer = this->ServerIP;
			this->Handle_DHCP_Request(c, packet, this->serverType);
		}
		break;
	case TYPE_TFTP:
		switch (packet->GetBuffer()[1])
		{
		case TFTP_RRQ:
			this->Handle_RRQ_Request(c, packet, this->serverType);
			break;
		case TFTP_ACK:
			this->Handle_ACK_Request(c, packet, this->serverType);
			break;
		case TFTP_ERR:
			this->Handle_ERR_Request(c, packet, this->serverType);

			break;
		default:
			c->TFTP_CLIENT.SetTFTPState(TFTP_Done);
			break;
		}
		break;
	default:
		break;
	}
	clients->erase(*c->id);
	delete packet;
}

int Connection::Send(Client* client)
{
	auto retval = SOCKET_ERROR;

	if (this->serverType == TYPE_DHCP)
	{
		struct sockaddr_in toAddr;
		ClearBuffer(&toAddr, sizeof(toAddr));

		toAddr.sin_family = AF_INET;
		toAddr.sin_addr.s_addr = INADDR_BROADCAST;
		toAddr.sin_port = htons(68);

		retval = sendto(this->Conn, client->Data->GetBuffer(),
			(int)client->Data->GetPosition(), 0, (struct sockaddr *)&toAddr, sizeof(toAddr));
	}
	else
		retval = sendto(this->Conn, client->Data->GetBuffer(), (int)client->Data->GetPosition(), 0, (struct sockaddr *)&this->remote, sizeof(this->remote));

	return retval;
}

Connection::~Connection()
{
	this->clients->clear();
	delete this->clients;
#ifdef _WIN32
	WSACleanup();
#endif
}

#ifdef WITH_TFTP
void Connection::Handle_ERR_Request(Client* client, Packet* packet, ServerType type)
{
	if (packet->GetBuffer()[3] == 0)
		return;

	client->TFTP_CLIENT.SetTFTPState(TFTP_Error);
	auto errmsg = new char[packet->GetLength() - 4];
	ClearBuffer(errmsg, packet->GetLength() - 4);

	strncpy(errmsg, &packet->GetBuffer()[4], packet->GetLength() - 4);

	delete[] errmsg;
}

void Connection::Handle_ACK_Request(Client* client, Packet* packet, ServerType type)
{
	if (client->TFTP_CLIENT.GetTFTPState() != TFTP_Download)
	{
		client->TFTP_CLIENT.SetTFTPState(TFTP_Error);
		return;
	}

	for (auto i = 0; i < client->TFTP_CLIENT.GetWindowSize(); i++)
	{
		auto chunk = (client->TFTP_CLIENT.GetBlockSize() < (client->TFTP_CLIENT.GetBytesToRead() - client->TFTP_CLIENT.GetBytesRead())) ?
			(size_t)client->TFTP_CLIENT.GetBlockSize() : (size_t)(client->TFTP_CLIENT.GetBytesToRead() - client->TFTP_CLIENT.GetBytesRead());

		client->Data = new Packet(type, 4 + chunk + 1);
		client->TFTP_CLIENT.SetBlock();

		client->Data->TFTP_Data(client->TFTP_CLIENT.GetBlock());

		client->TFTP_CLIENT.SetBytesRead(static_cast<long>(client->file->Read(client->Data->GetBuffer(),
			static_cast<long>(client->Data->GetPosition()), client->TFTP_CLIENT.GetBytesRead(), chunk)));

		client->Data->SetPosition(chunk);

		this->SocketState = this->Send(client);
		delete client->Data;

		if (client->TFTP_CLIENT.GetBytesRead() == client->TFTP_CLIENT.GetBytesToRead())
			client->TFTP_CLIENT.SetTFTPState(TFTP_Done);
	}


	if (client->TFTP_CLIENT.GetTFTPState() == TFTP_Done || client->TFTP_CLIENT.GetTFTPState() == TFTP_Error)
	{
		delete client->file;
		this->clients->erase(*client->id);
	}
}

void Connection::Handle_RRQ_Request(Client* client, Packet* packet, ServerType type)
{
	auto filename = new char[255];
	ClearBuffer(filename, 255);

	extString(&packet->GetBuffer()[2], 255, filename);
	client->file = new FileSystem(filename, FileReadBinary);

	delete[] filename;

	client->TFTP_CLIENT.SetBlock(0);

	if (client->file->Exist())
	{
		if (packet->TFTP_HasOption("octet"))
		{
			if (packet->TFTP_HasOption("blksize"))
			{
				char* blksize = new char[6];

				ClearBuffer(blksize, 6);
				extString(&packet->GetBuffer()[packet->TFTP_OptionOffset("blksize")], 6, blksize);

				client->TFTP_CLIENT.SetBlockSize(atoi(blksize));
				delete[] blksize;
			}
			else
				client->TFTP_CLIENT.SetBlockSize(1024);

			if (packet->TFTP_HasOption("windowsize"))
			{
				char* winsize = new char[6];

				ClearBuffer(winsize, 6);
				extString(&packet->GetBuffer()[packet->TFTP_OptionOffset("windowsize")], 6, winsize);

				client->TFTP_CLIENT.SetWindowSize(atoi(winsize));
				delete[] winsize;

				if (packet->TFTP_HasOption("msftwindow"))
				{
					char* msftwin = new char[6];
					ClearBuffer(msftwin, 6);

					extString(&packet->GetBuffer()[packet->TFTP_OptionOffset("msftwindow")], 6, msftwin);

					client->TFTP_CLIENT.SetMSFTWindow(27182);
					delete[] msftwin;
				}
				else
					client->TFTP_CLIENT.SetMSFTWindow(0);
			}
			else
				client->TFTP_CLIENT.SetWindowSize(1);

			client->TFTP_CLIENT.SetBytesToRead(client->file->Length());
			client->TFTP_CLIENT.SetBytesRead(0);

			client->Data = new Packet(type, 4 + client->TFTP_CLIENT.GetBlockSize());
			client->Data->TFTP_OptAck(client->TFTP_CLIENT.GetBlockSize(), client->TFTP_CLIENT.GetBytesToRead(), \
				client->TFTP_CLIENT.GetWindowSize(), client->TFTP_CLIENT.GetMSFTWindow());

			client->TFTP_CLIENT.SetTFTPState(TFTP_Download);

			this->SocketState = this->Send(client);

			delete client->Data;
		}
		else
		{
			auto msg = new std::string("The value for option \"mode\" is not supported!");

			client->Data = new Packet(type, 4 + msg->size() + 2);
			client->Data->TFTP_Error(8, msg);
			client->TFTP_CLIENT.SetTFTPState(TFTP_Error);

			this->SocketState = this->Send(client);

			delete client->Data;
			delete client->file;
			delete msg;
		}
	}
	else
	{
		auto filename = new std::string(client->file->Name());
		client->Data = new Packet(type, 4 + filename->size() + 2);
		client->Data->TFTP_Error(1, filename);
		client->TFTP_CLIENT.SetTFTPState(TFTP_Error);

		this->SocketState = this->Send(client);

		delete filename;
		delete client->Data;
		delete client->file;
	}
}
#endif

void Connection::Handle_DHCP_Request(Client* client, Packet* packet, ServerType type)
{
	client->Data = new Packet(type, 1260);

	/* BOOTP Type */
	client->Data->GetBuffer()[client->Data->GetPosition()] = 0x02;
	client->Data->SetPosition(1);

	/* Hardware Type (Copy also the additional 8 Bytes in one run) */
	client->Data->Write(&packet->GetBuffer()[BOOTP_OFFSET_HWTYPE], 9);

	/* BOOTP Flags */
	client->Data->Write(&packet->GetBuffer()[BOOTP_OFFSET_BOOTPFLAGS], sizeof(uint16_t));

	/* Client IP */
	client->Data->Write(&packet->GetBuffer()[BOOTP_OFFSET_CLIENTIP], sizeof(uint32_t));

	/* Your IP */
	client->Data->Write(&packet->GetBuffer()[BOOTP_OFFSET_YOURIP], sizeof(uint32_t));

	/* Next Server */
	client->Data->Write(&client->DHCP_CLIENT.NextServer, sizeof(uint32_t));

	/* Relay Agent */
	client->Data->Write(&packet->GetBuffer()[BOOTP_OFFSET_RELAYIP], sizeof(uint32_t));

	/* Client HW Address */
	client->Data->Write(&packet->GetBuffer()[BOOTP_OFFSET_MACADDR], 6);

	/* MAC Padding */
	client->Data->Write(&packet->GetBuffer()[BOOTP_OFFSET_MACPADDING], 10);

	/* Server name */
	char*hname = new char[64];
	ClearBuffer(hname, 64);
	gethostname(hname, 64);
	client->Data->Write(hname, 64);
	client->Data->SetPosition(128);
	delete[] hname;

	/* Client Boot file */
	ClearBuffer(&client->Data->GetBuffer()[108], 128);
	sprintf(&client->Data->GetBuffer()[108], "%s", \
		client->DHCP_CLIENT.GetBootfile(INTEL_X86).c_str());

	/* MAGIC COOKIE */
	client->Data->Write(&packet->GetBuffer()[BOOTP_OFFSET_COOKIE], sizeof(uint32_t));

	/* DHCP Message Type */
	client->Data->Add_DHCPOption(DHCP_Option(53, static_cast<unsigned char>
		(GetDHCPMessageType(client, packet, type))));

	/* DHCP Server Id */
	client->Data->Add_DHCPOption(DHCP_Option(54, this->ServerIP));


	/* DHCP Vendor Class */
	client->Data->Add_DHCPOption(DHCP_Option(60, "PXEClient"));

	/* DHCP Client GUID */
	client->Data->Add_DHCPOption(packet->Get_DHCPOption(97));

	//BuildWDSOptions(client, type);

	if (client->isWDSRequest)
		client->DHCP_CLIENT.SetArchitecture((CLIENT_ARCH)packet->GetBuffer()[289]);

	/* BCD Path (Microsoft) */
	if (client->GetBCDfile().size() != 0 && this->serverType == TYPE_BINL)
		client->Data->Add_DHCPOption(DHCP_Option(252, client->GetBCDfile()));

	/* DHCP Option "end of Packet" */
	client->Data->Commit();

	if (!client->isWDSRequest)
	{
		client->DHCP_CLIENT.WDS.SetRequestID(this->requestID);
		this->SocketState = this->Send(client);
	}
	else
	{
		if (client->DHCP_CLIENT.WDS.ActionDone == 1)
		{
			this->SocketState = this->Send(client);
			this->requestID += 1;
		}
	}

	clients->erase(*client->id);
	delete client->Data;
}
