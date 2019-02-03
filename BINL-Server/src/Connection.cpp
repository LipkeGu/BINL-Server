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

Connection::Connection(ServerType* type, uint32_t IPAddress)
{
	this->serverType = type;
	this->clients = new std::map<std::string, std::unique_ptr<Client>>();
	this->ServerIP = IPAddress;
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
		if (*this->serverType == TYPE_DHCP)
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
	while (this->SocketState != SOCKET_ERROR)
	{
		char* buffer = new char[16384];
		ClearBuffer(buffer, 16384);
#ifdef _WIN32
		int remotesocklen = sizeof(this->remote);
#else
		socklen_t remotesocklen = sizeof(this->remote);
#endif
		this->SocketState = recvfrom(this->Conn, buffer, 16384,
			0, (struct sockaddr *)&this->remote, &remotesocklen);

		if (this->SocketState == SOCKET_ERROR)
			continue;

		this->Handle_Request(this->serverType, remote, buffer, this->SocketState);
		delete[] buffer;
	}

	return this->SocketState;
}

Client* Connection::Get_Client(ServerType* type, sockaddr_in& remote, Packet* packet)
{
	std::string addr = std::string(inet_ntoa(this->remote.sin_addr));
	if (clients->find(addr) == clients->end())
	{
		clients->insert(std::pair<std::string, std::unique_ptr<Client>>(addr, new Client(this->serverType, addr)));
	}

	return clients->find(addr)->second.get();
}

void Connection::Handle_Request(ServerType* type, sockaddr_in& remote, const char* buffer, size_t length)
{
	Packet* packet = new Packet(this->serverType, length);
	memcpy(&packet->GetBuffer()[0], buffer, length);

	Client* c = this->Get_Client(this->serverType, remote, packet);

	switch (*GetClientType(c, packet, this->serverType))
	{
	case TYPE_DHCP:
	case TYPE_BINL:
		if (FindVendorOpt(packet->GetBuffer(), packet->GetLength(), DHCP_VENDORCLASS_PXE))
		{
			packet->Parse();
			c->dhcp->SetNextServer(this->ServerIP);
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
			break;
		}

		if (c->tftp->GetTFTPState() == TFTP_Done || c->tftp->GetTFTPState() == TFTP_Error)
			RemoveClient(c->id);
		break;
	default:
		break;
	}

	delete packet;
}

int Connection::Send(Packet* packet)
{
	int retval = SOCKET_ERROR;

	if (*this->serverType == TYPE_DHCP)
	{
		struct sockaddr_in toAddr;
		ClearBuffer(&toAddr, sizeof(toAddr));

		toAddr.sin_family = AF_INET;
		toAddr.sin_addr.s_addr = INADDR_BROADCAST;
		toAddr.sin_port = htons(68);

		retval = sendto(this->Conn, packet->GetBuffer(),
			(int)packet->GetPosition(), 0, (struct sockaddr *)&toAddr, sizeof(toAddr));
	}
	else
		retval = sendto(this->Conn, packet->GetBuffer(), (int)packet->GetPosition(),
			0, (struct sockaddr *)&this->remote, sizeof(this->remote));

	return retval;
}

void Connection::RemoveClient(std::string& id)
{
	this->clients->erase(id);
}

Connection::~Connection()
{
	this->clients->clear();
	delete this->clients;
#ifdef _WIN32
	WSACleanup();
#endif
}

void Connection::Handle_ERR_Request(Client* client, Packet* packet, ServerType* type)
{
	if (packet->GetBuffer()[3] == 0)
		return;

	client->tftp->SetTFTPState(TFTP_Error);
	auto errmsg = new char[packet->GetLength() - 4];
	ClearBuffer(errmsg, packet->GetLength() - 4);

	strncpy(errmsg, &packet->GetBuffer()[4], packet->GetLength() - 4);

	delete[] errmsg;
}

void Connection::Handle_ACK_Request(Client* client, Packet* packet, ServerType* type)
{
	if (client->tftp->GetTFTPState() != TFTP_Download)
	{
		client->tftp->SetTFTPState(TFTP_Error);
		return;
	}

	if (packet->GetLength() > 4)
		client->tftp->SetWindowSize(packet->GetBuffer()[4]);

	uint16_t blk = BS16(client->tftp->GetBlock());
	bool isInSync = memcmp(&packet->GetBuffer()[2], &blk, sizeof blk) == 0;

	if (!isInSync)
	{
		client->tftp->SetTFTPState(TFTP_Error);
		return;
	}

	FILE* fil = fopen(client->tftp->GetFilename().c_str(), "rb");
	for (uint16_t i = 0; i < client->tftp->GetWindowSize(); i++)
	{
		size_t* chunk = new size_t((client->tftp->GetBlockSize() < (client->tftp->GetBytesToRead() - client->tftp->GetBytesRead())) ?
			(size_t)client->tftp->GetBlockSize() : (size_t)(client->tftp->GetBytesToRead() - client->tftp->GetBytesRead()));

		Packet* response = new Packet(type, 4 + *chunk + 1);
		client->tftp->SetBlock(client->tftp->GetBlock() + 1);

		response->TFTP_Data(client->tftp->GetBlock());


		if (fseek(fil, client->tftp->GetBytesRead(), SEEK_SET) != 0)
			break;

		client->tftp->SetBytesRead(fread(&response->GetBuffer()[response->GetPosition()], 1, *chunk, fil));

		response->SetPosition(*chunk);
		delete chunk;

		this->SocketState = this->Send(response);

		delete response;

		if (client->tftp->GetBytesRead() == client->tftp->GetBytesToRead())
			client->tftp->SetTFTPState(TFTP_Done);
	}

	fclose(fil);
}

