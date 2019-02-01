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

const char* hostname_to_ip(const char* hostname);
uint32_t IP2Bytes(const char* address);
DHCPMsgTypes GetDHCPMessageType(Client* client, Packet* packet, ServerType type);
size_t GetOptionOffset(Packet* packet, uint8_t option);
bool IsWDSPacket(Packet* packet, size_t offset);
bool FindVendorOpt(const char* Buffer, size_t length, const char* expression);
ServerType GetClientType(Client* client, Packet* packet, ServerType type);
void BuildWDSOptions(Client* client, ServerType type);
void extString(const char* buf, size_t size, char* out);
std::string replace(std::string& str, const std::string& from, const std::string& to);
std::string iso_8859_1_to_utf8(std::string &str);
