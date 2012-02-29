/** @file

MODULE				: SliceHeaderH264

TAG						: SHH264

FILE NAME			: SliceHeaderH264.h

DESCRIPTION		: A class to hold H.264 slice header data for use in the 
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
#ifndef _SLICEHEADERH264_H
#define _SLICEHEADERH264_H

#pragma once

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class SliceHeaderH264
{
public:
	SliceHeaderH264(void);
	virtual ~SliceHeaderH264(void);

/// Public constants.
public:
	/// Slice types for _slice_type member.
	static const int P_Slice	    = 0;
	static const int B_Slice	    = 1;
	static const int I_Slice	    = 2;
	static const int SP_Slice     = 3;
	static const int SI_Slice     = 4;
	static const int P_Slice_All	= 5;
	static const int B_Slice_All	= 6;
	static const int I_Slice_All	= 7;
	static const int SP_Slice_All = 8;
	static const int SI_Slice_All = 9;

/// Public members.
public:
	int _id;																///< Slice number in a picture. Macroblocks are asigned to this id. (Not in stream.)

	int _first_mb_in_slice;									///< The raster ordered first macroblock number that starts this slice.
	int _type;															///< Defined by the slice type constants.
	int _pic_parameter_set_id;							///< Parameter set to use in this slice.
	int _frame_num;													///< Frame number from the last IDR frame.

	/// Dependent members that are included based on NAL type, sequence param set and pic param set.
	int _field_pic_flag;										///< When present: 1 = slice is a field, 0 (default) = slice is a frame.
	int _bottom_field_flag;									///< When present: 1 = slice is part of bottom field, 0 (default) = top field.
	int _idr_pic_id;												///< For NAL unit = 5, increments for consecutive idr pictures, identifies which pic an I-slice belongs to.
	int _pic_order_cnt_lsb;									///< Modulo pic order count for top field of frame or for a coded field.
	int _delta_pic_order_cnt_bottom;				///< Pic order count difference between bottom and top field of a frame.
	int _delta_pic_order_cnt[2];						///< Pic order count difference from expected order count of the top/bottom field (default = 0);
	int _redundant_pic_cnt;									///< 0 = slice belongs to primary coded pic, greater than 0 = redundant coded pic number.

	int _direct_spatial_mv_pred_flag;				///< For B-slices, method used for decoding motion vectors.
	int _num_ref_idx_active_override_flag;	///< Activates/deactivates following members.
	int _num_ref_idx_l0_active_minus1;
	int _num_ref_idx_l1_active_minus1;

	/// Ref picture list re-ordering members.
	int _ref_pic_list_reordering_flag_l0;		///< Signals the reordering of the pictures in list 0.
	int _ref_pic_list_reordering_flag_l1;		///< Signals the reordering of the pictures in list 1 for B-frames.
	int _reordering_of_pic_nums_idc;
	int _abs_diff_pic_num_minus1;
	int _long_term_pic_num;

	/// Weighted prediction table members.
	// TODO: Add wieght pred table members.

	/// Decoded ref picture marking members.
	int _no_output_of_prior_pics_flag;
	int _long_term_reference_flag;					///< Mark this IDR picture slice for long term reference.
	int _adaptive_ref_pic_marking_mode_flag;	
	// TODO: Add dec ref pic marking members for memory management control.

	int _cabac_init_idc;									///< Index for initialisation table for arethmetic entropy coding.

	int _qp;															///< QP used in this slice. (Not in stream.)
	int _qp_delta;												///< Change in QP from the previous slice.

	int _sp_for_switch_flag;							///< Specifies decoding process for SP-Slices.
	int _qs_delta;												///< Change in QS from previous slice.

	/// 0 = All edges are filtered, 1 = Filtering is disabled, 2 = Filter enabled but not on slice boundaries.
	int _disable_deblocking_filter_idc;		///< Control of the deblocking process is defined by this flag. (0 = default.)
	int _alpha_c0_offset_div2;						///< Offset in alpha and tC0 filter tables.
	int _beta_offset_div2;								///< Offset in beta filter table.

	int _group_change_cycle;							///< Slice group map unit change.

};// end class SliceHeaderH264.

#endif	//_SLICEHEADERH264_H
