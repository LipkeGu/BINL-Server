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

Packet::Packet(EventLog* pLogger, ServerType type, size_t bufferlen)
{
	this->Packetlen = bufferlen;
	this->buffer = new char[this->Packetlen];
	ClearBuffer(this->buffer, bufferlen);

	this->logger = pLogger;
	this->SetPosition(0);
	this->SetType(type);
}

size_t Packet::GetLength()
{
	return this->Packetlen;
}

void Packet::SetLength(size_t length)
{
	this->Packetlen = length;
}

char* Packet::GetBuffer()
{
	return this->buffer;
}

void Packet::SetType(ServerType type)
{
	this->type = type;
}

ServerType Packet::GetType()
{
	return this->type;
}

void Packet::SetPosition(size_t position)
{
	if (position != 0)
		this->position += position;
	else
		this->position = 0;
}

size_t Packet::GetPosition()
{
	return this->position;
}

void Packet::Write(const void* data, size_t length)
{
	if ((this->GetPosition() + length) < this->GetLength())
	{
		memcpy(&this->buffer[this->GetPosition()], data, length);
		this->SetPosition(length);
	}
}

void Packet::Clear()
{
	if (this->buffer != NULL)
		ClearBuffer(this->buffer, this->GetLength());

	if (this->GetPosition() != 0)
		this->SetPosition(0);
}

#ifdef WITH_TFTP
void Packet::TFTP_Error(uint8_t errcode, string* message)
{
	this->GetBuffer()[1] = (uint8_t)5;
	this->GetBuffer()[3] = errcode;
	this->SetPosition(4);

	this->Write(message->c_str(), message->length());

	this->GetBuffer()[this->GetPosition()] = 0;
	this->SetPosition(1);
}

void Packet::TFTP_Data(uint16_t block)
{
	this->GetBuffer()[1] = (uint8_t)3;
	this->GetBuffer()[2] = (block & 0xFF00) >> 8;
	this->GetBuffer()[3] = (block & 0x00FF);
	this->SetPosition(4);
}

void Packet::TFTP_OptAck(uint16_t blksize, long tsize, uint16_t windosize, uint16_t msftwindow)
{
	stringstream str_bs;
	stringstream str_ts;
	stringstream str_ws;
	stringstream str_mw;

	this->GetBuffer()[1] = (uint8_t)6;
	this->SetPosition(2);

	str_bs << blksize;
	this->Write("blksize", sizeof("blksize"));
	this->Write(str_bs.str().c_str(), str_bs.str().size());

	this->GetBuffer()[this->GetPosition()] = 0;
	this->SetPosition(1);

	str_ts << tsize;
	this->Write("tsize", sizeof("tsize"));
	this->Write(str_ts.str().c_str(), strlen(str_ts.str().c_str()));

	if (windosize > 1)
	{
		this->GetBuffer()[this->GetPosition()] = 0;
		this->SetPosition(1);

		str_ws << windosize;
		this->Write("windowsize", sizeof("windowsize"));
		this->Write(str_ws.str().c_str(), str_ws.str().size());
#ifdef VARWIN
		if (msftwindow > 1)
		{
			this->GetBuffer()[this->GetPosition()] = 0;
			this->SetPosition(1);

			str_mw << msftwindow;
			this->Write("msftwindow", sizeof("msftwindow"));
			this->Write(str_mw.str().c_str(), str_mw.str().size());
		}
#endif
	}
		
	this->GetBuffer()[this->GetPosition()] = 0;
	this->SetPosition(1);
}

bool Packet::TFTP_HasOption(const char* option)
{
	uint32_t i = 0;

	for (i = 2; i < this->GetLength(); i++)
		if (memcmp(option, &this->GetBuffer()[i], strlen(option)) == 0)
			return true;

	return false;
}

size_t Packet::TFTP_OptionOffset(const char* option)
{
	uint32_t i = 0;

	if (this->TFTP_HasOption(option))
		for (i = 2; i < this->GetLength(); i++)
			if (memcmp(option, &this->GetBuffer()[i], strlen(option)) == 0)
				return i + (strlen(option) + 1);

	return 0;
}

bool Packet::TFTP_isOPCode(uint8_t opcode)
{
	return (this->GetBuffer()[1] == opcode);
}
#endif

string Packet::HTTP_GetFileName()
{
	stringstream p;
	p << this->GetBuffer();

	size_t start = 0;
	size_t end = 0;

	string res = "/";

	start = p.str().find_first_of("T ");

	start += strlen("T ");

	res = p.str().substr(start);
	end = res.find_first_of(" HTT");

	res = res.substr(0, end);
	
	if (res == "/")
		return "index.html";
	else
	{
		if (res.c_str()[0] == '/')
			return res.substr(1);
		else
			return res;
	}
}

Packet::~Packet()
{
	delete[] this->buffer;
}
