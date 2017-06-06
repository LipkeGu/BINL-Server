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

#define _CRT_SECURE_NO_WARNINGS
#include "Includes.h"

FileSystem::FileSystem(string filename, FileOpenMode mode)
{
	this->mode = mode;
	this->filesize = 0;
	this->filename = this->ResolvePath(filename);
	this->isOpen = this->Open();
	this->ctype = "";

	if (this->isOpen)
	{
		if (this->filename.find(".css") != string::npos)
			this->ctype = "text/css";

		if (this->filename.find(".htm") != string::npos)
			this->ctype = "text/html";

		if (this->filename.find(".wim") != string::npos)
			this->ctype = "application/octet-stream";

		if (this->filename.find(".sdi") != string::npos)
			this->ctype = "application/octet-stream";
	}
}

bool FileSystem::Exist()
{
	return this->isOpen;
}

string FileSystem::CType()
{
	return this->ctype;
}

bool FileSystem::Open()
{
	switch (this->mode)
	{
	case FileWrite:
		this->file = fopen(this->filename.c_str(), "w");
		break;
	case FileRead:
		this->file = fopen(this->filename.c_str(), "r");
		break;
	case FileReadBinary:
		this->file = fopen(this->filename.c_str(), "rb");
		break;
	case FileWriteBinary:
		this->file = fopen(this->filename.c_str(), "wb");
		break;
	default:
		break;
	}

	if (this->file != NULL)
	{
		if (fseek(this->file, 0, SEEK_END) == 0 && this->mode == FileRead)
		{
			this->filesize = ftell(this->file);
			rewind(this->file);

			return true;
		}
		else
			return false;
	}
	else
		return false;
}

string FileSystem::ResolvePath(string path)
{
	string res = "./";

	if (path.find_first_of('\\') == 0 || path.find_first_of('/') == 0)
	{
		res = "./" + path.substr(1, path.length() - 1);
#ifdef _WIN32
		res = replace(res, "/", "\\");
#else
		res = replace(res, "\\", "/");
#endif
	}
	else
	{
		res = "./" + path;
#ifdef _WIN32
		res = replace(res, "//", "/");
		res = replace(res, "/", "\\");
#else
		res = replace(res, "\\\\", "\\");
		res = replace(res, "\\", "/");
#endif
	}

	return res;
}

size_t FileSystem::Write(const char* data, size_t length, size_t byteswritten, long seek)
{
	size_t res = 0;

	if (!this->isOpen)
		return res;

	if (length > 0 && this->mode == FileWrite)
	{
		if (fseek(this->file, seek, SEEK_END) == 0)
		{
			byteswritten += fwrite(data, 1, length, this->file);
			res = byteswritten;
		}
	}

	return res;
}

long FileSystem::Length()
{
	return this->filesize;
}

string FileSystem::Name()
{
	return this->filename;
}

size_t FileSystem::Read(char* dest, size_t dest_offset, long seek, size_t length)
{
	size_t res = 0;

	if (!this->isOpen)
		return res;

	if (fseek(this->file, seek, SEEK_SET) == 0 && this->mode == FileRead)
		res = fread(&dest[dest_offset], 1, length, this->file);

	return res;
}

void FileSystem::Close()
{
	if (this->isOpen)
	{
		fclose(this->file);
		this->filesize = 0;
	}
}

FileSystem::~FileSystem()
{
	this->Close();
}
