#include <cstdio>
#include <cstring>

#include "genwrap/genwrap.h"
#include "utl/getopt.h"
#include "utl/error.h"


static const char* gw_ver = "genwrap version 0.1 beta";
static const char* gw_usage = "genwrap <library> [options]";
static const char* gw_help[] = {
	"Shared library stub generator\n",

	"options:",
	"  -h, --help       Print help and exit",
	"  -v, --version    Print version and exit",
	"  -o, --output     Output files (stdout by default)",
	"  -n, --namespace  Namespace (library name by default)",
	0
};

static const char* gw_opts = ":hvo:n:";
static const utl_getopt_option gw_long_opts[] =
{
	{"help",	  0, 0, 'h'},
	{"version",	  0, 0, 'v'},
	{"output",	  1, 0, 'o'},
	{"namespace", 1, 0, 'n'},
	{0,           0, 0,  0 }
};


static void split(char* src, char delim, char** out, int len)
{
	char* ptr_old = src;
	char* ptr = src;
	
	for (int i = 0; i < len; i++)
	{
		ptr = strchr(ptr, delim);
		if (!ptr)
		{
			out[i] = ptr_old;
			break;
		}
		
		*ptr++ = 0;
		out[i] = ptr_old;
		ptr_old = ptr;
	}
}


static int gw_parse_args(int argc, char* argv[], gw_args& args)
{
	char opt;
	int longindex;
	int i = 0;
	utl_opt_state state = { 1, 1, 0, 0, 0 };

	while ((opt = utl_getopt_long(argc, argv, gw_opts, gw_long_opts, &longindex, &state)) != -1) {
		switch (opt) {
		case 'h':
			printf("usage: %s\n\n", gw_usage);
			while (gw_help[i] != 0)
				puts(gw_help[i++]);
			args.info = 1;
			return 1;
		case 'v':
			puts(gw_ver);
			args.info = 1;
			return 1;
		case 'n': args.name = state.optarg; break;
		case 'o':
			split(state.optarg, ',', &(args.output_cpp), 2);
			break;
		case '?':
			if (state.optopt)
				return utl_set_error(GW_ERROR_BAD_ARGS, "unknown option \"-%c\"", state.optopt);
			else
				return utl_set_error(GW_ERROR_BAD_ARGS, "unknown option \"%s\"", argv[state.optind - 1]);
		case ':':
			return utl_set_error(GW_ERROR_BAD_ARGS, "option \"-%c\" requires an argument", state.optopt);
		case '*':
			if (!args.library)
				args.library = argv[state.argind];
			else
				return utl_set_error(GW_ERROR_BAD_ARGS, "unknown no-option argument \"%s\"", argv[state.argind]);
			break;
		default: return 0;
		}
	}

	if (!args.library)
		return utl_set_error(GW_ERROR_BAD_ARGS, "\"name\" argument is required", gw_usage);

	return 1;
}


static inline int gw_print_error()
{
	const utl_error* err = utl_get_error();
	fprintf(stderr, "genwrap: error: %s\n", err->str);
	return err->code;
}


int gw_main(char* argv[], gw_args* args);

int main(int argc, char* argv[])
{
	gw_args args = { 0 };
	if (!gw_parse_args(argc, argv, args))
		return gw_print_error();
	
	if (!args.info && !gw_main(argv, &args))
		return gw_print_error();
	
	return 0;
}