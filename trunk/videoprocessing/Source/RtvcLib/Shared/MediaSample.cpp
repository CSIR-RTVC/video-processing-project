/** @file

MODULE				: MediaSample

FILE NAME			: MediaSample.cpp

DESCRIPTION			: 
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008, CSIR
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the CSIR nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===========================================================================
*/
#include "MediaSample.h"

#include <memory>

MediaSample::MediaSample( BYTE* pData, int nSize, double dStartTime, bool bIsSyncPoint)
	: m_nSize(nSize),
	m_dStartTime(dStartTime),
	m_bIsSynchronisationPoint(bIsSyncPoint)
{
	// Copy the data
	m_pData = new BYTE[nSize + 1];
	m_pData[nSize] = '\0';
	memcpy(m_pData, pData, m_nSize);
}

MediaSample::MediaSample( const MediaSample& rSample )
{
	m_nSize = rSample.m_nSize;
	m_dStartTime = rSample.m_dStartTime;
	m_bIsSynchronisationPoint = rSample.m_bIsSynchronisationPoint;
	m_pData = new BYTE[m_nSize + 1];
	m_pData[m_nSize] = '\0';
	memcpy(m_pData, rSample.m_pData, m_nSize);
}

MediaSample::~MediaSample(void)
{
	// Free memory
	delete[] m_pData;
	m_pData = NULL;
}

MediaSample* MediaSample::createMediaSample( BYTE* pData, int nSize, double dStartTime, bool bIsSyncPoint )
{
	return new MediaSample(pData, nSize, dStartTime, bIsSyncPoint);
}

MediaSample& MediaSample::operator=( const MediaSample& rSample )
{
	if (this == &rSample )
	{
		return *this;
	}

	m_nSize = rSample.m_nSize;
	m_dStartTime = rSample.m_dStartTime;
	m_bIsSynchronisationPoint = rSample.m_bIsSynchronisationPoint;
	memcpy(m_pData, rSample.m_pData, m_nSize);
	return *this;
}
