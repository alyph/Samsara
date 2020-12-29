#include "fileio.h"
#include "filesystem.h"

void FileTokenReadStream::open(const String& path)
{
	*this = {};
	File file;
	file.open(path, FileMode::read);
	if (file.fp)
	{
		const auto size = filesystem::file_size(path);
		buffer.reserve(size + 64); // add a little padding and make sure the file content is smaller than the buffer size
		size_t read_size = file.read(buffer.data, buffer.capacity());
		asserts(read_size < buffer.capacity());
		buffer.data[read_size] = 0;
		buffer.resize(read_size);
	}
}






