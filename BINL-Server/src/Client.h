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
	Client(ServerType type, string* id, char* hwaddr);
	~Client();

	void SetWDSMessage(string message);
	string GetWDSMessage();

	void SetRequestID(uint32_t id);
	uint32_t GetRequestID();

	void SetNextAction(WDSNBP_OPTION_NEXTACTION action);
	WDSNBP_OPTION_NEXTACTION GetNextAction();

	void SetRetryCount(uint16_t action);
	uint16_t GetRetryCount();

	void SetPollInterval(uint16_t interval);
	uint16_t GetPollInterval();

	void SetType(ServerType type);
	ServerType GetType();

	void SetDHCPMessageType(DHCPMsgTypes type);
	DHCPMsgTypes GetDHCPMessageType();

	CLIENT_ARCH GetArchitecture();
	void SetArchitecture(CLIENT_ARCH arch);

	void SetBootfile(string file);
	string GetBootfile(CLIENT_ARCH arch);

	void SetBCDfile(string file);
	string GetBCDfile();

	void SetReferralServer(uint32_t addr);
	uint32_t GetReferralServer();
#ifdef WITH_TFTP
#ifdef VARWIN
	bool AllowVariableWindowSize;
#endif
	void SetTFTPState(TFTP_State state);
	TFTP_State GetTFTPState();

	void SetBlock();
	void SetBlock(uint16_t block);
	uint16_t GetBlock();

	void SetWindowSize(uint16_t window);
	uint16_t GetWindowSize();

	void SetBlockSize(uint16_t blocksize);
	uint16_t GetBlockSize();

#ifdef VARWIN
	void SetMSFTWindow(uint16_t window);
	uint16_t GetMSFTWindow();
#endif
	
	bool GetACK(Packet* packet);
	uint8_t retries;
	long last_bytesRead;
#endif

	void SetBytesToRead(long bytes);
	long GetBytesToRead();

	void SetBytesRead(long bytes);
	long GetBytesRead();

	void SetSocket(SOCKET* s);
	SOCKET* GetSocket();
	
	string AdminMessage;
	bool ServerSelection;
	bool isWDSRequest;
	Packet* Data;
	string* id;
	int http_status;
	string http_description;

	WDSNBP_OPTION_NEXTACTION NextAction;
	uint8_t ActionDone;

	uint16_t PollIntervall;
	uint16_t RetryCount;
	
	uint32_t NextServer;
	uint32_t referralIP;

#ifdef WITH_TFTP
	uint16_t last_block;
	uint16_t max_blocksize;
#endif
	FileSystem* file;
private:
#ifdef WITH_TFTP
	uint16_t windowsize;
#ifdef VARWIN
	uint16_t msftwindow;
#endif
	uint16_t block;
	uint16_t blocksize;

	TFTP_State tftp_state;
#endif
	uint32_t requestid;
	long bytesread;

	long bytesToRead;
	string bootfile;
	string bcdfile;
	ServerType type;
	CLIENT_ARCH arch;
	DHCPMsgTypes msgtype;
	SOCKET* s;
};
