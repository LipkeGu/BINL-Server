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

class Client
{
public:
	Client(ServerType* type, std::string id);
	~Client();

	void SetType(ServerType* type);
	ServerType* GetType();
	std::string id;

	DHCP_CLIENT* dhcp = nullptr;
	TFTP_CLIENT* tftp = nullptr;
private:
	ServerType* type;
};
