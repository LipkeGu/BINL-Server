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

Packet::Packet(ServerType* type, size_t bufferlen)
{
	this->Packetlen = bufferlen;
	ClearBuffer(this->buffer, sizeof buffer);

	this->SetPosition(0);
	this->SetType(type);
}

void Packet::Add_DHCPOption(const DHCP_Option option)
{
	Remove_DHCPOption(option.Option);

	dhcp_options.insert(std::pair<unsigned char, DHCP_Option>(option.Option, option));
}

void Packet::Add_TFTPOption(const TFTP_Option option)
{
	Remove_TFTPOption(option.Option);

	tftp_options.insert(std::pair<std::string, TFTP_Option>(option.Option, option));
}

void Packet::Remove_DHCPOption(const unsigned char& opt)
{
	if (Has_DHCPOption(opt))
		dhcp_options.erase(opt);
}

void Packet::Remove_TFTPOption(const std::string& opt)
{
	if (Has_TFTPOption(opt))
		tftp_options.erase(opt);
}

bool Packet::Has_DHCPOption(const unsigned char& option)
{
	return dhcp_options.find(option) != dhcp_options.end();
}

bool Packet::Has_TFTPOption(const std::string& option)
{
	return tftp_options.find(option) != tftp_options.end();
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

void Packet::SetType(ServerType* type)
{
	this->type = type;
}

ServerType* Packet::GetType()
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

void Packet::Commit()
{
	switch (*this->GetType())
	{
	case TYPE_DHCP:
	case TYPE_BINL:
		Add_DHCPOption(DHCP_Option(0xff));
		this->SetPosition(0);
		this->SetPosition(240);

		for (auto & option : dhcp_options)
		{
			Write(&option.second.Option, sizeof(unsigned char));
			Write(&option.second.Length, sizeof(unsigned char));

			if (option.second.Length != 1)
				Write(&option.second.Value, option.second.Length);
			else if (option.second.Length != 0)
				Write(&option.second.Value, option.second.Length);
		}

		for (auto i = this->GetLength(); this->GetLength() > 240; i--)
			if (static_cast<unsigned char>(this->GetBuffer()[i]) == static_cast<unsigned char>(0xff))
			{
				this->SetPosition(0);
				this->SetPosition(i + 1);
				return;
			}
		break;
	}

}

void Packet::Parse()
{
	switch (*this->GetType())
	{
	case TYPE_DHCP:
	case TYPE_BINL:
		for (auto i = 240; i < this->GetLength(); i++)
		{
			if (static_cast<unsigned char>(this->GetBuffer()[i]) == static_cast<unsigned char>(0xff))
				break;

			if (static_cast<unsigned char>(this->GetBuffer()[i + 1]) == static_cast<unsigned char>(1))
			{
				auto x = this->GetBuffer()[i];
				Add_DHCPOption(DHCP_Option(static_cast<unsigned char>(x),
					static_cast<unsigned char>(this->GetBuffer()[i + 2])));
			}
			else
				Add_DHCPOption(DHCP_Option(static_cast<unsigned char>(this->GetBuffer()[i]),
					static_cast<unsigned char>(this->GetBuffer()[i + 1]), &this->GetBuffer()[i + 2]));

			i += 1 + this->GetBuffer()[i + 1];
		}
		break;
	}
}

void Packet::Write(const void* data, size_t length)
{
	if ((this->GetPosition() + length) < this->GetLength())
	{
		memcpy(&this->buffer[this->GetPosition()], data, length);
		this->SetPosition(length);
	}
}

void Packet::Write(const char* data, size_t length)
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

void Packet::TFTP_Error(uint8_t errcode, std::string* message)
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
	std::stringstream* ss = new std::stringstream;

	this->GetBuffer()[1] = (uint8_t)6;
	this->SetPosition(2);

	*ss << blksize;
	this->Write("blksize", sizeof("blksize"));
	this->Write(ss->str().c_str(), ss->str().size());
	delete ss;

	this->GetBuffer()[this->GetPosition()] = 0;
	this->SetPosition(1);

	ss = new std::stringstream;
	*ss << tsize;
	this->Write("tsize", sizeof("tsize"));
	this->Write(ss->str().c_str(), ss->str().size());
	delete ss;

	if (windosize > 1)
	{
		this->GetBuffer()[this->GetPosition()] = 0;
		this->SetPosition(1);

		ss = new std::stringstream;
		*ss << windosize;
		this->Write("windowsize", sizeof("windowsize"));
		this->Write(ss->str().c_str(), ss->str().size());
		delete ss;

		if (msftwindow > 1)
		{
			this->GetBuffer()[this->GetPosition()] = 0;
			this->SetPosition(1);

			ss = new std::stringstream;
			*ss << msftwindow;
			this->Write("msftwindow", sizeof("msftwindow"));
			this->Write(ss->str().c_str(), ss->str().size());
			delete ss;
		}
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

DHCP_Option Packet::Get_DHCPOption(const unsigned char option)
{
	return this->dhcp_options.at(option);
}

TFTP_Option Packet::Get_TFTPOption(const std::string option)
{
	return this->tftp_options.at(option);
}

Packet::~Packet()
{
}