std::string Connection::ResolvePath(std::string& path)
{
	std::string res = "./";

	if (path.find_first_of('\\') == 0 || path.find_first_of('/') == 0)
	{
		res = "./" + path.substr(1, path.length() - 1);
#ifdef _WIN32
		res = replace(res, "/", "\\");
#else
		res = replace(res, "\\", "/");
#endif
	}
	else
	{
		res = "./" + path;
#ifdef _WIN32
		res = replace(res, "//", "/");
		res = replace(res, "/", "\\");
#else
		res = replace(res, "\\\\", "\\");
		res = replace(res, "\\", "/");
#endif
	}

	return res;
}

void Connection::Handle_RRQ_Request(Client* client, Packet* packet, ServerType* type)
{
	char filename[255];
	ClearBuffer(filename, 255);
	Packet* response = nullptr;
	size_t* pktLength = nullptr;

	extString(&packet->GetBuffer()[2], 255, filename);
	client->tftp->SetFilename(ResolvePath(std::string(filename)));
	
	client->tftp->SetBlock(0);
	/*
	if (file->Exist())
	{
	*/
	if (packet->TFTP_HasOption("octet"))
	{
		if (packet->TFTP_HasOption("blksize"))
		{
			char blksize[6];

			ClearBuffer(blksize, 6);
			extString(&packet->GetBuffer()[packet->TFTP_OptionOffset("blksize")], 6, blksize);

			client->tftp->SetBlockSize(atoi(blksize));
		}
		else
			client->tftp->SetBlockSize(1024);

		if (packet->TFTP_HasOption("windowsize"))
		{
			char winsize[6];

			ClearBuffer(winsize, 6);
			extString(&packet->GetBuffer()[packet->TFTP_OptionOffset("windowsize")], 6, winsize);

			client->tftp->SetWindowSize(atoi(winsize));

			if (packet->TFTP_HasOption("msftwindow"))
			{
				char msftwin[6];
				ClearBuffer(msftwin, 6);

				extString(&packet->GetBuffer()[packet->TFTP_OptionOffset("msftwindow")], 6, msftwin);

				client->tftp->SetMSFTWindow(27182);
			}
			else
				client->tftp->SetMSFTWindow(0);
		}
		else
			client->tftp->SetWindowSize(1);
		FILE* f = nullptr;
		f = fopen(client->tftp->GetFilename().c_str(), "rb");
		if (!f)
			return;
		
		fseek(f, 0, SEEK_END);
		client->tftp->SetBytesToRead(ftell(f));
		rewind(f);
		fclose(f);

		client->tftp->SetBytesRead(0);

		pktLength = new size_t(static_cast<size_t>(
			type, 4 + client->tftp->GetBlockSize()));

		response = new Packet(type, *pktLength);
		response->TFTP_OptAck(client->tftp->GetBlockSize(), client->tftp->GetBytesToRead(), \
			client->tftp->GetWindowSize(), client->tftp->GetMSFTWindow());

		client->tftp->SetTFTPState(TFTP_Download);

		this->SocketState = this->Send(response);
	}
	else
	{
		std::string msg = "The value for option \"mode\" is not supported!";

		pktLength = new size_t(static_cast<size_t>(4 + msg.size() + 2));

		response = new Packet(type, *pktLength);
		response->TFTP_Error(8, &msg);
		client->tftp->SetTFTPState(TFTP_Error);

		this->SocketState = this->Send(response);
	}
	/*
	}
	else
	{
		std::string filename = file->Name();
		pktLength = new size_t(static_cast<size_t>(4 + filename.size() + 2));
		response = new Packet(type, *pktLength);
		response->TFTP_Error(1, &filename);
		this->SocketState = this->Send(response);
		client->tftp->SetTFTPState(TFTP_Error);
	}
	*/
	delete response;
	delete pktLength;
}

