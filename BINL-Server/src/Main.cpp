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

#include "Main.h"
#ifdef _WIN32
DWORD WINAPI DHCP_Thread(void* threadArgs)
#else
void* DHCP_Thread(void* threadArgs)
#endif
{
	_dhcp = new Server(67, std::unique_ptr<ServerType>(new ServerType(TYPE_DHCP)).get(), *((uint32_t *)threadArgs));
	
	delete _dhcp;
	return 0;
}

#ifdef _WIN32
DWORD WINAPI TFTP_Thread(void* threadArgs)
#else
void* TFTP_Thread(void* threadArgs)
#endif
{
	_tftp = new Server(69, std::unique_ptr<ServerType>(new ServerType(TYPE_TFTP)).get(), *((uint32_t *)threadArgs));

	delete _tftp;
	return 0;
}

int main(int argc, char* argv[])
{
	int i;
	uint32_t ip = 0;

	if (argc > 1)
		for (i = 0; i < argc; i++)
		{
			if (memcmp(argv[i], "-ip", 6) == 0) /* THIS Server IP */
				ip = IP2Bytes(argv[(i + 1)]);
		}

#ifdef _WIN32
	DHCP_Handle = CreateThread(0, 0, &DHCP_Thread, &ip, 0, &DHCP_ThreadID);
#else
	pthread_create(&DHCP_ThreadID, 0, &DHCP_Thread, &ip);
#endif

#ifdef _WIN32
	TFTP_Handle = CreateThread(0, 0, &TFTP_Thread, &ip, 0, &TFTP_ThreadID);
#else
	pthread_create(&TFTP_ThreadID, 0, &TFTP_Thread, &ip);
#endif
	_binl = new Server(4011, std::unique_ptr<ServerType>(new ServerType(TYPE_BINL)).get(), ip);

	delete _binl;
#ifdef _WIN32
	CloseHandle(DHCP_Handle);
	CloseHandle(TFTP_Handle);
#else
	pthread_exit(&DHCP_ThreadID);
	pthread_exit(&TFTP_ThreadID);
#endif
	return 0;
}
