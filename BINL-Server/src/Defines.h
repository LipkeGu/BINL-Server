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
	WDSBP_OPT_END = 0xff
};

enum WDSNBP_OPTION_NEXTACTION
{
	APPROVAL = 0x01,
	REFERRAL = 0x03,
	ABORT = 0x05
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

typedef enum ServerType
{
	TYPE_DHCP = 0x00,
	TYPE_TFTP = 0x01,
	TYPE_BINL = 0x02
};

enum DHCPMsgTypes
{
	DHCP_DIS = 0x01,
	DHCP_OFF = 0x02,
	DHCP_REQ = 0x03,
	DHCP_ACK = 0x05,
	DHCP_IFM = 0x08
};

enum TFTP_OPCODES
{
	TFTP_RRQ = 1,
	TFTP_WRQ = 2,
	TFTP_DAT = 3,
	TFTP_ACK = 4,
	TFTP_ERR = 5
};

enum CLIENT_ARCH
{
	INTEL_X86 = 0x00,
	INTEL_X64 = 0x06,
	INTEL_EFI = 0x07
};

enum TFTP_State
{
	TFTP_HandShake = 0x00,
	TFTP_Download = 0x01,
	TFTP_Done = 0x02,
	TFTP_Error = 0x03
};

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

typedef struct RBCP {
	RBCP() {
		this->item = new uint16_t(0);
		this->layer = new uint16_t(0);
	};

	~RBCP() {
		delete this->item;
		delete this->layer;
	};
private:
	uint16_t* item;
	uint16_t* layer;
};

typedef struct WDS {
	WDS() {
		this->NextAction = new uint8_t(WDSNBP_OPTION_NEXTACTION::APPROVAL);
		this->ActionDone = new uint8_t(0);
		this->PollIntervall = new uint16_t(10);
		this->RetryCount = new uint16_t(10);
		this->requestid = new uint32_t(1);
		this->bcdfile = new std::string("");
		this->referalIP = new uint32_t(0);
		this->AdminMessage = new std::string("");
		this->ServerSelection = new bool(0);
	}

	~WDS() {
		delete this->NextAction;
		delete this->ActionDone;
		delete this->PollIntervall;
		delete this->RetryCount;
		delete this->requestid;
		delete this->bcdfile;
		delete this->referalIP;
		delete this->AdminMessage;
		delete this->ServerSelection;
	}

	std::string WDS::GetBCDfile()
	{
		return *this->bcdfile;
	}

	void WDS::SetReferralServer(uint32_t addr)
	{
		*this->referalIP = addr;
	}

	uint32_t WDS::GetReferalServer()
	{
		return *this->referalIP;
	}

	void WDS::SetRequestID(uint32_t id)
	{
		*this->requestid = id;
	}

	uint32_t WDS::GetRequestID()
	{
		return *this->requestid;
	}

	void WDS::SetActionDone(const unsigned char& done)
	{
		this->ActionDone;
	}

	unsigned char& WDS::GetActionDone()
	{
		return *this->ActionDone;
	}

	void WDS::SetWDSMessage(std::string message)
	{
		*this->AdminMessage = message;
	}

	std::string WDS::GetWDSMessage()
	{
		return *this->AdminMessage;
	}

	void WDS::SetNextAction(WDSNBP_OPTION_NEXTACTION action)
	{
		*this->NextAction = action;
	}

	uint8_t WDS::GetNextAction()
	{
		return *this->NextAction;
	}

	void WDS::SetRetryCount(uint16_t action)
	{
		*this->RetryCount = action;
	}

	uint16_t WDS::GetRetryCount()
	{
		return *this->RetryCount;
	}

	void WDS::SetPollInterval(uint16_t interval)
	{
		*this->PollIntervall = interval;
	}

	uint16_t WDS::GetPollInterval()
	{
		return *this->PollIntervall;
	}

	void WDS::SetBCDfile(std::string file)
	{
		*this->bcdfile = file;
	}
private:
	uint8_t* NextAction;
	uint8_t* ActionDone;
	uint16_t* PollIntervall;
	uint16_t* RetryCount;
	uint32_t* requestid;
	std::string* bcdfile;
	uint32_t* referalIP;
	std::string* AdminMessage;
	bool* ServerSelection;
} WDS;

typedef struct DHCP_CLIENT
{
	DHCP_CLIENT()
	{
		this->wds = new WDS();
		this->rbcp = new RBCP();
		this->arch = new uint16_t(0);
		this->bootfile = new std::string("");
		this->msgtype = new uint8_t(1);
		this->NextServer = new uint32_t(0);
		this->isWDSRequest = new bool(false);
	}
	
	~DHCP_CLIENT()
	{
		delete this->wds;
		delete this->rbcp;
		delete this->arch;
		delete this->bootfile;
		delete this->msgtype;
		delete this->NextServer;
		delete this->isWDSRequest;
	}

	void DHCP_CLIENT::SetNextServer(uint32_t ip)
	{
		*this->NextServer = ip;
	}

	uint32_t DHCP_CLIENT::GetNextServer()
	{
		return *this->NextServer;
	}

	void DHCP_CLIENT::SetMessageType(uint8_t type)
	{
		*this->msgtype = type;
	}

	uint8_t DHCP_CLIENT::GetMessageType()
	{
		return *this->msgtype;
	}

	void DHCP_CLIENT::SetBootfile(std::string file)
	{
		*this->bootfile = file;
	}

	uint16_t DHCP_CLIENT::GetArchitecture()
	{
		return *this->arch;
	}

	void DHCP_CLIENT::SetArchitecture(CLIENT_ARCH arch)
	{
		*this->arch = arch;
	}
	
	bool DHCP_CLIENT::GetIsWDSRequest()
	{
		return *this->isWDSRequest;
	}

	void DHCP_CLIENT::SetIsWDSRequest(bool is)
	{
		*this->isWDSRequest = is;
	}

	std::string DHCP_CLIENT::GetBootfile(CLIENT_ARCH arch)
	{
		switch (arch)
		{
		case INTEL_X86:
			switch (this->wds->GetNextAction())
			{
			case ABORT:
				*this->bootfile = "Boot\\x86\\abortpxe.com";
				break;
			default:
				*this->bootfile = "Boot\\x86\\pxeboot.n12";
				//			this->bcdfile = "Boot\\x86\\default.bcd";
				break;
			}
			break;
		case INTEL_X64:
			switch (this->wds->GetNextAction())
			{
			case ABORT:
				*this->bootfile = "Boot\\x64\\abortpxe.com";
				//			this->bcdfile = "";
				break;
			default:
				*this->bootfile = "Boot\\x64\\pxeboot.n12";
				//			this->bcdfile = "Boot\\x64\\default.bcd";
				break;
			}
			break;
		case INTEL_EFI:
			*this->bootfile = "Boot\\efi\\bootmgfw.efi";
			//		this->bcdfile = "Boot\\efi\\default.bcd";
			break;
		default:
			*this->bootfile = "Boot\\x86\\wdsnbp.com";
			//		this->bcdfile = "";
			break;
		}

		return *this->bootfile;
	}
	WDS* wds;
	RBCP* rbcp;
private:
	bool* isWDSRequest;
	std::string* bootfile;
	uint16_t* arch;
	uint8_t* msgtype;
	uint32_t* NextServer;
} DHCP_CLIENT;

typedef struct TFTP_CLIENT
{
	void TFTP_CLIENT::SetTFTPState(uint8_t state)
	{
		*this->tftp_state = state;
	}

	uint8_t TFTP_CLIENT::GetTFTPState()
	{
		return *this->tftp_state;
	}

	void TFTP_CLIENT::SetBlock()
	{
		this->block = this->block + 1;
	}

	void TFTP_CLIENT::SetBlock(uint16_t block)
	{
		*this->block = block;
	}

	uint16_t TFTP_CLIENT::GetWindowSize()
	{
		return *this->windowsize;
	}

	void TFTP_CLIENT::SetWindowSize(uint16_t window)
	{
		*this->windowsize = window;
	}

	uint16_t TFTP_CLIENT::GetBlockSize()
	{
		return *this->blocksize;
	}

	void TFTP_CLIENT::SetBlockSize(uint16_t blocksize)
	{
		*this->blocksize = blocksize;
	}

	uint16_t TFTP_CLIENT::GetMSFTWindow()
	{
		return *this->msftwindow;
	}

	void TFTP_CLIENT::SetMSFTWindow(uint16_t window)
	{
		*this->msftwindow = window;
	}

	uint16_t TFTP_CLIENT::GetBlock()
	{
		return *this->block;
	}

	long TFTP_CLIENT::GetBytesToRead()
	{
		return *this->bytesToRead;
	}

	void TFTP_CLIENT::SetBytesToRead(long bytes)
	{
		*this->bytesToRead = bytes;
	}

	long TFTP_CLIENT::GetBytesRead()
	{
		return *this->bytesread;
	}

	void TFTP_CLIENT::SetBytesRead(long bytes)
	{
		if (bytes == 0)
			*this->bytesread = bytes;
		else
			*this->bytesread += bytes;
	}

	std::string TFTP_CLIENT::GetFilename()
	{
		return *this->filename;
	}

	void TFTP_CLIENT::SetFilename(std::string& filename)
	{
		*this->filename = filename;
	}

	TFTP_CLIENT()
	{
		this->block = new uint16_t(0);
		this->blocksize = new uint16_t(1456);
		this->bytesread = new long(0);
		this->bytesToRead = new long(0);
		this->msftwindow = new uint16_t(27182);
		this->retries = new uint8_t(0);
		this->tftp_state = new uint8_t(TFTP_Error);
		this->windowsize = new uint16_t(1);
		this->filename = new std::string("");
	}

	~TFTP_CLIENT() {
		delete this->filename;
		delete this->block;
		delete this->blocksize;
		delete this->bytesread;
		delete this->bytesToRead;
		delete this->msftwindow;
		delete this->retries;
		delete this->tftp_state;
		delete this->windowsize;
	}

private:
	uint8_t* retries;
	uint16_t* windowsize;
	uint16_t* msftwindow;
	uint16_t* block;
	uint16_t* blocksize;
	uint8_t* tftp_state;

	std::string* filename;

	long* bytesread;
	long* bytesToRead;
} TFTP_CLIENT;