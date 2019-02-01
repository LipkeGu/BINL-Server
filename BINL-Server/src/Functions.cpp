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

DHCPMsgTypes GetDHCPMessageType(Client* client, Packet* packet, ServerType type)
{
	if (client == NULL)
		return DHCP_OFF;

	DHCPMsgTypes t = (DHCPMsgTypes)packet->GetBuffer()[BOOTP_OFFSET_MSGTYPE];

	switch (t)
	{
	case DHCP_DIS: // Discover
		client->DHCP_CLIENT.SetDHCPMessageType(DHCP_DIS);
		return DHCP_OFF;
	case DHCP_IFM:
	case DHCP_REQ: // Request
		client->DHCP_CLIENT.SetDHCPMessageType((t == DHCP_REQ) ? DHCP_REQ : DHCP_IFM);
		return DHCP_ACK;
	default:
		return DHCP_OFF;
	}
}

void extString(const char* buf, size_t size, char* out)
{
	strncpy(out, buf, size - 1);
}

ServerType GetClientType(Client* client, Packet* packet, ServerType type)
{
	switch ((Packet_OPCode)packet->GetBuffer()[0])
	{
	case BOOTREPLY:
	case BOOTREQUEST:
		client->SetType(type);
		client->isWDSRequest = IsWDSPacket(packet,
			GetOptionOffset(packet, 250));
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

void BuildWDSOptions(Client* client, ServerType type)
{
	char* tmpbuffer = new char[4096];
	ClearBuffer(tmpbuffer, 4096);

	uint8_t* offset = new uint8_t(2);
	uint8_t* length = new uint8_t(0);
	uint8_t* option = new uint8_t(0);
	uint8_t* DHCPend = new uint8_t(0xff);
	uint8_t* realsize = new uint8_t(0);

	if (client->DHCP_CLIENT.WDS.GetNextAction() == REFERRAL && client->DHCP_CLIENT.WDS.GetReferralServer() == 0)
		client->DHCP_CLIENT.WDS.SetNextAction(APPROVAL);

	// Next Action
	*option = WDSBP_OPT_NEXT_ACTION;
	*length = (uint8_t)sizeof(uint8_t);

	memcpy(&tmpbuffer[*offset], &option, sizeof(uint8_t));
	*offset += sizeof(uint8_t);

	memcpy(&tmpbuffer[*offset], &length, sizeof(uint8_t));
	*offset += sizeof(uint8_t);

	memcpy(&tmpbuffer[*offset], &client->DHCP_CLIENT.WDS.NextAction, sizeof(uint8_t));
	*offset += sizeof(uint8_t);

	// RequestID
	*option = WDSBP_OPT_REQUEST_ID;
	*length = (uint8_t)sizeof(uint32_t);

	memcpy(&tmpbuffer[*offset], &option, sizeof(uint8_t));
	*offset += sizeof(uint8_t);

	memcpy(&tmpbuffer[*offset], &length, sizeof(uint8_t));
	*offset += sizeof(uint8_t);

	uint32_t* reqid = new uint32_t(BS32(client->DHCP_CLIENT.WDS.GetRequestID()));
	memcpy(&tmpbuffer[*offset], &reqid, sizeof(uint32_t));
	*offset += sizeof(uint32_t);
	delete reqid;

	// Poll Interval
	*option = WDSBP_OPT_POLL_INTERVAL;
	*length = (uint8_t)sizeof(uint16_t);

	memcpy(&tmpbuffer[*offset], &option, sizeof(uint8_t));
	*offset += sizeof(uint8_t);

	memcpy(&tmpbuffer[*offset], &length, sizeof(uint8_t));
	*offset += sizeof(uint8_t);

	memcpy(&tmpbuffer[*offset], &client->DHCP_CLIENT.WDS.PollIntervall, sizeof(uint16_t));
	*offset += sizeof(uint16_t);

	// Poll Retry Count
	*option = WDSBP_OPT_POLL_RETRY_COUNT;
	*length = (uint8_t)sizeof(uint16_t);

	memcpy(&tmpbuffer[*offset], &option, sizeof(uint8_t));
	*offset += sizeof(uint8_t);

	memcpy(&tmpbuffer[*offset], &length, sizeof(uint8_t));
	*offset += sizeof(uint8_t);

	memcpy(&tmpbuffer[*offset], &client->DHCP_CLIENT.WDS.RetryCount, sizeof(uint16_t));
	*offset += sizeof(uint16_t);

	if (client->DHCP_CLIENT.WDS.GetNextAction() == REFERRAL &&
		client->DHCP_CLIENT.WDS.GetReferralServer() != 0)
	{
		if (client->DHCP_CLIENT.WDS.ServerSelection)
		{

			// Allow Server selection
			*option = WDSBP_OPT_ALLOW_SERVER_SELECTION;
			*length = (uint8_t)sizeof(uint8_t);

			memcpy(&tmpbuffer[*offset], &option, sizeof(uint8_t));
			*offset += sizeof(uint8_t);

			memcpy(&tmpbuffer[*offset], &length, sizeof(uint8_t));
			*offset += sizeof(uint8_t);

			uint8_t* val = new uint8_t(1);

			memcpy(&tmpbuffer[*offset], &val, sizeof(uint8_t));
			*offset += sizeof(uint8_t);

			delete val;
		}

		// Referal Server
		*option = WDSBP_OPT_REFERRAL_SERVER;
		*length = (uint8_t)sizeof(uint32_t);

		memcpy(&tmpbuffer[*offset], &option, sizeof(uint8_t));
		*offset += sizeof(uint8_t);

		memcpy(&tmpbuffer[*offset], &length, sizeof(uint8_t));
		*offset += sizeof(uint8_t);

		uint32_t* ipaddr = new uint32_t(client->DHCP_CLIENT.WDS.GetReferralServer());

		memcpy(&tmpbuffer[*offset], &ipaddr, sizeof(uint32_t));
		*offset += sizeof(uint32_t);

		delete ipaddr;
	}

	// Action Done
	*option = WDSBP_OPT_ACTION_DONE;
	*length = (uint8_t)sizeof(uint8_t);

	memcpy(&tmpbuffer[*offset], &option, sizeof(uint8_t));
	*offset += sizeof(uint8_t);

	memcpy(&tmpbuffer[*offset], &length, sizeof(uint8_t));
	*offset += sizeof(uint8_t);

	memcpy(&tmpbuffer[*offset], &client->DHCP_CLIENT.WDS.ActionDone, sizeof(uint8_t));
	*offset += sizeof(uint8_t);


	if (client->DHCP_CLIENT.WDS.GetWDSMessage().size() != 0)
	{
		// Admin Message
		*option = WDSBP_OPT_MESSAGE;
		*length = (uint8_t)client->DHCP_CLIENT.WDS.GetWDSMessage().size();

		memcpy(&tmpbuffer[*offset], &option, sizeof(uint8_t));
		*offset += sizeof(uint8_t);

		memcpy(&tmpbuffer[*offset], &length, sizeof(uint8_t));
		*offset += sizeof(uint8_t);

		strncpy(&tmpbuffer[*offset], client->DHCP_CLIENT.WDS.GetWDSMessage().c_str(), \
			client->DHCP_CLIENT.WDS.GetWDSMessage().size() + 1);

		*offset += (uint8_t)client->DHCP_CLIENT.WDS.GetWDSMessage().size() + 1;
	}

	memcpy(&tmpbuffer[*offset], &DHCPend, sizeof(uint8_t));
	*offset += sizeof(uint8_t);

	tmpbuffer[0] = (uint8_t)250;
	tmpbuffer[0] = (uint8_t)*offset;
	
	client->Data->Write(tmpbuffer, *offset);

	delete[] tmpbuffer;
	delete[] offset;
	delete[] realsize;
	delete[] length;
	delete[] option;
	delete[] DHCPend;
}

void handle_args(int data_len, char* Data[])
{
}