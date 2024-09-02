#include <cstdlib>
#include <cstdio>
#include <ctype.h>
#include <cstring>
#include <malloc.h>
#include <vector>

#include "utl/error.h"
#include "genwrap/windows.h"
#include "genwrap/genwrap.h"


static const char* get_file_name_with_ext(const char* src)
{
	const char* res;
	const char* last_slash = strrchr(src, '/');
	const char* last_backslash = strrchr(src, '\\');

	if (last_slash > last_backslash)
		res = ++last_slash;
	else if (last_backslash > last_slash)
		res = ++last_backslash;
	else
		res = src;
	
	return res;
}


static void get_file_name(const char* src, char* out)
{
	const char* start = get_file_name_with_ext(src);

	const char* end = strchr(start, '.');
	if (!end)
		end = start + strlen(start);

	size_t len = end - start;
	memcpy(out, start, len);
}


static u8* pe_buf;
static image_nt_headers32* nt_hdr32 = 0;
static image_nt_headers64* nt_hdr64 = 0;

static void* pe_map_va(u32 va)
{
	u16 n;
	image_section_header* s;
	void* res = 0;

	if (nt_hdr32)
	{
		n = nt_hdr32->file_header.number_of_sections;

		s = (image_section_header*)
			((u8*)&nt_hdr32->optional_header +
			nt_hdr32->file_header.size_of_optional_header);
	}
	else
	{
		n = nt_hdr64->file_header.number_of_sections;

		s = (image_section_header*)
			((u8*)&nt_hdr64->optional_header +
			nt_hdr64->file_header.size_of_optional_header);

	}

	for (; n; --n, ++s)
	{
		if (va < s->virtual_address || va >= s->virtual_address + s->misc.virtual_size)
			continue;
		
		va -= s->virtual_address;
		va += s->pointer_to_raw_data;

		res = pe_buf + va;
		break;
	}

	if (!res)
	{
		utl_set_error(GW_ERROR_BAD_SHARED, "ABUNAI: invalid virtual address");
		return 0;
	}

	return res;
}

static image_export_directory* pe_exports(const char* file)
{
	image_dos_header* dos_hdr;
	image_data_directory* dir;

	/*
		should allow the OS to do copy on write if the dll is being
		used by at least one program
	*/
	pe_buf = (u8*)os_mmap_file(file);
	if (!pe_buf)
		return 0;

	dos_hdr = (image_dos_header*)pe_buf;

	if (dos_hdr->e_magic != 0x5a4d)
	{
		utl_set_error(GW_ERROR_BAD_SHARED, "invalid DOS header in \"%s\"", file);
		return 0;
	}

	nt_hdr32 = (image_nt_headers32*)(pe_buf + dos_hdr->e_lfanew);

	if (nt_hdr32->signature != 0x00004550)
	{
		utl_set_error(GW_ERROR_BAD_SHARED, "invalid NT header in \"%s\"", file);
		return 0;
	}

	if (nt_hdr32->optional_header.magic == 0x10b || nt_hdr32->optional_header.magic == 0x107)
		goto foundheader;

	nt_hdr64 = (image_nt_headers64*)nt_hdr32;
	nt_hdr32 = 0;

	if (nt_hdr64->optional_header.magic != 0x20b)
	{
		utl_set_error(GW_ERROR_BAD_SHARED, "invalid optional header in \"%s\"", file);
		return 0;
	}

foundheader:
	if (nt_hdr32)
		dir = nt_hdr32->optional_header.data_directory;
	else
		dir = nt_hdr64->optional_header.data_directory;

	return (image_export_directory*)pe_map_va(dir->virtual_address);
}


static FILE* try_open_file(const char* fname)
{
	FILE* res = fopen(fname, "w");
	if (!res)
	{
		utl_set_error(GW_ERROR_BAD_FILE, "error while writing file \"%s\"", fname);
		return 0;
	}
	
	return res;
}


static FILE* fout_cpp;
static FILE* fout_asm;
static bool fout_is_stdout = false;


static void get_names(image_export_directory const* exports, std::vector<const char*>* out)
{
	u32* name_offsets = (u32*)pe_map_va(exports->address_of_names);

	for (u32 i = 0; i < exports->number_of_names; ++i)
		out->push_back((const char*)pe_map_va(name_offsets[i]));
}


