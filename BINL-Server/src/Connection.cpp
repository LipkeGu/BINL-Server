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

using namespace std;

Connection::Connection(EventLog *pLogger, ServerType type, uint32_t IPAddress)
{
	this->Logger = pLogger;
	this->serverType = type;
	this->remotesocklen = sizeof(this->remote);
	this->clients = new map<string, Client*>();
	this->ServerIP = IPAddress;
	this->requestID = htonl(1);
	this->AddressFamily = AF_INET;
	this->Protocol = (type != HTTP) ? IPPROTO_UDP : IPPROTO_TCP;
	this->SocketType = (type != HTTP) ? SOCK_DGRAM : SOCK_STREAM;
	this->Conn = SOCKET_ERROR;
	this->SocketState = SOCKET_ERROR;
}

int Connection::CreateUDPSocket(int Broadcast, int ReUseAddr, uint16_t port)
{
	this->Conn = socket(this->AddressFamily, this->SocketType, this->Protocol);

	if (this->Conn != SOCKET_ERROR)
	{
		if (this->serverType == DHCP)
		{
			this->SocketState = setsockopt(this->Conn, SOL_SOCKET,
				SO_REUSEADDR, (const char*)&ReUseAddr, sizeof(ReUseAddr));

			if (this->SocketState == SOCKET_ERROR)
			{
				this->Logger->LogBuffer = new char[1024];
				ClearBuffer(this->Logger->LogBuffer, 1024);
				
				sprintf(this->Logger->LogBuffer, "Failed to set SO_REUSEADDR on socket: %s", strerror(errno));
				
				this->Logger->Report(Error, this->Logger->LogBuffer);
				delete this->Logger->LogBuffer;

				return this->SocketState;
			}

			this->SocketState = setsockopt(this->Conn, SOL_SOCKET,
				SO_BROADCAST, (const char*)&Broadcast, sizeof(Broadcast));

			if (this->SocketState == SOCKET_ERROR)
			{

				this->Logger->LogBuffer = new char[1024];
				ClearBuffer(this->Logger->LogBuffer, 1024);
				
				sprintf(this->Logger->LogBuffer, "Failed to set SO_BROADCAST on socket: %s", strerror(errno));
				
				this->Logger->Report(Error, this->Logger->LogBuffer);
				delete this->Logger->LogBuffer;

				return this->SocketState;
			}
		}

		if (this->serverType == HTTP)
		{
			int keepalive = 1;
			
			this->SocketState = setsockopt(this->Conn, SOL_SOCKET,
				SO_KEEPALIVE, (const char*)&keepalive, sizeof(keepalive));
			
			if (this->SocketState == SOCKET_ERROR)
			{
				this->Logger->LogBuffer = new char[1024];
				ClearBuffer(this->Logger->LogBuffer, 1024);
				
				sprintf(this->Logger->LogBuffer, "Failed to set SO_KEEPALIVE on socket: %s", strerror(errno));
				
				this->Logger->Report(Error, this->Logger->LogBuffer);
				delete this->Logger->LogBuffer;

				return this->SocketState;
			}
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
	Packet* packet;
	SOCKET* s;

	while (this->SocketState != SOCKET_ERROR)
	{
		packet = new Packet(this->Logger, this->serverType, 2048);

		if (this->serverType != HTTP)
			this->SocketState = recvfrom(this->Conn, packet->GetBuffer(), (int)packet->GetLength(),
				0, (struct sockaddr *)&this->remote, &this->remotesocklen);
		else
		{
			listen(this->Conn, 5);
			s = new SOCKET(accept(this->Conn, (struct sockaddr *) &this->remote, &this->remotesocklen));
			if (!s)
			{
				close(*s);
				delete s;
				continue;
			}
#ifdef _WIN32
			this->SocketState = read(*s, packet->GetBuffer(), (int)packet->GetLength(), 0);
#else
			this->SocketState = read(*s, packet->GetBuffer(), packet->GetLength());
#endif
		}

		if (this->SocketState == SOCKET_ERROR || this->SocketState == 0)
		{
			if (this->serverType == HTTP)
			{
				close(*s);
				delete s;
			}

			delete packet;
			continue;
		}
		else
		{
			string addr = inet_ntoa(this->remote.sin_addr);

			packet->SetLength(this->SocketState);
			tmpClient = clients->find(addr);

			if (tmpClient == clients->end())
			{
				char* hwaddr;
#ifdef WITH_TFTP
				if (this->serverType != TFTP && this->serverType != HTTP)
				{
#endif
					hwaddr = new char[packet->GetBuffer()[BOOTP_OFFSET_MACLEN] + 12];
					ClearBuffer(hwaddr, packet->GetBuffer()[BOOTP_OFFSET_MACLEN] + 12);

					uint8_t hwadr[6];

					memcpy(&hwadr, &packet->GetBuffer()[BOOTP_OFFSET_MACADDR],
						packet->GetBuffer()[BOOTP_OFFSET_MACLEN]);
					
					sprintf(hwaddr, "%02X:%02X:%02X:%02X:%02X:%02X", hwadr[0],
						hwadr[1], hwadr[2], hwadr[3], hwadr[4], hwadr[5]);
#ifdef WITH_TFTP
				}
				else
					hwaddr = new char[1]{ 0x00 };
#endif
				clients->insert(pair<string, Client*>(addr, new Client(
					this->Logger, this->serverType, addr, hwaddr)));

				if (this->serverType == HTTP)
					clients->find(addr)->second->SetSocket(s);

				delete[] hwaddr;
			}

			switch (GetClientType(clients->find(addr)->second, packet, this->serverType))
			{
			case DHCP:
			case BINL:
				if (FindVendorOpt(packet->GetBuffer(), packet->GetLength(), DHCP_VENDORCLASS_PXE))
				{
					clients->find(addr)->second->NextServer = this->ServerIP;
					this->Handle_DHCP_Request(clients->find(addr)->second, packet, this->serverType);
				}
				break;
#ifdef WITH_TFTP
			case TFTP:
				switch (packet->GetBuffer()[1])
				{
				case TFTP_RRQ:
					this->Handle_RRQ_Request(clients->find(addr)->second, packet, this->serverType);
					break;
				case TFTP_ACK:
					this->Handle_ACK_Request(clients->find(addr)->second, packet, this->serverType);
					break;
				case TFTP_ERR:
					this->Handle_ERR_Request(clients->find(addr)->second, packet, this->serverType);
					clients->erase(addr);
					break;
				default:
					clients->find(addr)->second->SetTFTPState(TFTP_Done);
					break;
				}
				break;
#endif
			case HTTP:
				this->Handle_HTTP_Request(clients->find(addr)->second, packet, this->serverType);
				clients->erase(addr);
				break;
			default:
				break;
			}

			delete packet;
			addr.clear();
		}
	}
	
	return this->SocketState;
}

int Connection::Send(Client* client)
{
	if (this->serverType == DHCP)
	{
		struct sockaddr_in toAddr;
		ClearBuffer(&toAddr, sizeof(toAddr));

		toAddr.sin_family = AF_INET;
		toAddr.sin_addr.s_addr = INADDR_BROADCAST;
		toAddr.sin_port = htons(68);

		this->SocketState = sendto(this->Conn, client->Data->GetBuffer(), (int)client->Data->GetPosition(), 0,
			(struct sockaddr *)&toAddr, sizeof(toAddr));
	}
	else
		if (this->serverType != HTTP)
		{
			this->SocketState = sendto(this->Conn, client->Data->GetBuffer(), (int)client->Data->GetPosition(), 0,
				(struct sockaddr *)&this->remote, sizeof(this->remote));
		}
		else
		{
			size_t bs = 0;

			do
			{
				bs += send(*client->GetSocket(), &client->Data->GetBuffer()[bs], (int)client->Data->GetPosition() - bs, 0);
			} while (bs < client->Data->GetPosition());
		}

	return this->SocketState;
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
	if (packet->GetBuffer()[3] != 0)
	{
		client->SetTFTPState(TFTP_Error);
		char* errmsg = new char[packet->GetLength() - 4];
		ClearBuffer(errmsg, packet->GetLength() - 4);

		strncpy(errmsg, &packet->GetBuffer()[4], packet->GetLength() - 4);
		this->Logger->Report(Error, "[Client (" + client->id + ")]: " + string(errmsg));

		delete[] errmsg;
	}
}

void Connection::Handle_ACK_Request(Client* client, Packet* packet, ServerType type)
{
	if (!client->GetACK(packet))
	{
		if (client->retries == 2)
			client->SetTFTPState(TFTP_Error);
		else
		{
			client->SetBytesRead(client->last_bytesRead);
			client->SetBlock(client->last_block);

			client->retries++;
		}
	}
	else
	{
		if (client->GetTFTPState() == TFTP_Download)
		{
			client->retries = 0;
			size_t chunk = 0;
			uint16_t i = 0;

			for (i = 0; i < client->GetWindowSize(); i++)
			{
				chunk = (client->GetBlockSize() < (client->GetBytesToRead() - client->GetBytesRead())) ?
					(size_t)client->GetBlockSize() : (size_t)(client->GetBytesToRead() - client->GetBytesRead());

				client->Data = new Packet(this->Logger, type, 4 + chunk + 1);

				client->last_block = client->GetBlock();
				client->SetBlock();

				client->Data->TFTP_Data(client->GetBlock());
				client->last_bytesRead = client->GetBytesRead();

				client->SetBytesRead(client->file->Read(client->Data->GetBuffer(),
					client->Data->GetPosition(), client->GetBytesRead(), chunk));

				client->Data->SetPosition(chunk);

				this->SocketState = this->Send(client);
				delete client->Data;

				if (client->GetBytesRead() == client->GetBytesToRead())
					client->SetTFTPState(TFTP_Done);
			}
		}
		else
			client->SetTFTPState(TFTP_Error);
	}

	if (client->GetTFTPState() == TFTP_Done || client->GetTFTPState() == TFTP_Error)
	{
		delete client->file;
		this->clients->erase(client->id);
	}
}

void Connection::Handle_RRQ_Request(Client* client, Packet* packet, ServerType type)
{
	char* filename = new char[255];
	ClearBuffer(filename, 255);

	extString(&packet->GetBuffer()[2], 255, filename);
	client->file = new FileSystem(string(filename), FileReadBinary);

	delete[] filename;

	client->SetBlock(0);

	if (client->file->Exist())
	{
		if (packet->TFTP_HasOption("octet"))
		{
			if (packet->TFTP_HasOption("blksize"))
			{
				char* blksize = new char[6];

				ClearBuffer(blksize, 6);
				extString(&packet->GetBuffer()[packet->TFTP_OptionOffset("blksize")], 6, blksize);

				client->SetBlockSize(atoi(blksize));
				delete[] blksize;
			}
			else
				client->SetBlockSize(1024);

			if (packet->TFTP_HasOption("windowsize"))
			{
				char* winsize = new char[6];

				ClearBuffer(winsize, 6);
				extString(&packet->GetBuffer()[packet->TFTP_OptionOffset("windowsize")], 6, winsize);
				
				client->SetWindowSize(atoi(winsize));
				delete[] winsize;

#ifdef VARWIN
				if (packet->TFTP_HasOption("msftwindow") && client->AllowVariableWindowSize)
				{
					char* msftwin = new char[6];
					ClearBuffer(msftwin, 6);

					extString(&packet->GetBuffer()[packet->TFTP_OptionOffset("msftwindow")], 6, msftwin);
					
					client->SetMSFTWindow(27182);
					delete[] msftwin;
				}
				else
					client->SetMSFTWindow(0);
#endif
			}
			else
				client->SetWindowSize(1);

			client->SetBytesToRead(client->file->Length());
			client->SetBytesRead(0);

			client->Data = new Packet(this->Logger, type, 4 + client->GetBlockSize());
			client->Data->TFTP_OptAck(client->GetBlockSize(),
				client->GetBytesToRead(), client->GetWindowSize(), client->GetMSFTWindow());

			client->SetTFTPState(TFTP_Download);

			this->SocketState = this->Send(client);

			delete client->Data;
		}
		else
		{
			string* msg = new string("The value for option \"mode\" is not supported!");

			client->Data = new Packet(this->Logger, type, 4 + msg->size() + 2);
			client->Data->TFTP_Error(8, msg);
			client->SetTFTPState(TFTP_Error);

			this->SocketState = this->Send(client);

			delete client->Data;
			delete client->file;
			delete msg;
		}
	}
	else
	{
		string* filename = new string(client->file->Name());
		client->Data = new Packet(this->Logger, type, 4 + filename->size() + 2);
		client->Data->TFTP_Error(1, filename);
		client->SetTFTPState(TFTP_Error);

		this->SocketState = this->Send(client);

		delete filename;
		delete client->Data;
		delete client->file;
	}
}
#endif

void Connection::Handle_DHCP_Request(Client* client, Packet* packet, ServerType type)
{
	client->Data = new Packet(this->Logger, type, 1260);

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
	client->Data->Write(&client->NextServer, sizeof(uint32_t));

	/* Relay Agent */
	client->Data->Write(&packet->GetBuffer()[BOOTP_OFFSET_RELAYIP], sizeof(uint32_t));

	/* Client HW Address */
	client->Data->Write(&packet->GetBuffer()[BOOTP_OFFSET_MACADDR], 6);

	/* MAC Padding */
	client->Data->Write(&packet->GetBuffer()[BOOTP_OFFSET_MACPADDING], 10);

	/* Server name */
	char* hname = new char[64];
	ClearBuffer(hname, 64);
	gethostname(hname, 64);
	client->Data->Write(hname, 64);
	delete[] hname;

	client->Data->SetPosition(128);

	/* Client Boot file */
	ClearBuffer(&client->Data->GetBuffer()[108], 128);
	sprintf(&client->Data->GetBuffer()[108], "%s", client->GetBootfile(INTEL_X86).c_str());

	/* MAGIC COOKIE */
	client->Data->Write(&packet->GetBuffer()[BOOTP_OFFSET_COOKIE], sizeof(uint32_t));

	/* DHCP Message Type */
	client->Data->GetBuffer()[client->Data->GetPosition()] = 0x35;
	client->Data->SetPosition(1);

	client->Data->GetBuffer()[client->Data->GetPosition()] = 0x1;
	client->Data->SetPosition(1);

	client->Data->GetBuffer()[client->Data->GetPosition()] = \
		(uint8_t)GetDHCPMessageType(client, packet, type);
	client->Data->SetPosition(1);

	/* DHCP Server Id */
	client->Data->GetBuffer()[client->Data->GetPosition()] = 0x36;
	client->Data->SetPosition(1);

	client->Data->GetBuffer()[client->Data->GetPosition()] = \
		(uint8_t)sizeof(this->ServerIP);
	client->Data->SetPosition(1);

	client->Data->Write(&this->ServerIP, sizeof(uint32_t));

	/* DHCP Vendor Class */
	client->Data->GetBuffer()[client->Data->GetPosition()] = 0x3C;
	client->Data->SetPosition(1);

	client->Data->GetBuffer()[client->Data->GetPosition()] = 0x09;
	client->Data->SetPosition(1);

	string* vendorIdent = new string("PXEClient");
	client->Data->Write(vendorIdent->c_str(), vendorIdent->size());
	delete vendorIdent;

	/* DHCP Client GUID */
	uint32_t uuid_offset = GetOptionOffset(packet, 97);

	if (uuid_offset != 0)
	{
		client->Data->Write(&packet->GetBuffer()[uuid_offset], 19);
		uuid_offset = 0;
	}

	BuildWDSOptions(client, type);

	if (client->isWDSRequest)
		client->SetArchitecture((CLIENT_ARCH)packet->GetBuffer()[289]);

	/* BCD Path (Microsoft) */
	if (client->GetBCDfile().size() != 0 && this->serverType == BINL)
	{
		client->Data->GetBuffer()[client->Data->GetPosition()] = (uint8_t)252;
		client->Data->SetPosition(1);

		client->Data->GetBuffer()[client->Data->GetPosition()] = (uint8_t)client->GetBCDfile().size();
		client->Data->SetPosition(1);

		client->Data->Write(client->GetBCDfile().c_str(), client->GetBCDfile().size());
	}

	/* DHCP Option "end of Packet" */
	client->Data->GetBuffer()[client->Data->GetPosition()] = (uint8_t)255;
	client->Data->SetPosition(1);

	if (!client->isWDSRequest)
	{
		client->SetRequestID(this->requestID);
		this->SocketState = this->Send(client);
	}
	else
	{
		if (client->ActionDone == 1)
		{
			this->SocketState = this->Send(client);
			this->requestID += 1;
		}
	}

	clients->erase(client->id);
	delete client->Data;
}

void Connection::Handle_HTTP_Request(Client* client, Packet* packet, ServerType type)
{
	client->file = new FileSystem("http/" + packet->HTTP_GetFileName(), FileRead);

	if (client->file->Exist())
	{
		client->http_description = "OK";
		client->http_status = 200;

		if (client->file->CType().find("text/") != string::npos)
		{
			char* tmp = new char[client->file->Length()];
			ClearBuffer(tmp, client->file->Length());

			client->SetBytesRead(client->file->Read(tmp, 0, 0, client->file->Length()));

			stringstream ss;
			ss << HTTP_SetHeader(client->http_status, client->file->CType(),
				client->http_description, client->file->Length());
			
			ss << tmp;

			client->Data = new Packet(this->Logger, client->GetType(), ss.str().size() + 1);
			client->Data->Write(ss.str().c_str(), ss.str().size());
			
			this->SocketState = Send(client);
			close(*client->GetSocket());
			
			delete client->Data;
			delete[] tmp;
		}
		else
		{
			client->SetBytesToRead(client->file->Length());
			
			size_t bs = 0;
			size_t chunk = 16384;

			do
			{
				chunk = (16384 < (client->GetBytesToRead() - client->GetBytesRead())) ?
					(size_t)16384 : (size_t)(client->GetBytesToRead() - client->GetBytesRead());
				
				client->Data = new Packet(this->Logger, client->GetType(), chunk);
				
				bs = client->file->Read(client->Data->GetBuffer(),
					client->Data->GetPosition(), client->GetBytesRead(), chunk);
				
				client->SetBytesRead(bs);
				client->Data->SetPosition(bs);
				
				this->SocketState = Send(client);
				
				delete client->Data;

			} while (client->GetBytesRead() < client->GetBytesToRead());
						
			close(*client->GetSocket());
		}
	}
	else
	{	
		string* content;
		client->Data = new Packet(this->Logger, client->GetType(), 1024);
		client->http_description = "Not Found";
		client->http_status = 404;

		content = new string(HTTP_Error(client, client->http_description));
		client->Data->Write(content->c_str(), content->size());
		delete content;

		this->SocketState = Send(client);
		
		close(*client->GetSocket());
		delete client->Data;
	}

	delete client->file;
	clients->erase(client->id);
}