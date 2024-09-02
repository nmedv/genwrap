#ifndef _WRAPPER_INTERNAL_H_
#define _WRAPPER_INTERNAL_H_

#include "Lake/Wrapper.h"


namespace Lake {

typedef struct {
	void*       ptr;
	const char* name;
} *WrapperProc;

typedef struct {
	void*       handle;
	const char* name;
	int32       count;
	WrapperProc procs;
} *WrapperModule;

}


#endif // !_WRAPPER_INTERNAL_H_