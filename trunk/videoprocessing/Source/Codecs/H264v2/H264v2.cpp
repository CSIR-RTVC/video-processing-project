/** @file

MODULE				: H264

TAG					: H264

FILE NAME			: H264v2.cpp

DESCRIPTION			: H264v2Factory implementation	

LICENSE	: GNU Lesser General Public License

Copyright (c) 2008 - 2012, CSIR
All rights reserved.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/
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
