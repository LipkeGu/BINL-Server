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

enum WDSNBP_Options
{
	WDSBP_OPT_ARCHITECTURE = 1,
	WDSBP_OPT_NEXT_ACTION = 2,
	WDSBP_OPT_POLL_INTERVAL = 3,
	WDSBP_OPT_POLL_RETRY_COUNT = 4,
	WDSBP_OPT_REQUEST_ID = 5,
	WDSBP_OPT_MESSAGE = 6,
	WDSBP_OPT_VERSION_QUERY = 7,
	WDSBP_OPT_SERVER_VERSION = 8,
	WDSBP_OPT_REFERRAL_SERVER = 9,
	WDSBP_OPT_PXE_CLIENT_PROMPT = 11,
	WDSBP_OPT_PXE_PROMPT_DONE = 12,
	WDSBP_OPT_NBP_VER = 13,
	WDSBP_OPT_ACTION_DONE = 14,
	WDSBP_OPT_ALLOW_SERVER_SELECTION = 15,
	WDSBP_OPT_SERVER_FEATURES = 16,
	WDSBP_OPT_END = 255
};

enum WDSNBP_OPTION_NEXTACTION
{
	APPROVAL = 1,
	REFERRAL = 3,
	ABORT = 5
};

enum LogType
{
	Info,
	Warn,
	Error,
	Debug
};

enum ServerType
{
	DHCP,
#ifdef WITH_TFTP
	TFTP,
#endif
	BINL,
	HTTP
};

enum DHCPMsgTypes
{
	DHCP_DIS = 1,
	DHCP_OFF = 2,
	DHCP_REQ = 3,
	DHCP_ACK = 5,
	DHCP_IFM = 8
};

#ifdef WITH_TFTP
enum TFTP_OPCODES
{
	TFTP_RRQ = 1,
	TFTP_WRQ = 2,
	TFTP_DAT = 3,
	TFTP_ACK = 4,
	TFTP_ERR = 5
};
#endif

enum CLIENT_ARCH
{
	INTEL_X86 = 0,
	INTEL_X64 = 6,
	INTEL_EFI = 7
};

#ifdef WITH_TFTP
enum TFTP_State
{
	TFTP_HandShake = 0,
	TFTP_Download = 1,
	TFTP_Done = 2,
	TFTP_Error = 3
};
#endif

enum Packet_OPCode
{
	BOOTREQUEST = 1,
	BOOTREPLY = 2,

	RISREQUEST = 81,
	RISREPLY = 82
};

enum FileOpenMode
{
	FileRead = 0,
	FileWrite = 1,
	FileReadBinary = 2,
	FileWriteBinary = 3
};

enum HTTPStatuscode
{
	OK = 200,
	ForBidden = 403,
	NotFound = 404
};
