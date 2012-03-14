/** @file

MODULE				: PicParamSetH264

TAG						: PPSH264

FILE NAME			: PicParamSetH264.h

DESCRIPTION		: A class to hold H.264 picture parameter set data for use in the 
								H264v2Codec class.

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
