#ifndef _GENWRAP_H_
#define _GENWRAP_H_

typedef struct {
	int		info;
	char*	library;
	char*	output_cpp;
	char*	output_asm;
	char*	name;
} gw_args;

enum gw_errors_e {
	GW_ERROR_BAD_ARGS = 2,
	GW_ERROR_NO_FILE,
	GW_ERROR_BAD_FILE,
	GW_ERROR_BAD_SHARED
};

#endif