static void gen_cpp(gw_args* args, std::vector<const char*>* names)
{
	char* library_name = (char*)calloc(strlen(args->library), 1);
	get_file_name(args->library, library_name);
	size_t procs_count = names->size();

	char* file = args->output_cpp;
	if (!file) file = "";

	fprintf(fout_cpp,
		"// %s\n"
		"// Generated automatically using genwrap\n\n"

		"typedef struct {\n"
		"	void*       ptr;\n"
		"	const char* name;\n"
		"} Proc;\n\n"

		"typedef struct {\n"
		"	void*       handle;\n"
		"	const char* name;\n"
		"	int         count;\n"
		"	Proc*       procs;\n"
		"} Module;\n\n"
		
		"extern \"C\" Proc %s_procs[%lld] = {\n",
		file, library_name, procs_count
	);

	for (size_t i = 0; i < procs_count; i++)
	{
		fprintf(fout_cpp, "\t{ 0, \"%s\" }", names->at(i));
		if (i != procs_count - 1)
			fputs(",\n", fout_cpp);
		else
			fputs("\n", fout_cpp);
	}

	fprintf(fout_cpp,
		"};\n\n"
		
		"Module %s_module = {\n"
		"	0,\n"
		"	\"%s\",\n"
		"	%lld,\n"
		"	%s_procs\n"
		"};\n\n"
		
		"namespace %s {\n\n"

		"void* GetModule()\n"
		"{\n"
		"	return (void*)(&%s_module);\n"
		"}\n\n"

		"}\n",
		library_name, get_file_name_with_ext(args->library), procs_count,
		library_name, args->name, library_name
	);

	free(library_name);
}


static void gen_asm(gw_args* args, std::vector<const char*>* names)
{
	char* library_name = (char*)calloc(strlen(args->library), 1);
	get_file_name(args->library, library_name);

	char* file = args->output_asm;
	if (!file) file = "";

	fprintf(fout_asm,
		"; %s\n"
		"; Generated automatically using genwrap\n\n"

		"EXTERN %s_procs:QWORD\n"
		"_TEXT SEGMENT\n\n",
		file, library_name
	);

	for (int i = 0; i < names->size(); i++)
	{
		const char* name = names->at(i);
		fprintf(fout_asm,
			"%s PROC\n"
			"jmp [%s_procs + %d * 16]\n"
			"%s ENDP\n\n",
			name, library_name, i, name
		);
	}

	fputs(
		"_TEXT ENDS\n"
		"END",
		fout_asm
	);

	free(library_name);
}


int gw_main(char* argv[], gw_args* args)
{
	int res = 0;
	std::vector<const char*> names;
	size_t output_cpp_len;
	char* output_asm;
	size_t library_len = strlen(args->library);
	char* name = (char*)_malloca(library_len);
	memset(name, 0, library_len);
	
	if (args->output_cpp)
	{
		output_cpp_len = strlen(args->output_cpp);
		output_asm = (char*)_malloca(output_cpp_len + 5);

		if (!(fout_cpp = try_open_file(args->output_cpp)))
			return 0;
		
		if (!args->output_asm)
		{
			args->output_asm = output_asm;
			memcpy(args->output_asm, args->output_cpp, output_cpp_len);
			memcpy(args->output_asm + output_cpp_len, ".asm\0", 5);
		}
		
		if (!(fout_asm = try_open_file(args->output_asm)))
			return 0;
	}
	else
	{
		fout_cpp = stdout;
		fout_asm = stdout;
		fout_is_stdout = true;
	}

	if (!args->name)
	{
		args->name = name;
		get_file_name(args->library, args->name);
	}
	
	image_export_directory* exports = pe_exports(args->library);
	if (!exports)
		goto end;
	
	get_names(exports, &names);
	gen_cpp(args, &names);
	gen_asm(args, &names);
	res = 1;

end:
	if (args->output_asm)
		_freea(output_asm);
	_freea(name);

	if (!fout_is_stdout)
	{
		fclose(fout_cpp);
		fclose(fout_asm);
	}

	return res;
}
