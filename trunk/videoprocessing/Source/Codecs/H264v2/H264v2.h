// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the H264V2_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// H264V2_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#pragma once

#ifdef H264V2_EXPORTS
#define H264V2_API __declspec(dllexport)
#else
#define H264V2_API __declspec(dllimport)
#endif

/// This dll has only one purpose to instantiate a H264v2Codec instance. The
/// obligation of scope management is left to the calling functions.
#include "H264v2Codec.h"

// This class is exported from the H264v2.dll
class H264V2_API H264v2Factory 
{
public:
	H264v2Factory(void);
	~H264v2Factory(void);

	/// Interface.
	H264v2Codec* GetCodecInstance(void);
	void				 ReleaseCodecInstance(ICodecv2* pInst);
};	///end H264v2Factory.
