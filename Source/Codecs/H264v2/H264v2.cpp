// H264v2.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "H264v2.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

H264v2Factory::H264v2Factory(void)
{
}//end constructor.

H264v2Factory::~H264v2Factory(void)
{
}//end destructor.

H264v2Codec* H264v2Factory::GetCodecInstance(void)
{
	return(new H264v2Codec());
}//end GetCodecInstance.

void H264v2Factory::ReleaseCodecInstance(ICodecv2* pInst)
{
	if(pInst != NULL)
	{
		delete pInst;
		pInst = NULL;
	}//end if pInst...
}//end ReleaseCodecInstance.
