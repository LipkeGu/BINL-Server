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

class Packet
{
public:
	Packet(EventLog* pLogger, ServerType type, size_t bufferlen);
	~Packet();

	size_t GetLength();
	void SetLength(size_t length);
	size_t GetPosition();
	void SetPosition(size_t position);
	void Clear();
	void Write(const void* data, size_t length);
	char* GetBuffer();
	void SetType(ServerType type);
	ServerType GetType();
	void TFTP_Error(uint8_t errcode, string* message);
	void TFTP_Data(uint16_t block);
	void TFTP_OptAck(uint16_t blksize, long tsize, uint16_t windosize, uint16_t msftwindow);
	string HTTP_GetFileName();
	bool TFTP_HasOption(const char* option);
	size_t TFTP_OptionOffset(const char* option);
	bool TFTP_isOPCode(uint8_t opcode);
private:
	EventLog* logger;
	char* buffer;

	ServerType type;
	size_t position;
	size_t Packetlen;
};
