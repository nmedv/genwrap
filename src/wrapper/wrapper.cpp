#include <cstdio>
#include <Windows.h>

#include "genwrap/wrapper_internal.h"


namespace Lake {

static WrapperError wrap_errbuf_global;


int32 Wrapper::SetError(int32 code, const char* msg)
{
	wrap_errbuf_global.code = code;

	char buf[256];

	if (!FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf,
		(sizeof(buf) / sizeof(wchar_t)), NULL))
		return 0;

	if (msg)
		snprintf(wrap_errbuf_global.str, LK_WRAP_ERROR_MAX_STR_SIZE, "%s: \"%s\"", buf, msg);
	else
	{
		memset(wrap_errbuf_global.str, 0, 256);
		memcpy(wrap_errbuf_global.str, buf, strlen(buf));
	}

	return 0;
}


WrapperError* Wrapper::GetError()
{
	if (wrap_errbuf_global.code)
		return &wrap_errbuf_global;
	else
		return 0;
}


int32 Wrapper::UpdateModule(Module module)
{
	WrapperModule m = (WrapperModule)module;

	void* ptr;
	for (int32 i = 0; i < m->count; i++)
	{
		ptr = (void*)GetProcAddress((HMODULE)m->handle, m->procs[i].name);
		if (!ptr)
		{
			Wrapper::SetError(-2, m->procs[i].name);
			UnloadModule(module);
			return 0;
		}
		else
			m->procs[i].ptr = ptr;
	}

	return 1;
}


int32 Wrapper::LoadModule(Module module)
{
	WrapperModule m = (WrapperModule)module;

	m->handle = (void*)LoadLibraryA(m->name);
	if (!m->handle)
		return Wrapper::SetError(-1, m->name);

	return UpdateModule(module);
}


int32 Wrapper::UnloadModule(Module module)
{
	if (FreeLibrary((HMODULE)(((WrapperModule)module)->handle)))
		return 1;
	else
		return Wrapper::SetError(-3, ((WrapperModule)module)->name);
}


void* Wrapper::GetProcAddr(const char* procName, Module module)
{
	void* res = (void*)GetProcAddress((HMODULE)((WrapperModule)module)->handle, procName);
	if (!res)
		Wrapper::SetError(-2, procName);

	return res;
}


}