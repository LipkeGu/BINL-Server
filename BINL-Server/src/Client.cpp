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

Client::Client(ServerType* type, std::string id)
{
	this->SetType(type);
	this->id = id;

	switch (*type)
	{
	case TYPE_DHCP:
	case TYPE_BINL:
		this->dhcp = new DHCP_CLIENT();
		break;
	case TYPE_TFTP:
		this->tftp = new TFTP_CLIENT();
		break;
	}
}

void Client::SetType(ServerType* type)
{
	this->type = type;
}

ServerType* Client::GetType()
{
	return this->type;
}

Client::~Client()
{
	delete this->dhcp;
	delete this->tftp;
}