void Connection::Handle_DHCP_Request(Client* client, Packet* packet, ServerType* type)
{
	Packet* response = new Packet(type, 1260);

	/* BOOTP Type */
	response->GetBuffer()[response->GetPosition()] = 0x02;
	response->SetPosition(1);

	/* Hardware Type (Copy also the additional 8 Bytes in one run) */
	response->Write(&packet->GetBuffer()[BOOTP_OFFSET_HWTYPE], 9);

	/* BOOTP Flags */
	response->Write(&packet->GetBuffer()[BOOTP_OFFSET_BOOTPFLAGS], sizeof(uint16_t));

	/* Client IP */
	response->Write(&packet->GetBuffer()[BOOTP_OFFSET_CLIENTIP], sizeof(uint32_t));

	/* Your IP */
	response->Write(&packet->GetBuffer()[BOOTP_OFFSET_YOURIP], sizeof(uint32_t));

	/* Next Server */
	uint32_t nextIP = client->dhcp->GetNextServer();
	response->Write(&nextIP, sizeof(uint32_t));

	/* Relay Agent */
	response->Write(&packet->GetBuffer()[BOOTP_OFFSET_RELAYIP], sizeof(uint32_t));

	/* Client HW Address */
	response->Write(&packet->GetBuffer()[BOOTP_OFFSET_MACADDR], 6);

	/* MAC Padding */
	response->Write(&packet->GetBuffer()[BOOTP_OFFSET_MACPADDING], 10);

	/* Server name */
	char hname[64];
	ClearBuffer(hname, sizeof hname);
	gethostname(hname, sizeof hname);
	response->Write(hname, sizeof hname);
	response->SetPosition(128);

	/* Client Boot file */
	ClearBuffer(&response->GetBuffer()[108], 128);
	sprintf(&response->GetBuffer()[108], "%s", DHCP_BOOTFILE);

	/* MAGIC COOKIE */
	response->Write(&packet->GetBuffer()[BOOTP_OFFSET_COOKIE], sizeof(uint32_t));

	/* DHCP Message Type */
	response->Add_DHCPOption(DHCP_Option(53, static_cast<unsigned char>
		(GetDHCPMessageType(client, packet, type))));

	/* DHCP Server Id */
	response->Add_DHCPOption(DHCP_Option(54, this->ServerIP));


	/* DHCP Vendor Class */
	response->Add_DHCPOption(DHCP_Option(60, "PXEClient"));

	/* DHCP Client GUID */
	response->Add_DHCPOption(packet->Get_DHCPOption(97));

	//BuildWDSOptions(client, type);

	if (client->dhcp->GetIsWDSRequest())
		client->dhcp->SetArchitecture((CLIENT_ARCH)packet->GetBuffer()[289]);

	/* BCD Path (Microsoft) */
	if (client->dhcp->wds->GetBCDfile().size() != 0 && *this->serverType == TYPE_BINL)
		response->Add_DHCPOption(DHCP_Option(252, client->dhcp->wds->GetBCDfile()));

	// WDS Option -> Used by WDSNBP
	std::vector<DHCP_Option>* wdsOptions = new std::vector<DHCP_Option>();
	wdsOptions->emplace_back(WDSBP_OPT_NEXT_ACTION, static_cast<unsigned char>(client->dhcp->wds->GetNextAction()));
	wdsOptions->emplace_back(WDSBP_OPT_REQUEST_ID, static_cast<unsigned long>(BS32(client->dhcp->wds->GetRequestID())));
	wdsOptions->emplace_back(WDSBP_OPT_POLL_INTERVAL, static_cast<unsigned short>(BS16(client->dhcp->wds->GetPollInterval())));
	wdsOptions->emplace_back(WDSBP_OPT_POLL_RETRY_COUNT, static_cast<unsigned short>(BS16(client->dhcp->wds->GetRetryCount())));

	if (client->dhcp->wds->GetNextAction() == REFERRAL)
	{
		wdsOptions->emplace_back(WDSBP_OPT_REFERRAL_SERVER, client->dhcp->wds->GetReferalServer());
		wdsOptions->emplace_back(WDSBP_OPT_ALLOW_SERVER_SELECTION, static_cast<unsigned char>(1));
	}

	wdsOptions->emplace_back(WDSBP_OPT_ACTION_DONE, static_cast<unsigned char>(client->dhcp->wds->GetActionDone()));

	if (client->dhcp->wds->GetWDSMessage().size() != 0)
		wdsOptions->emplace_back(WDSBP_OPT_MESSAGE, client->dhcp->wds->GetWDSMessage());

	response->Add_DHCPOption(DHCP_Option(250, *wdsOptions));
	wdsOptions->clear();
	delete wdsOptions;

	/* DHCP Option "end of Packet" */
	response->Commit();

	this->SocketState = this->Send(response);


	delete response;
	RemoveClient(client->id);
}
