#ifndef __trace_h__
#define __trace_h__

#include <stdio.h>

#ifndef _NDEBUG
#	define trace(X) std::cout << X;
#else
#	define trace(X)
#endif // _NDEBUG


#endif // __trace_h__
