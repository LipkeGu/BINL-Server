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
#include "Includes.h"

Client::Client(ServerType type, std::string* id, char* hwaddr)
{
	this->SetType(type);
	this->id = id;
}

void Client::SetType(ServerType type)
{
	this->type = type;
}

ServerType Client::GetType()
{
	return this->type;
}

void Client::DHCP_CLIENT::SetDHCPMessageType(DHCPMsgTypes type)
{
	this->msgtype = type;
}

DHCPMsgTypes Client::DHCP_CLIENT::GetDHCPMessageType()
{
	return this->msgtype;
}

void Client::DHCP_CLIENT::WDS::SetReferralServer(uint32_t addr)
{
	this->referralIP = addr;
}

uint32_t Client::DHCP_CLIENT::WDS::GetReferralServer()
{
	return this->referralIP;
}

void Client::DHCP_CLIENT::WDS::SetRequestID(uint32_t id)
{
	this->requestid = id;
}

uint32_t Client::DHCP_CLIENT::WDS::GetRequestID()
{
	return this->requestid;
}

void Client::SetSocket(SOCKET* s)
{
	this->s = s;
}

SOCKET* Client::GetSocket()
{
	return this->s;
}

CLIENT_ARCH Client::DHCP_CLIENT::GetArchitecture()
{
	return this->arch;
}

void Client::DHCP_CLIENT::SetArchitecture(CLIENT_ARCH arch)
{
	this->arch = arch;
}

#ifdef WITH_TFTP
void Client::TFTP_CLIENT::SetTFTPState(TFTP_State state)
{
	this->tftp_state = state;
}

TFTP_State Client::TFTP_CLIENT::GetTFTPState()
{
	return this->tftp_state;
}
#endif

void Client::DHCP_CLIENT::SetBootfile(std::string file)
{
	this->bootfile = file;
}

void Client::DHCP_CLIENT::WDS::SetWDSMessage(std::string message)
{
	this->AdminMessage = message;
}

std::string Client::DHCP_CLIENT::WDS::GetWDSMessage()
{
	return this->AdminMessage;
}

void Client::DHCP_CLIENT::WDS::SetNextAction(WDSNBP_OPTION_NEXTACTION action)
{
	this->NextAction = action;
}

WDSNBP_OPTION_NEXTACTION Client::DHCP_CLIENT::WDS::GetNextAction()
{
	return this->NextAction;
}

void Client::DHCP_CLIENT::WDS::SetRetryCount(uint16_t action)
{
	this->RetryCount = action;
}

uint16_t Client::DHCP_CLIENT::WDS::GetRetryCount()
{
	return this->RetryCount;
}

void Client::DHCP_CLIENT::WDS::SetPollInterval(uint16_t interval)
{
	this->PollIntervall = interval;
}

uint16_t Client::DHCP_CLIENT::WDS::GetPollInterval()
{
	return this->PollIntervall;
}

std::string Client::DHCP_CLIENT::GetBootfile(CLIENT_ARCH arch)
{
	switch (arch)
	{
	case INTEL_X86:
		switch (this->WDS.GetNextAction())
		{
		case ABORT:
			this->bootfile = "Boot\\x86\\abortpxe.com";
			break;
		default:
			this->bootfile = "Boot\\x86\\pxeboot.n12";
//			this->bcdfile = "Boot\\x86\\default.bcd";
			break;
		}
		break;
	case INTEL_X64:
		switch (this->WDS.GetNextAction())
		{
		case ABORT:
			this->bootfile = "Boot\\x64\\abortpxe.com";
//			this->bcdfile = "";
			break;
		default:
			this->bootfile = "Boot\\x64\\pxeboot.n12";
//			this->bcdfile = "Boot\\x64\\default.bcd";
			break;
		}
		break;
	case INTEL_EFI:
		this->bootfile = "Boot\\efi\\bootmgfw.efi";
//		this->bcdfile = "Boot\\efi\\default.bcd";
		break;
	default:
		this->bootfile = DHCP_BOOTFILE;
//		this->bcdfile = "";
		break;
	}

	return this->bootfile;
}

void Client::SetBCDfile(std::string file)
{
	this->bcdfile = file;
}

std::string Client::GetBCDfile()
{
	return this->bcdfile;
}

#ifdef WITH_TFTP
void Client::TFTP_CLIENT::SetBlock()
{
	this->block++;
}

void Client::TFTP_CLIENT::SetBlock(uint16_t block)
{
	this->block = block;
}

uint16_t Client::TFTP_CLIENT::GetWindowSize()
{
	return this->windowsize;
}

void Client::TFTP_CLIENT::SetWindowSize(uint16_t window)
{
	this->windowsize = window;
}

uint16_t Client::TFTP_CLIENT::GetBlockSize()
{
	return this->blocksize;
}

void Client::TFTP_CLIENT::SetBlockSize(uint16_t blocksize)
{
	this->blocksize = (blocksize < this->max_blocksize) ?
		blocksize : this->max_blocksize;
}

uint16_t Client::TFTP_CLIENT::GetMSFTWindow()
{
	return this->msftwindow;
}

void Client::TFTP_CLIENT::SetMSFTWindow(uint16_t window)
{
	this->msftwindow = window;
}
#endif

uint16_t Client::TFTP_CLIENT::GetBlock()
{
	return this->block;
}

bool Client::TFTP_CLIENT::GetACK(Packet* packet)
{
	bool b = false;
#
	if (packet->GetLength() > 4)
		this->SetWindowSize(packet->GetBuffer()[4]);
#
	char* x = new char[2];
	x[0] = (this->GetBlock() & 0xFF00) >> 8;
	x[1] = (this->GetBlock() & 0x00FF);

	b = (memcmp(&packet->GetBuffer()[2], x, 2) == 0) ?
		true : false;

	delete x;

	return b;
}

long Client::TFTP_CLIENT::GetBytesToRead()
{
	return this->bytesToRead;
}

void Client::TFTP_CLIENT::SetBytesToRead(long bytes)
{
	this->bytesToRead = bytes;
}

long Client::TFTP_CLIENT::GetBytesRead()
{
	return this->bytesread;
}

void Client::TFTP_CLIENT::SetBytesRead(long bytes)
{
	if (bytes == 0)
		this->bytesread = bytes;
	else
		this->bytesread += bytes;
}

Client::~Client()
{
	if (this->s)
		close(*this->s);

	if (this->file)
	{
		this->file->Close();
		delete this->file;
	}

	delete this->Data;
	delete id;
}
