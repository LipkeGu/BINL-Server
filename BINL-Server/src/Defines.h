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
#include <vector>

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

typedef struct TFTP_Option
{
	std::string Option = "";
	size_t Length = 0;
	char Value[256];

	TFTP_Option()
	{

	}

	TFTP_Option(const std::string& opt, const std::string& value)
	{
		ClearBuffer(Value, (value.size() + 1));

		Option = opt;
		Length = value.size();
		memcpy(&Value, value.c_str(), value.size());
	}

	TFTP_Option(const unsigned char opt)
	{
		ClearBuffer(Value, 1);

		Option = opt;
		Length = 0;
	}

	TFTP_Option(const std::string& opt, size_t length, const unsigned short value)
	{
		ClearBuffer(Value, 2);
		Option = opt;
		memcpy(&Value, &value, 2);
	}

	~TFTP_Option()
	{

	}
} TFTP_Option;

typedef struct DHCP_Option
{
	DHCP_Option()
	{
		memset(this, 0, sizeof(this));
	};

	DHCP_Option(const unsigned char opt, const unsigned char length, const void* value)
	{
		Option = opt;
		Length = length;
		memset(Value, 0, 1024);

		if (length != 0)
			memcpy(&Value, value, Length);
	}

	DHCP_Option(const unsigned char opt, const std::string value)
	{
		Option = opt;
		Length = static_cast<unsigned char>(value.size());
		memset(&Value, 0, 1024);

		if (Length != 0)
			memcpy(Value, value.c_str(), Length);
	}

	DHCP_Option(const unsigned char opt, const unsigned char value)
	{
		Option = opt;
		Length = 1;
		memset(&Value, 0, 1024);

		if (Length != 0)
			memcpy(Value, &value, Length);
	}

	DHCP_Option(const unsigned char opt, const unsigned short value)
	{
		Option = opt;
		Length = 2;
		memset(&Value, 0, 1024);

		if (Length != 0)
			memcpy(&Value, &value, Length);
	}

	DHCP_Option(const unsigned char opt, const unsigned int value)
	{
		Option = opt;
		Length = 4;
		memset(&Value, 0, 1024);

		if (Length != 0)
			memcpy(&Value, &value, Length);
	}

	DHCP_Option(const unsigned char opt, const unsigned long value)
	{
		Option = opt;
		Length = 4;
		memset(&Value, 0, 1024);

		if (Length != 0)
			memcpy(&Value, &value, Length);
	}

	DHCP_Option(const unsigned char opt, const std::vector<DHCP_Option> value)
	{
		Option = opt;
		Length = 0;

		auto offset = 0;

		// Get the entire options length!
		for (const auto & option : value)
			Length += option.Length + 2;

		memset(&Value, 0, Length);

		for (const auto & option : value)
		{
			memcpy(&Value[offset], &option.Option, 1);
			offset += 1;

			memcpy(&Value[offset], &option.Length, 1);
			offset += 1;

			memcpy(&Value[offset], &option.Value, option.Length);
			offset += option.Length;
		}

		Length = offset;
	}

	DHCP_Option(const unsigned char opt)
	{
		Option = opt;
		Length = 0;
		memset(&Value, 0, 1024);
	}

	unsigned char Option;
	unsigned char Length;

	char Value[1024];
} DHCP_Option;

enum ServerType
{
	TYPE_DHCP,
#ifdef WITH_TFTP
	TYPE_TFTP,
#endif
	TYPE_BINL,
};

enum DHCPMsgTypes
{
	DHCP_DIS = 0x01,
	DHCP_OFF = 0x02,
	DHCP_REQ = 0x03,
	DHCP_ACK = 0x05,
	DHCP_IFM = 0x08
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
