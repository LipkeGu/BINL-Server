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
#define _CRT_SECURE_NO_WARNINGS

#include "Includes.h"

const char* hostname_to_ip(const char* hostname)
{
	return inet_ntoa(**(struct in_addr**)gethostbyname(hostname)->h_addr_list);
}

uint32_t IP2Bytes(const char* addr)
{
	struct in_addr ipvalue;
	inet_pton(AF_INET, addr, &ipvalue);

	return ipvalue.s_addr;
}

DHCPMsgTypes GetDHCPMessageType(Client* client, Packet* packet, ServerType* type)
{
	if (client == NULL)
		return DHCP_OFF;

	DHCPMsgTypes t = (DHCPMsgTypes)packet->GetBuffer()[BOOTP_OFFSET_MSGTYPE];

	switch (t)
	{
	case DHCP_DIS: // Discover
		client->dhcp->SetMessageType(DHCP_DIS);
		return DHCP_OFF;
	case DHCP_IFM:
	case DHCP_REQ: // Request
		client->dhcp->SetMessageType((t == DHCP_REQ) ? DHCP_REQ : DHCP_IFM);
		return DHCP_ACK;
	default:
		return DHCP_OFF;
	}
}

void extString(const char* buf, size_t size, char* out)
{
	strncpy(out, buf, size - 1);
}

ServerType* GetClientType(Client* client, Packet* packet, ServerType* type)
{
	switch ((Packet_OPCode)packet->GetBuffer()[0])
	{
	case BOOTREPLY:
	case BOOTREQUEST:
		client->SetType(type);
		client->dhcp->SetIsWDSRequest(IsWDSPacket(packet,
			GetOptionOffset(packet, 250)));
		break;
	case RISREQUEST:
	case RISREPLY:
		client->SetType(type);
		break;
	default:
		break;
	}

	return client->GetType();
}

std::string GetCurDir()
{
	char* cCurrentPath = new char[260];
	std::string x = "";

#ifdef _WIN32 
	_getcwd(cCurrentPath, 260);
#else
	getcwd(cCurrentPath, 260);
#endif
	x = cCurrentPath[260];
	
	delete[] cCurrentPath;

	return x;
}

std::string replace(std::string& str, const std::string& from, const std::string& to)
{
	size_t start_pos = str.find(from);

	while (str.find(from) != std::string::npos)
	{
		start_pos = str.find(from);
		
		if (start_pos != std::string::npos)
			str = str.replace(start_pos, from.length(), to);
	}

	return str;
}

size_t GetOptionOffset(Packet* packet, uint8_t option)
{
	size_t i = 0;

	for (i = packet->GetLength(); i > 243; i--)
		if (memcmp(&packet->GetBuffer()[i], &option, sizeof(uint8_t)) == 0)
			return i;

	return 0;
}

bool IsWDSPacket(Packet* packet, size_t offset)
{
	size_t length = packet->GetBuffer()[offset + 1];
	uint8_t pxeclientend = 255;

	return (memcmp(&packet->GetBuffer()[(offset + 1 + length)],
		&pxeclientend, sizeof(uint8_t)) == 0) ? true : false;
}

std::string iso_8859_1_to_utf8(std::string &str)
{
	std::string strOut = "";
	uint8_t* ch = new uint8_t(0);

	for (std::string::iterator it = str.begin(); it != str.end(); ++it)
	{
		*ch = *it;
		if (*ch < 0x80)
			strOut.push_back(*ch);
		else
		{
			strOut.push_back(0xc0 | *ch >> 6);
			strOut.push_back(0x80 | (*ch & 0x3f));
		}
	}

	delete ch;
	return strOut;
}

bool FindVendorOpt(const char* Buffer, size_t length, const char* expression)
{
	size_t i = 0;

	for (i = 0; i < length; i++)
		if (memcmp(expression, &Buffer[i], 9) == 0)
			return true;

	return false;
}

void handle_args(int data_len, char* Data[])
{
}