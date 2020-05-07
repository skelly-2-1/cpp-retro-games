/*
@file

    trace.h

@purpose

    Debugging for NS (for now)
*/

#ifdef PLATFORM_NS
#include "macros.h"

#ifndef NS_ENABLE_NXLINK
#define TRACE(fmt,...) ((void)0)
#else
#include <unistd.h>
#define TRACE(fmt,...) printf("%s: " fmt "\n", __PRETTY_FUNCTION__, ## __VA_ARGS__)
#endif
#endif