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
	this->SetMsg("");
	this->logtype = Info;
	this->LogBuffer = NULL;
}

EventLog::~EventLog()
{
	delete this->context;
	delete this->message;
}

void EventLog::SetContext(string context)
{
	if (context.size() > 3)
		this->context = new string(context);
}

void EventLog::SetMsg(string message)
{
	if (message.size() > 3)
		this->message = new string(message);
}

string EventLog::GetMsg()
{
	return *this->message;
}

string EventLog::GetContext()
{
	return *this->context;
}

void EventLog::WriteLine()
{
	string* prefix;

	if (this->context->size() > 3)
	{
		switch (this->logtype)
		{
		case Info:
			prefix = new string("[I]");
			break;

		case Warn:
			prefix = new string("[W]");
			break;

		case Error:
			prefix = new string("[E]");
			break;
#ifdef DEBUG
		case Debug:
			prefix = new string("[D]");
			break;
#endif
		default:
			return;
		}
	
		cout << *prefix << " " << this->GetContext() << ": " << this->GetMsg() << endl;

		delete prefix;
	}
}

void EventLog::Report(LogType type, string message)
{
	this->SetMsg(message);
	this->logtype = type;

	this->WriteLine();
}
