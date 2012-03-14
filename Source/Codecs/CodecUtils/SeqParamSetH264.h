/** @file

MODULE				: SeqParamSetH264

TAG						: SPSH264

FILE NAME			: SeqParamSetH264.h

DESCRIPTION		: A class to hold H.264 sequence parameter set data for use in the 
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
#ifndef _SEQPARAMSETH264_H
#define _SEQPARAMSETH264_H

#pragma once

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class SeqParamSetH264
{
public:
	SeqParamSetH264(int seqParamSetId = 0);
	virtual ~SeqParamSetH264(void);

/// Public operators.
public:
  void Copy(SeqParamSetH264* from) { CopyProxy(this, from); }
  int Equals(SeqParamSetH264* to) { return(EqualsProxy(this, to)); }

/// Private operators.
private:
  static void CopyProxy(SeqParamSetH264* me, SeqParamSetH264* from);
  static int EqualsProxy(SeqParamSetH264* me, SeqParamSetH264* to);

/// Public constants.
public:

/// Public members.

	/// Only the baseline profile at level 2 is used in this implemention. Therefore, most 
	/// of these parameters are are constant and not used.
public:
	int _seq_parameter_set_id;										///< The index for this definition of sequence params [0..31].

	int _profile_idc;															///< Baseline profile = 66.
	int _constraint_set0_flag;										///< = 0.
	int _constraint_set1_flag;										///< = 0.
	int _constraint_set2_flag;										///< = 0.
	int _constraint_set3_flag;										///< = 0.
	int _level_idc;																///< = 20 (Level num = 2, calculated as 10 x level num).

	int _chroma_format_idc;												///< Range = [0..3]. When not present = 1 (4:2:0).
	int _residual_colour_transform_flag;					///< = 0.
	int _bit_depth_luma_minus8;										///< Range = [0..4]. When not present = 0 (8 bits/lum component).
	int _bit_depth_chroma_minus8;									///< Range = [0..4]. When not present = 0 (8 bits/chr component).
	int _qpprime_y_zero_transform_bypass_flag;		///< If QP = 0, this flag indicates bypassing the transform process. When not present = 0.

	int _seq_scaling_matrix_present_flag;					///< = 0. No scaling matrices for baseline profile. Indicates existence of _seq_scaling_list_present_flag flags.
	int _seq_scaling_list_present_flag[8];				///< if _seq_scaling_matrix_present_flag, these indicate lists not present/present (0/1).

	int _log2_max_frame_num_minus4;								///< Range = [0..12]. MaxFrameNum = 2^(_log2_max_frame_num_minus4 + 4) for slice _frame_num derivations.

	int _pic_order_cnt_type;											///< Range = [0..2]. Decoding technique selected for mapping coding order count to frame num.
	int _log2_max_pic_order_cnt_lsb_minus4;				///< For B-Slice decoding.  Range = [0..12]. There are no B-Slices in the baseline profile.
	int _delta_pic_order_always_zero_flag;				///< For B-Slice decoding.
	int _offset_for_non_ref_pic;									///< For B-Slice decoding.
	int _offset_for_top_to_bottom_field;					///< For B-Slice decoding.
	int _num_ref_frames_in_pic_order_cnt_cycle;		///< For B-Slice decoding.
	int _offset_for_ref_frame[256];								///< For B-Slice decoding.

	int _num_ref_frames;													///< Max num of short-term and long-term ref frames used for prediction.
	int _gaps_in_frame_num_value_allowed_flag;		///< Indicates allowed values of slice _frame_num.

	int _pic_width_in_mbs_minus1;									///< Width of decoded pic in units of macroblocks. Derive lum and chr pic width from this variable.
	int _pic_height_in_map_units_minus1;					///< Map units are 2 macroblocks for fields and 1 for frames. Derive lum and chr pic height from this variable.

	int _frame_mbs_only_flag;											///< = 1 (baseline profile). fields/frames = 0/1.
	int _mb_adaptive_frame_field_flag;						///< Indicates switching between frame and field macroblocks. When not present = 0.

	int _direct_8x8_inference_flag;								///< Methods for B-Slice motion vector decoding.  There are no B-Slices in the baseline profile.

	int _frame_cropping_flag;											///< Indicates that the cropping params below follow in the stream.
	int _frame_crop_left_offset;									///< In frame coordinate units.
	int _frame_crop_right_offset;									///< In frame coordinate units.
	int _frame_crop_top_offset;										///< In frame coordinate units.
	int _frame_crop_bottom_offset;								///< In frame coordinate units.

	int _vui_parameters_present_flag;							///< vui parameters follow.

};// end class SeqParamSetH264.

#endif	//_SEQPARAMSETH264_H
