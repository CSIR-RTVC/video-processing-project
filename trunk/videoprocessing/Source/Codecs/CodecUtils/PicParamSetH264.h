/** @file

MODULE				: PicParamSetH264

TAG						: PPSH264

FILE NAME			: PicParamSetH264.h

DESCRIPTION		: A class to hold H.264 picture parameter set data for use in the 
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
#ifndef _PICPARAMSETH264_H
#define _PICPARAMSETH264_H

#pragma once

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class PicParamSetH264
{
public:
	PicParamSetH264(int pic_param_set_id = 0);
	virtual ~PicParamSetH264(void);

/// Public operators.
public:
  void Copy(PicParamSetH264* from) { CopyProxy(this, from); }
  int Equals(PicParamSetH264* to) { return(EqualsProxy(this, to)); }

/// Private operators.
private:
  static void CopyProxy(PicParamSetH264* me, PicParamSetH264* from);
  static int EqualsProxy(PicParamSetH264* me, PicParamSetH264* to);

/// Public constants.
public:

/// Public members.
public:
	int _pic_parameter_set_id;										///< The index for this definition of picture params [0..255].
	int _seq_parameter_set_id;										///< This pic param set refers to this seq param set. Range = [0..31].

	int _entropy_coding_mode_flag;								///< CAVLC/CABAC = 0/1.
	int _pic_order_present_flag;									///< Pic order count syntax elements are present in the slice header.

	int _num_slice_groups_minus1;									///< = 0 (baseline profile). Num of slice groups for a pic.
	int _slice_group_map_type;										///< Defines map units to slice groups. Range = [0..6].
	int _run_length_minus1[7];										///< Slice group mapping.
	int _top_left[7];															///< Slice group mapping.
	int _bottom_right[7];													///< Slice group mapping.
	int _slice_group_change_direction_flag;				///< Slice group mapping.
	int _slice_group_change_rate_minus1;					///< Slice group mapping.
	int _pic_size_in_map_units_minus1;						///< Num of map units (macroblocks for frames) in the pic. Macroblock width * height.
	int _slice_group_id[7];												///< Slice group mapping.

	int _num_ref_idx_l0_active_minus1;						///< Max ref index for ref pic list0.
	int _num_ref_idx_l1_active_minus1;						///< Max ref index for ref pic list1. (For B-Slices)

	int _weighted_pred_flag;											///< Indicates weighted prediction applied to P-Slices.
	int _weighted_bipred_idc;											///< Weighted prediction type applied to B-Slices.

	int _pic_init_qp_minus26;											///< Initial value of lum SliceQP - 26 for each slice. The slice_qp_delata modifies this value. Rng = [-26..25].
	int _pic_init_qs_minus26;											///< For SP and SI slices.
	int _chroma_qp_index_offset;									///< Offset added to lum QP (and QS) for Cb chr QP values. Rng = [-12..12].
	int _second_chroma_qp_index_offset;						///< For Cr chr QP. When not present = _chroma_qp_index_offset above.
	int _deblocking_filter_control_present_flag;	///< Indicates presence of elements in slice header to change the characteristics of the deblocking filter.
	int _constrained_intra_pred_flag;							///< = 1. Indicates that intra macroblock prediction can only be done from other intra macroblocks. 
	int _redundant_pic_cnt_present_flag;					///< Indicates that redundant pic count elements are in the slice header.
	int _transform_8x8_mode_flag;									///< = 0. Indicates 8x8 transform is used. When not present = 0.
	int _pic_scaling_matrix_present_flag;					///< = 0. Indicates params are present to modify the scaling lists specified in the seq param sets.
	int _pic_scaling_list_present_flag[8];				///< Scaling list syntax structure present for scaling list i = 1..8.

};// end class PicParamSetH264.

#endif	//_PICPARAMSETH264_H
