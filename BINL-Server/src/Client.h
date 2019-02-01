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
	Client(ServerType type, std::string* id, char* hwaddr);
	~Client();

	struct DHCP_CLIENT {
		struct WDS {
			void SetWDSMessage(std::string message);
			std::string GetWDSMessage();

			void SetRequestID(uint32_t id);
			uint32_t GetRequestID();

			void SetNextAction(WDSNBP_OPTION_NEXTACTION action);
			WDSNBP_OPTION_NEXTACTION GetNextAction();

			void SetRetryCount(uint16_t action);
			uint16_t GetRetryCount();

			void SetPollInterval(uint16_t interval);
			uint16_t GetPollInterval();

			void SetReferralServer(uint32_t addr);
			uint32_t GetReferralServer();

			WDSNBP_OPTION_NEXTACTION NextAction = APPROVAL;
			uint8_t ActionDone = 0;
			uint16_t PollIntervall = 10;
			uint16_t RetryCount = 10;
			uint32_t requestid = 1;
			
			uint32_t referralIP = 0;
			std::string AdminMessage = "";
			bool ServerSelection = false;
		} WDS;

		struct RBCP {

		};

		void SetDHCPMessageType(DHCPMsgTypes type);
		DHCPMsgTypes GetDHCPMessageType();

		CLIENT_ARCH GetArchitecture();
		void SetArchitecture(CLIENT_ARCH arch);

		void SetBootfile(std::string file);
		std::string GetBootfile(CLIENT_ARCH arch);

		std::string bootfile = "";
		CLIENT_ARCH arch = INTEL_X86;
		DHCPMsgTypes msgtype = DHCP_DIS;
		uint32_t NextServer = 0;
	} DHCP_CLIENT;

	struct TFTP_CLIENT {
		void SetTFTPState(TFTP_State state);
		TFTP_State GetTFTPState();

		void SetBlock();
		void SetBlock(uint16_t block);
		uint16_t GetBlock();

		void SetWindowSize(uint16_t window);
		uint16_t GetWindowSize();

		void SetBlockSize(uint16_t blocksize);
		uint16_t GetBlockSize();

		void SetMSFTWindow(uint16_t window);
		uint16_t GetMSFTWindow();

		bool GetACK(Packet* packet);

		void SetBytesToRead(long bytes);
		long GetBytesToRead();

		void SetBytesRead(long bytes);
		long GetBytesRead();
	private:
		uint8_t retries = 0;
		long last_bytesRead = 0;
		uint16_t last_block = 0;
		uint16_t max_blocksize = 8192;
		uint16_t windowsize = 1;
		uint16_t msftwindow = 27182;
		uint16_t block = 0;
		uint16_t blocksize = 1456;
		TFTP_State tftp_state = TFTP_Error;

		long bytesread = 0;
		long bytesToRead = 0;
	} TFTP_CLIENT;

	void SetType(ServerType type);
	ServerType GetType();

	void SetBCDfile(std::string file);
	std::string GetBCDfile();

	void SetSocket(SOCKET* s);
	SOCKET* GetSocket();
	
	bool isWDSRequest = false;
	Packet* Data = nullptr;
	std::string* id = nullptr;
	FileSystem* file = nullptr;
private:
	std::string bcdfile ="";
	ServerType type;

	SOCKET* s = nullptr;
};
