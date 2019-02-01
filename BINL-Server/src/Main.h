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

#ifdef _WIN32
DWORD DHCP_ThreadID;
HANDLE DHCP_Handle;

DWORD TFTP_ThreadID;
HANDLE TFTP_Handle;

DWORD WINAPI DHCP_Thread(void* threadArgs);
DWORD WINAPI TFTP_Thread(void* threadArgs);
#else
pthread_t DHCP_ThreadID;
pthread_t TFTP_ThreadID;
pthread_t HTTP_ThreadID;

void* DHCP_Thread(void* threadArgs);
void* TFTP_Thread(void* threadArgs);
#endif
Server* _dhcp;
Server* _binl;
Server* _tftp;

