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

class FileSystem
{
public:
	FileSystem(string filename, FileOpenMode mode);
	~FileSystem();

	bool Exist();
	
	size_t Write(const char* data, size_t length, size_t byteswritten, long seek);
	size_t Read(char* dest, size_t dest_offset, long seek, size_t length);
	string Name();
	long Length();
	string CType();
	void Close();

private:
	string ResolvePath(string path);
	bool Open();
	bool isOpen;
	string* ctype;

	long filesize;

	FILE* file;
	string* filename;
	FileOpenMode mode;
};
