/** @file

MODULE				: NalHeaderH264

TAG						: NHH264

FILE NAME			: NalHeaderH264.h

DESCRIPTION		: A class to hold H.264 NAL header data for use in the 
								H264v2Codec class.

LICENSE	: GNU Lesser General Public License

Copyright (c) 2008 - 2013, CSIR
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
