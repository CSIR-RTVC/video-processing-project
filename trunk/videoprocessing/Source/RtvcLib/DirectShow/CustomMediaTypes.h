/** @file

MODULE				: CustomMediaTypes

FILE NAME			: CustomMediaTypes.h

DESCRIPTION			: Header file for custom media type GUIDS
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2012, CSIR
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
#pragma once

/// Custom CSIR YUV420 Planer media format
// {C528B79F-F025-4eb7-92B0-46CE8167E9F8}
static const GUID MEDIASUBTYPE_YUV420P = 
{ 0xc528b79f, 0xf025, 0x4eb7, { 0x92, 0xb0, 0x46, 0xce, 0x81, 0x67, 0xe9, 0xf8 } };

//// {14E89EEF-55C0-4342-A652-9E6D68E298E7}
//static const GUID MEDIASUBTYPE_VPP_H264 = 
//{ 0x14e89eef, 0x55c0, 0x4342, { 0xa6, 0x52, 0x9e, 0x6d, 0x68, 0xe2, 0x98, 0xe7 } };

/// Custom RTVC H264 media format
// {BDF25152-046B-4509-8E55-6C73831C8DC4}
static const GUID MEDIASUBTYPE_VPP_H264 = 
{ 0xbdf25152, 0x46b, 0x4509, { 0x8e, 0x55, 0x6c, 0x73, 0x83, 0x1c, 0x8d, 0xc4 } };

// {B940AE21-195E-4CE6-B324-E703AE733AEC}
DEFINE_GUID(MEDIASUBTYPE_OPUS,
  0xb940ae21, 0x195e, 0x4ce6, 0xb3, 0x24, 0xe7, 0x3, 0xae, 0x73, 0x3a, 0xec);

