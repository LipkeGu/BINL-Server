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

Client::Client(ServerType type, string* id, char* hwaddr)
{
	this->SetType(type);
	this->id = id;
	this->http_description = "OK";
	this->http_status = 200;
	this->s = NULL;

#ifdef WITH_TFTP
	if (type != TFTP)
	{
#endif
		this->arch = INTEL_X86;
		this->isWDSRequest = false;
		this->NextAction = APPROVAL;
		this->msgtype = DHCP_DIS;
		this->PollIntervall = BS16(1);
		this->ServerSelection = false;
		this->RetryCount = BS16(255);
		this->ActionDone = 1;
		this->NextServer = 0;
		this->referralIP = 0;
		this->AdminMessage = "IP: " + *this->id + "\tMAC: " + string(hwaddr);
		this->requestid = 0;
#ifdef WITH_TFTP
	}
	else
	{
		this->file = NULL;
#ifdef VARWIN
		this->AllowVariableWindowSize = false;
		this->msftwindow = 27182;
#endif
		this->block = 0;
		this->tftp_state = TFTP_HandShake;
		this->blocksize = 1024;
		this->windowsize = 1;
		this->max_blocksize = 8192;
		this->retries = 0;
	}
#endif

	this->bytesToRead = 0;
	this->bytesread = 0;
}

void Client::SetType(ServerType type)
{
	this->type = type;
}

ServerType Client::GetType()
{
	return this->type;
}

void Client::SetDHCPMessageType(DHCPMsgTypes type)
{
	this->msgtype = type;
}

DHCPMsgTypes Client::GetDHCPMessageType()
{
	return this->msgtype;
}

void Client::SetReferralServer(uint32_t addr)
{
	this->referralIP = addr;
}

uint32_t Client::GetReferralServer()
{
	return this->referralIP;
}

void Client::SetRequestID(uint32_t id)
{
	this->requestid = id;
}

uint32_t Client::GetRequestID()
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

CLIENT_ARCH Client::GetArchitecture()
{
	return this->arch;
}

void Client::SetArchitecture(CLIENT_ARCH arch)
{
	this->arch = arch;
}

#ifdef WITH_TFTP
void Client::SetTFTPState(TFTP_State state)
{
	this->tftp_state = state;
}

TFTP_State Client::GetTFTPState()
{
	return this->tftp_state;
}
#endif

void Client::SetBootfile(string file)
{
	this->bootfile = file;
}

void Client::SetWDSMessage(string message)
{
	this->AdminMessage = message;
}

string Client::GetWDSMessage()
{
	return this->AdminMessage;
}

void Client::SetNextAction(WDSNBP_OPTION_NEXTACTION action)
{
	this->NextAction = action;
}

WDSNBP_OPTION_NEXTACTION Client::GetNextAction()
{
	return this->NextAction;
}

void Client::SetRetryCount(uint16_t action)
{
	this->RetryCount = action;
}

uint16_t Client::GetRetryCount()
{
	return this->RetryCount;
}

void Client::SetPollInterval(uint16_t interval)
{
	this->PollIntervall = interval;
}

uint16_t Client::GetPollInterval()
{
	return this->PollIntervall;
}

string Client::GetBootfile(CLIENT_ARCH arch)
{
	if (this->isWDSRequest)
	{
		switch (arch)
		{
		case INTEL_X86:
			switch (this->GetNextAction())
			{
			case ABORT:
				this->bootfile = "Boot\\x86\\abortpxe.com";
				this->bcdfile = "";
				break;
			default:
				this->bootfile = "Boot\\x86\\pxeboot.n12";
				this->bcdfile = "Boot\\x86\\default.bcd";
				break;
			}
			break;
		case INTEL_X64:
			switch (this->GetNextAction())
			{
			case ABORT:
				this->bootfile = "Boot\\x64\\abortpxe.com";
				this->bcdfile = "";
				break;
			default:
				this->bootfile = "Boot\\x64\\pxeboot.n12";
				this->bcdfile = "Boot\\x64\\default.bcd";
				break;
			}
			break;
		case INTEL_EFI:
			this->bootfile = "Boot\\efi\\bootmgfw.efi";
			this->bcdfile = "Boot\\efi\\default.bcd";
				break;
		default:
			this->bootfile = DHCP_BOOTFILE;
			this->bcdfile = "";
			break;
		}
	}
	else
	{
		this->bootfile = DHCP_BOOTFILE;
		this->bcdfile = "";
	}

	return this->bootfile;
}

void Client::SetBCDfile(string file)
{
	this->bcdfile = file;
}

string Client::GetBCDfile()
{
	return this->bcdfile;
}

#ifdef WITH_TFTP
void Client::SetBlock()
{
	this->block++;
}

void Client::SetBlock(uint16_t block)
{
	this->block = block;
}

uint16_t Client::GetWindowSize()
{
	return this->windowsize;
}

void Client::SetWindowSize(uint16_t window)
{
	this->windowsize = window;
}

uint16_t Client::GetBlockSize()
{
	return this->blocksize;
}

void Client::SetBlockSize(uint16_t blocksize)
{
	this->blocksize = (blocksize < this->max_blocksize) ?
		blocksize : this->max_blocksize;
}

#ifdef VARWIN
uint16_t Client::GetMSFTWindow()
{
	return this->msftwindow;
}

void Client::SetMSFTWindow(uint16_t window)
{
	this->msftwindow = window;
}
#endif

uint16_t Client::GetBlock()
{
	return this->block;
}

bool Client::GetACK(Packet* packet)
{
	bool b = false;
#ifdef VARWIN
	if (packet->GetLength() > 4 && this->AllowVariableWindowSize)
		this->SetWindowSize(packet->GetBuffer()[4]);
#endif

	char* x = new char[2];
	x[0] = (this->GetBlock() & 0xFF00) >> 8;
	x[1] = (this->GetBlock() & 0x00FF);
	
	b = (memcmp(&packet->GetBuffer()[2], x, 2) == 0) ?
		true : false;

	delete x;
	
	return b;
}
#endif

long Client::GetBytesToRead()
{
	return this->bytesToRead;
}

void Client::SetBytesToRead(long bytes)
{
	this->bytesToRead = bytes;
}

long Client::GetBytesRead()
{
	return this->bytesread;
}

void Client::SetBytesRead(long bytes)
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
