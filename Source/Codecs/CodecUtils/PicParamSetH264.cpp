/** @file

MODULE				: PicParamSetH264

TAG						: PPSH264

FILE NAME			: PicParamSetH264.cpp

DESCRIPTION		: A class to hold H.264 picture parameter set data for use in the 
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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include "PicParamSetH264.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
PicParamSetH264::PicParamSetH264(int pic_param_set_id)
{
	_pic_parameter_set_id										= pic_param_set_id;	///< The index for this definition of picture params [0..255].
	_seq_parameter_set_id										= -1;								///< This pic param set refers to this seq param set. Range = [0..31].

	_entropy_coding_mode_flag								= 0;								///< Baseline profile = 0. CAVLC/CABAC = 0/1.
	_pic_order_present_flag									= 0;								///< Pic order count syntax elements are present in the slice header.

	_num_slice_groups_minus1								= 0;								///< = 0 (baseline profile). Num of slice groups for a pic.
	_slice_group_map_type										= 0;								///< Defines map units to slice groups. Range = [0..6].
	/// Not initialised: _run_length_minus1[7];									///< Slice group mapping.
	/// Not initialised: _top_left[7];													///< Slice group mapping.
	/// Not initialised: _bottom_right[7];											///< Slice group mapping.
	_slice_group_change_direction_flag			= 0;								///< Slice group mapping.
	_slice_group_change_rate_minus1					= 0;								///< Slice group mapping.
	_pic_size_in_map_units_minus1						= 395;							///< For 352x288 frame. Num of map units (macroblocks for frames) in the pic. Macroblock width * height.
	/// Not initialised: _slice_group_id[7];										///< Slice group mapping.

	_num_ref_idx_l0_active_minus1						= 0;								///< Max ref index for ref pic list0.
	_num_ref_idx_l1_active_minus1						= 0;								///< Max ref index for ref pic list1. (For B-Slices)

	_weighted_pred_flag											= 0;								///< Indicates weighted prediction applied to P-Slices.
	_weighted_bipred_idc										= 0;								///< Weighted prediction type applied to B-Slices.

	_pic_init_qp_minus26										= 0;								///< Initial value of lum SliceQP - 26 for each slice. The slice_qp_delata modifies this value. Rng = [-26..25].
	_pic_init_qs_minus26										= 0;								///< For SP and SI slices.
	_chroma_qp_index_offset									= 0;								///< Offset added to lum QP (and QS) for Cb chr QP values. Rng = [-12..12].
	_second_chroma_qp_index_offset					= 0;								///< For Cr chr QP. When not present = _chroma_qp_index_offset above.
	_deblocking_filter_control_present_flag = 0;								///< Indicates presence of elements in slice header to change the characteristics of the deblocking filter.
	_constrained_intra_pred_flag						= 1;								///< = 1. Indicates that intra macroblock prediction can only be done from other intra macroblocks. 
	_redundant_pic_cnt_present_flag					= 0;								///< Indicates that redundant pic count elements are in the slice header.
	_transform_8x8_mode_flag								= 0;								///< = 0. Indicates 8x8 transform is used. When not present = 0.
	_pic_scaling_matrix_present_flag				= 0;								///< = 0. Indicates params are present to modify the scaling lists specified in the seq param sets.
	/// Not initialised:_pic_scaling_list_present_flag[8];			///< Scaling list syntax structure present for scaling list i = 0..7.

}//end constructor.

PicParamSetH264::~PicParamSetH264(void)
{
}//end destructor.

void PicParamSetH264::CopyProxy(PicParamSetH264* me, PicParamSetH264* from)
{
	me->_pic_parameter_set_id										= from->_pic_parameter_set_id;
	me->_seq_parameter_set_id										= from->_seq_parameter_set_id;

	me->_entropy_coding_mode_flag								= from->_entropy_coding_mode_flag;
	me->_pic_order_present_flag									= from->_pic_order_present_flag;

	me->_num_slice_groups_minus1								= from->_num_slice_groups_minus1;
	me->_slice_group_map_type										= from->_slice_group_map_type;
	/// Not initialised: _run_length_minus1[7];
	/// Not initialised: _top_left[7];
	/// Not initialised: _bottom_right[7];
	me->_slice_group_change_direction_flag			= from->_slice_group_change_direction_flag;
	me->_slice_group_change_rate_minus1					= from->_slice_group_change_rate_minus1;
	me->_pic_size_in_map_units_minus1						= from->_pic_size_in_map_units_minus1;
	/// Not initialised: _slice_group_id[7];

	me->_num_ref_idx_l0_active_minus1						= from->_num_ref_idx_l0_active_minus1;
	me->_num_ref_idx_l1_active_minus1						= from->_num_ref_idx_l1_active_minus1;

	me->_weighted_pred_flag											= from->_weighted_pred_flag;
	me->_weighted_bipred_idc										= from->_weighted_bipred_idc;

	me->_pic_init_qp_minus26										= from->_pic_init_qp_minus26;
	me->_pic_init_qs_minus26										= from->_pic_init_qs_minus26;
	me->_chroma_qp_index_offset									= from->_chroma_qp_index_offset;
	me->_second_chroma_qp_index_offset					= from->_second_chroma_qp_index_offset;
	me->_deblocking_filter_control_present_flag = from->_deblocking_filter_control_present_flag;
	me->_constrained_intra_pred_flag						= from->_constrained_intra_pred_flag; 
	me->_redundant_pic_cnt_present_flag					= from->_redundant_pic_cnt_present_flag;
	me->_transform_8x8_mode_flag								= from->_transform_8x8_mode_flag;
	me->_pic_scaling_matrix_present_flag				= from->_pic_scaling_matrix_present_flag;
	/// Not initialised:_pic_scaling_list_present_flag[8];
}//end CopyProxy.

int PicParamSetH264::EqualsProxy(PicParamSetH264* me, PicParamSetH264* to)
{
	if( (me->_pic_parameter_set_id							!= to->_pic_parameter_set_id)||
	    (me->_seq_parameter_set_id							!= to->_seq_parameter_set_id) )
        return(0);

	if( (me->_entropy_coding_mode_flag					!= to->_entropy_coding_mode_flag)||
	    (me->_pic_order_present_flag						!= to->_pic_order_present_flag) )
        return(0);

	if( (me->_num_slice_groups_minus1						!= to->_num_slice_groups_minus1)||
	    (me->_slice_group_map_type							!= to->_slice_group_map_type)||
	/// Not initialised: _run_length_minus1[7];
	/// Not initialised: _top_left[7];
	/// Not initialised: _bottom_right[7];
	    (me->_slice_group_change_direction_flag	!= to->_slice_group_change_direction_flag)||
	    (me->_slice_group_change_rate_minus1		!= to->_slice_group_change_rate_minus1)||
	    (me->_pic_size_in_map_units_minus1			!= to->_pic_size_in_map_units_minus1) )
	/// Not initialised: _slice_group_id[7];
        return(0);

	if( (me->_num_ref_idx_l0_active_minus1			!= to->_num_ref_idx_l0_active_minus1)||
	    (me->_num_ref_idx_l1_active_minus1			!= to->_num_ref_idx_l1_active_minus1) )
        return(0);

	if( (me->_weighted_pred_flag								!= to->_weighted_pred_flag)||
	    (me->_weighted_bipred_idc								!= to->_weighted_bipred_idc) )
        return(0);

	if( (me->_pic_init_qp_minus26								!= to->_pic_init_qp_minus26)||
	    (me->_pic_init_qs_minus26								!= to->_pic_init_qs_minus26)||
	    (me->_chroma_qp_index_offset						!= to->_chroma_qp_index_offset)||
	    (me->_second_chroma_qp_index_offset			!= to->_second_chroma_qp_index_offset)||
	    (me->_deblocking_filter_control_present_flag != to->_deblocking_filter_control_present_flag)||
	    (me->_constrained_intra_pred_flag				!= to->_constrained_intra_pred_flag)|| 
	    (me->_redundant_pic_cnt_present_flag		!= to->_redundant_pic_cnt_present_flag)||
	    (me->_transform_8x8_mode_flag						!= to->_transform_8x8_mode_flag)||
	    (me->_pic_scaling_matrix_present_flag		!= to->_pic_scaling_matrix_present_flag) )
	/// Not initialised:_pic_scaling_list_present_flag[8];
        return(0);

  return(1);
}//end EqualsProxy.
