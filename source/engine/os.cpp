#include "os.h"
#include <Windows.h>

String get_executable_name()
{
	const size_t max_len = MAX_PATH + 1;
	char exe_file[max_len];
	size_t len = GetModuleFileName(NULL, exe_file, max_len);

	// error case
	if (len == 0 || len == max_len)
	{
		return String{};
	}
	else
	{
		return String{exe_file, len};
	}
}
