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

#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <errno.h>
#include <map>
#include <stddef.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <sstream>

#ifdef _WIN32
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <inttypes.h>
#include <direct.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Advapi32.lib")
#define read(s,b,l,f) recv(s,b,l,f);
#define  _write(s,b,l,f)  send(s,b,l,f);
#define close(s) closesocket(s);

#define ClearBuffer(x, y) memset(x, 0, y)
#else
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <pthread.h>
#include <sys/wait.h>
#include <dirent.h>
#define WITH_TFTP 1
#define VARWIN 1

typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef int HANDLE;
typedef int SOCKET;

#define ClearBuffer(x, y) bzero(x, y)
#define SOCKET_ERROR -1
#endif

using namespace std;

#ifndef BINLSERVER
#define BINLSERVER

#include "Defines.h"
#include "FileSystem.h"
#include "Eventlog.h"
#include "Packet.h"
#include "Client.h"
#include "Functions.h"
#include "Connection.h"
#include "Server.h"

#define DHCP_MINIMAL_PACKET_SIZE			240
#define BOOTP_OFFSET_BOOTPTYPE				0
#define BOOTP_OFFSET_HWTYPE					1
#define BOOTP_OFFSET_MACLEN					2
#define BOOTP_OFFSET_HOPS					3
#define BOOTP_OFFSET_TRANSID				4
#define BOOTP_OFFSET_SECONDS				8
#define BOOTP_OFFSET_BOOTPFLAGS				10
#define BOOTP_OFFSET_YOURIP					16
#define BOOTP_OFFSET_CLIENTIP				12
#define BOOTP_OFFSET_NEXTSERVER				20
#define BOOTP_OFFSET_RELAYIP				24
#define BOOTP_OFFSET_MACADDR				28
#define BOOTP_OFFSET_MACPADDING				34
#define BOOTP_OFFSET_MSGTYPE				242

#define BOOTP_OFFSET_COOKIE					236

#define DHCP_BOOTFILE						"Boot/x86/wdsnbp.com"
#define DHCP_VENDORCLASS_PXE				"PXEClient"

#define WDS_MODE_RIS			0
#define WDS_MODE_WDS			1
#define WDS_MODE_UNK			2

#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN						1234
#endif

#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN						4321
#endif

#ifndef __BYTE_ORDER
#if defined(_BIG_ENDIAN)
#define __BYTE_ORDER __BIG_ENDIAN
#elif defined(_LITTLE_ENDIAN)
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define BS32(x) x
#define BS16(x) x
#elif __BYTE_ORDER == __BIG_ENDIAN
#define BS16(x) (((uint16_t)(x) >> 8) | (((uint16_t)(x) & 0xff) << 8))
#define BS32(x) (((uint32_t)(x) >> 24) | (((uint32_t)(x) >> 8) & 0xff00) | \
				(((uint32_t)(x) << 8) & 0xff0000) | ((uint32_t)(x) << 24))
#else
#define BS16(x) (((uint16_t)(x) >> 8) | (((uint16_t)(x) & 0xff) << 8))
#define BS32(x) (((uint32_t)(x) >> 24) | (((uint32_t)(x) >> 8) & 0xff00) | \
				(((uint32_t)(x) << 8) & 0xff0000) | ((uint32_t)(x) << 24))
#endif
#endif
