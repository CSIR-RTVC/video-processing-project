/** @file

MODULE				: SliceHeaderH264

TAG						: SHH264

FILE NAME			: SliceHeaderH264.cpp

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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include "SliceHeaderH264.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
SliceHeaderH264::SliceHeaderH264(void)
{
	_id																	= 0;	///< Slice number in a picture.

	_first_mb_in_slice									= 0;	///< The raster ordered first macroblock number that starts this slice.
	_type																= 0;	///< Defined by the slice type constants.
	_pic_parameter_set_id								= 0;	///< Parameter set to use in this slice.
	_frame_num													= 0;	///< Frame number from the last IDR frame.

	_field_pic_flag											= 0;	///< When present: 1 = slice is a field, 0 (default) = slice is a frame.
	_bottom_field_flag									= 0;	///< When present: 1 = slice is part of bottom field, 0 (default) = top field.
	_idr_pic_id													= 0;	///< For NAL unit = 5, increments for consecutive idr pictures, identifies which pic an I-slice belongs to.
	_pic_order_cnt_lsb									= 0;	///< Modulo pic order count for top field of frame or for a coded field.
	_delta_pic_order_cnt_bottom					= 0;	///< Pic order count difference between bottom and top field of a frame.
	_delta_pic_order_cnt[0]							= 0;	///< Pic order count difference from expected order count of the top/bottom field (default = 0);
	_delta_pic_order_cnt[1]							= 0;
	_redundant_pic_cnt									= 0;	///< 0 = slice belongs to primary coded pic, greater than 0 = redundant coded pic number.

	_direct_spatial_mv_pred_flag				= 0;	///< For B-slices, method used for decoding motion vectors.
	_num_ref_idx_active_override_flag		= 0;	///< Activates/deactivates following members.
	_num_ref_idx_l0_active_minus1				= 0;
	_num_ref_idx_l1_active_minus1				= 0;

	_ref_pic_list_reordering_flag_l0		= 0;	///< Signals the reordering of the pictures in list 0.
	_reordering_of_pic_nums_idc					= 0;
	_abs_diff_pic_num_minus1						= 0;
	_long_term_pic_num									= 0;

	_no_output_of_prior_pics_flag				= 0;
	_long_term_reference_flag						= 0;	///< Mark this IDR picture slice for long term reference.
	_adaptive_ref_pic_marking_mode_flag = 0;	
	
	_cabac_init_idc											= 0;	///< Index for initialisation table for arethmetic entropy coding.

	_qp 																= 0;	///< QP used in this slice.
	_qp_delta														= 0;	///< Change in QP from the previous slice.

	_sp_for_switch_flag									= 0;	///< Specifies decoding process for SP-Slices.
	_qs_delta														= 0;	///< Change in QS from previous slice.

	_disable_deblocking_filter_idc			= 0;	///< Partial disabling of deblocking filter. (0 = default.)
	_alpha_c0_offset_div2								= 0;	///< Offset in alpha and tC0 filter tables.
	_beta_offset_div2										= 0;	///< Offset in beta filter table.

	_group_change_cycle									= 0;	///< Slice group map unit change.
}//end constructor.

SliceHeaderH264::~SliceHeaderH264(void)
{
}//end destructor.

