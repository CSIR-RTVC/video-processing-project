/** @file

MODULE				: NalHeaderH264

TAG						: NHH264

FILE NAME			: NalHeaderH264.h

DESCRIPTION		: A class to hold H.264 NAL header data for use in the 
								H264v2Codec class.

COPYRIGHT			:	(c)CSIR 2007-2010 all rights resevered

LICENSE				: Software License Agreement (BSD License)

RESTRICTIONS	: Redistribution and use in source and binary forms, with or without 
								modification, are permitted provided that the following conditions 
								are met:

								* Redistributions of source code must retain the above copyright notice, 
								this list of conditions and the following disclaimer.
								* Redistributions in binary form must reproduce the above copyright notice, 
								this list of conditions and the following disclaimer in the documentation 
								and/or other materials provided with the distribution.
								* Neither the name of the CSIR nor the names of its contributors may be used 
								to endorse or promote products derived from this software without specific 
								prior written permission.

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

=========================================================================================
*/
#ifndef _NALHEADERH264_H
#define _NALHEADERH264_H

#pragma once

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class NalHeaderH264
{
public:
	NalHeaderH264(void);
	virtual ~NalHeaderH264(void);

/// Public constants.
public:
	/// Payload types for _unit_type member.
	static const int Unspecified							= 0;
	static const int NonIDR_NoPartition_Slice	= 1;
	static const int NonIDR_PartitionA_Slice	= 2;
	static const int NonIDR_PartitionB_Slice	= 3;
	static const int NonIDR_PartitionC_Slice	= 4;
	static const int IDR_Slice								= 5;
	static const int SEI											= 6;
	static const int SeqParamSet							= 7;
	static const int PicParamSet							= 8;
	static const int AccessUnitDelimiter			= 9;
	static const int EndOfSeq									= 10;
	static const int EndOfStream							= 11;
	static const int FillerData								= 12;
	static const int SeqParamSetExt						= 13;
	/// 14..18 Reserved.
	static const int Aux_NoPartition_Slice		= 19;
	/// 20..23 Reserved.
	/// 24..31 Unspecified.

/// Public members.
public:
	int _ref_idc;				///< Indicates reference parameter sets or IDR coded slices in the NAL payload. 
	int _unit_type;			///< NAL contents type.

};// end class NalHeaderH264.

#endif	//_NALHEADERH264_H
