/** @file

MODULE				: MediaSample

FILE NAME			: MediaSample.h

DESCRIPTION			: 
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008, Meraka Institute
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the Meraka Institute nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

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
#pragma once

// STL
#include <string>

#ifndef BYTE
	typedef unsigned char BYTE;
#endif

/**
* Encapsulates raw media data, size and start time of the sample
* Extended Media Sample to have a sync point: this can be used as a flag to say this is the first i-frame
* Or that this is the first sample that has been RTCP synchronized
*/
class MediaSample
{
public:
	MediaSample(const MediaSample& rSample);
	MediaSample& operator=(const MediaSample& rSample);
	virtual ~MediaSample(void);
	static MediaSample* createMediaSample(BYTE* pData, int nSize, double dStartTime = 0.0, bool bIsSyncPoint = false);

	const BYTE* getData() const { return m_pData; }
	int getSize() const {return m_nSize;}
	double StartTime() const { return m_dStartTime; }

  bool isMarkerSet() const { return m_bMarker; }
  void setMarker(bool val) { m_bMarker = val; }
private:
	MediaSample(BYTE* pData, int nSize, double dStartTime, bool bIsSyncPoint);
	
	BYTE* m_pData;
	int m_nSize;
	double m_dStartTime;
	bool m_bMarker;
};
