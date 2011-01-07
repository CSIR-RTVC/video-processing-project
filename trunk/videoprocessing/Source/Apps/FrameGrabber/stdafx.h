// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <Windows.h>

#include <stdio.h>
#include <tchar.h>

#pragma warning(push)     // disable for this header only
#pragma warning(disable:4312)
// DirectShow
#include <streams.h>
#pragma warning(pop)      // restore original warning level


// TODO: reference additional headers your program requires here
#include <Shared/CommonDefs.h>

// Define _BUILD_FOR_SHORT to use shorts for YUV image handling
// This setting needs to be defined consistently for this project and 
// Image static library, and the RGB YUV conversion filters 
#define _BUILD_FOR_SHORT
