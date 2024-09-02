#include <Windows.h>

#define os_basename PathFindFileNameA
#define os_devnull() fopen("NUL:", "w")

#define OS_MAX_PATH MAX_PATH


typedef LONGLONG i64;
typedef INT      i32;
typedef SHORT    i16;
typedef CHAR     i8;

typedef ULONGLONG u64;
typedef DWORD     u32;
typedef WORD      u16;
typedef BYTE      u8;


#pragma pack(push, 4)
typedef struct {
    u16 machine;
    u16 number_of_sections;
    u32 time_date_stamp;
    u32 pointer_to_symbol_table;
    u32 number_of_symbols;
    u16 size_of_optional_header;
    u16 characteristics;
} image_file_header;

typedef struct {
    u32 virtual_address;
    u32 size;
} image_data_directory;

typedef struct {
    u16 magic; /* 0x10b or 0x107 */
    u8 major_linker_version;
    u8 minor_linker_version;
    u32 size_of_code;
    u32 size_of_initialized_data;
    u32 size_of_uninitialized_data;
    u32 address_of_entry_point;
    u32 base_of_code;
    u32 base_of_data;
    u32 image_base;
    u32 section_alignment;
    u32 file_alignment;
    u16 major_operating_system_version;
    u16 minor_operating_system_version;
    u16 major_image_version;
    u16 minor_image_version;
    u16 major_subsystem_version;
    u16 minor_subsystem_version;
    u32 win32_version_value;
    u32 size_of_image;
    u32 size_of_headers;
    u32 check_sum;
    u16 subsystem;
    u16 dll_characteristics;
    u32 size_of_stack_reserve;
    u32 size_of_stack_commit;
    u32 size_of_heap_reserve;
    u32 size_of_heap_commit;
    u32 loader_flags;
    u32 number_of_rva_and_sizes;
    image_data_directory data_directory[16];
} image_optional_header32;

typedef struct {
    u32 signature; /* 0x00004550 */
    image_file_header file_header;
    image_optional_header32 optional_header;
} image_nt_headers32;

typedef struct {
    u32 characteristics;
    u32 time_date_stamp;
    u16 major_version;
    u16 minor_version;
    u32 name;
    u32 base;
    u32 number_of_functions;
    u32 number_of_names;
    u32 address_of_functions;
    u32 address_of_names;
    u32 address_of_name_ordinals;
} image_export_directory;
#pragma pack(pop)

#pragma pack(push, 8)
typedef struct {
    u16 magic; /* 0x20b */
    u8 major_linker_version;
    u8 minor_linker_version;
    u32 size_of_code;
    u32 size_of_initialized_data;
    u32 size_of_uninitialized_data;
    u32 address_of_entry_point;
    u32 base_of_code;
    u64 image_base;
    u32 section_alignment;
    u32 file_alignment;
    u16 major_operating_system_version;
    u16 minor_operating_system_version;
    u16 major_image_version;
    u16 minor_image_version;
    u16 major_subsystem_version;
    u16 minor_subsystem_version;
    u32 win32_version_value;
    u32 size_of_image;
    u32 size_of_headers;
    u32 check_sum;
    u16 subsystem;
    u16 dll_characteristics;
    u64 size_of_stack_reserve;
    u64 size_of_stack_commit;
    u64 size_of_heap_reserve;
    u64 size_of_heap_commit;
    u32 loader_flags;
    u32 number_of_rva_and_sizes;
    image_data_directory data_directory[16];
} image_optional_header64;

typedef struct {
    u32 signature; /* 0x00004550 */
    image_file_header file_header;
    image_optional_header64 optional_header;
} image_nt_headers64;
#pragma pack(pop)

#pragma pack(push, 2)
typedef struct {
    u16 e_magic; /* 5A4D */
    u16 e_cblp;
    u16 e_cp;
    u16 e_crlc;
    u16 e_cparhdr;
    u16 e_minalloc;
    u16 e_maxalloc;
    u16 e_ss;
    u16 e_sp;
    u16 e_csum;
    u16 e_ip;
    u16 e_cs;
    u16 e_lfarlc;
    u16 e_ovno;
    u16 e_res[4];
    u16 e_oemid;
    u16 e_oeminfo;
    u16 e_res2[10];
    i32 e_lfanew;
} image_dos_header;

typedef struct {
    char name[8];

    union {
        u32 physical_address;
        u32 virtual_size;
    } misc;

    u32 virtual_address;
    u32 size_of_raw_data;
    u32 pointer_to_raw_data;
    u32 pointer_to_relocations;
    u32 pointer_to_linenumbers;
    u16 number_of_relocations;
    u16 number_of_linenumbers;
    u32 characteristics;
} image_section_header;
#pragma pack(pop)


void* os_mmap_file(char const* path);
void printgle(char const* msg);