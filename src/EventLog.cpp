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

using namespace std;

EventLog::EventLog(string context)
{
	this->SetContext(context);
	this->message = "";
	this->logtype = Info;
	this->LogBuffer = NULL;
}

EventLog::~EventLog()
{
	this->SetMsg("");
	this->SetContext("");
}

void EventLog::SetContext(string context)
{
	if (context.size() > 3)
		this->context = context;
}

void EventLog::SetMsg(string message)
{
	if (message.size() > 3)
		this->message = message;
}

string EventLog::GetMsg()
{
	return this->message;
}

string EventLog::GetContext()
{
	return this->context;
}

void EventLog::WriteLine()
{
	string prefix = "";

	if (this->context.size() > 3)
	{
		switch (this->logtype)
		{
		case Info:
#ifdef _WIN32
			if (!SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xf))
				this->Report(Debug, "Color Change not supported!");
#endif
			prefix = "[I] ";
			break;

		case Warn:
#ifdef _WIN32
			if (!SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xe))
				this->Report(Debug, "Color Change not supported!\n");
#endif
			prefix = "[W] ";
			break;

		case Error:
#ifdef _WIN32
			if (!SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xc))
				this->Report(Debug, "Color Change not supported!");
#endif
			prefix = "[E] ";
			break;
#ifdef _DEBUG
		case Debug:
			prefix = "[D] ";
			break;
#endif
		default:
			return;
		}
	
		cout << prefix << this->GetContext() << ": " << this->GetMsg() << endl;
#ifdef _WIN32
		if (!SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xf))
			this->Report(Debug, "Color Change not supported!");
#endif
	}
}

void EventLog::Report(LogType type, string message)
{
	this->SetMsg(message);
	this->logtype = type;

	this->WriteLine();
}
