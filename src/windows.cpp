#include <cstdio>
#include <Shlwapi.h>

#include "genwrap/genwrap.h"
#include "genwrap/windows.h"
#include "utl/error.h"


static void printgle(char const* msg)
{
	char const* buf = 0;
	u32 nchars =
		FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			0, GetLastError(),
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			(char*)&buf, 0, 0
		);

	if (!nchars)
		buf = "(could not format error message)";

	utl_set_error(GW_ERROR_BAD_FILE, "%s: %s", msg, buf);
}


/* doesn't need to be unmappable for my usage */
void* os_mmap_file(const char* path)
{
	void* res;
	HANDLE hfile;
	HANDLE hmapping;

	hfile = CreateFileA(
		path,
		GENERIC_READ,
		FILE_SHARE_READ,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0
	);

	if (hfile == INVALID_HANDLE_VALUE)
	{
		printgle("CreateFileA");
		return 0;
	}

	hmapping = CreateFileMapping(hfile, 0, PAGE_READONLY, 0, 0, 0);

	if (!hmapping)
	{
		printgle("CreateFileMapping");
		CloseHandle(hfile);
		return 0;
	}

	res = MapViewOfFile(hmapping, FILE_MAP_READ, 0, 0, 0);

	if (!res) {
		printgle("MapViewOfFile");
	}

	return res;
}