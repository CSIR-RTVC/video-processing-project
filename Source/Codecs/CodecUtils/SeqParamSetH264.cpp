/** @file

MODULE				: SeqParamSetH264

TAG						: SPSH264

FILE NAME			: SeqParamSetH264.cpp

DESCRIPTION		: A class to hold H.264 sequence parameter set data for use in the 
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

#include "SeqParamSetH264.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
SeqParamSetH264::SeqParamSetH264(int seqParamSetId)
{
	_seq_parameter_set_id										= seqParamSetId;	///< The index for this definition of sequence params [0..31].

	_profile_idc														= 66;							///< Baseline profile = 66.
	_constraint_set0_flag										= 1;							///< = 1.
	_constraint_set1_flag										= 0;							///< = 0.
	_constraint_set2_flag										= 0;							///< = 0.
	_constraint_set3_flag										= 0;							///< = 0.
	_level_idc															= 20;							///< = 20 (Level num = 2, calculated as 10 x level num).

	_chroma_format_idc											= 1;							///< Range = [0..3]. When not present = 1 (4:2:0).
	_residual_colour_transform_flag					= 0;							///< = 0.
	_bit_depth_luma_minus8									= 0;							///< Range = [0..4]. When not present = 0 (8 bits/lum component).
	_bit_depth_chroma_minus8								= 0;							///< Range = [0..4]. When not present = 0 (8 bits/chr component).
	_qpprime_y_zero_transform_bypass_flag		= 0;							///< If QP = 0, this flag indicates bypassing the transform process. When not present = 0.

	_seq_scaling_matrix_present_flag				= 0;							///< = 0. No scaling matrices for baseline profile. Indicates existence of _seq_scaling_list_present_flag flags.
	/// Not initialised _seq_scaling_list_present_flag[8]			///< if _seq_scaling_matrix_present_flag, these indicate lists not present/present (0/1).

	_log2_max_frame_num_minus4							= 12;							///< Range = [0..12]. MaxFrameNum = 2^(_log2_max_frame_num_minus4 + 4) for slice _frame_num derivations.

	_pic_order_cnt_type											= 2;							///< Range = [0..2]. Decoding technique selected for mapping coding order count to frame num. [2 = 1 to 1 mapping]
	_log2_max_pic_order_cnt_lsb_minus4			= 0;							///< For B-Slice decoding.  There are no B-Slices in the baseline profile.
	_delta_pic_order_always_zero_flag				= 0;							///< For B-Slice decoding.
	_offset_for_non_ref_pic									= 0;							///< For B-Slice decoding.
	_offset_for_top_to_bottom_field					= 0;							///< For B-Slice decoding.
	_num_ref_frames_in_pic_order_cnt_cycle	= 0;							///< For B-Slice decoding.
	/// Not initialised _offset_for_ref_frame[256];						///< For B-Slice decoding.

	_num_ref_frames													= 1;							///< Max num of short-term and long-term ref frames.
	_gaps_in_frame_num_value_allowed_flag		= 0;							///< Indicates allowed values of slice _frame_num.

	_pic_width_in_mbs_minus1								= 21;							///< 16*(21+1)=352 Width of decoded pic in units of macroblocks. Derive lum and chr pic width from this variable.
	_pic_height_in_map_units_minus1					= 17;							///< 16*(17+1)=288 Map units are 2 macroblocks for fields and 1 for frames. Derive lum and chr pic height from this variable.

	_frame_mbs_only_flag										= 1;							///< = 1 (baseline profile). fields/frames = 0/1.
	_mb_adaptive_frame_field_flag						= 0;							///< Indicates switching between frame and field macroblocks. When not present = 0.

	_direct_8x8_inference_flag							= 0;							///< Methods for B-Slice motion vector decoding.  There are no B-Slices in the baseline profile.

	_frame_cropping_flag										= 0;							///< Indicates that the cropping params below follow in the stream.
	_frame_crop_left_offset									= 0;							///< In frame coordinate units.
	_frame_crop_right_offset								= 0;							///< In frame coordinate units.
	_frame_crop_top_offset									= 0;							///< In frame coordinate units.
	_frame_crop_bottom_offset								= 0;							///< In frame coordinate units.

	_vui_parameters_present_flag						= 0;							///< vui parameters follow.

}//end constructor.

SeqParamSetH264::~SeqParamSetH264(void)
{
}//end destructor.

void SeqParamSetH264::CopyProxy(SeqParamSetH264* me, SeqParamSetH264* from)
{
	me->_seq_parameter_set_id	                  = from->_seq_parameter_set_id;

	me->_profile_idc														= from->_profile_idc;
	me->_constraint_set0_flag										= from->_constraint_set0_flag;
	me->_constraint_set1_flag										= from->_constraint_set1_flag;
	me->_constraint_set2_flag										= from->_constraint_set2_flag;
	me->_constraint_set3_flag										= from->_constraint_set3_flag;
	me->_level_idc															= from->_level_idc;

	me->_chroma_format_idc											= from->_chroma_format_idc;
	me->_residual_colour_transform_flag					= from->_residual_colour_transform_flag;
	me->_bit_depth_luma_minus8									= from->_bit_depth_luma_minus8;
	me->_bit_depth_chroma_minus8								= from->_bit_depth_chroma_minus8;
	me->_qpprime_y_zero_transform_bypass_flag		= from->_qpprime_y_zero_transform_bypass_flag;

	me->_seq_scaling_matrix_present_flag				= from->_seq_scaling_matrix_present_flag;
	/// Not initialised _seq_scaling_list_present_flag[8]

	me->_log2_max_frame_num_minus4							= from->_log2_max_frame_num_minus4;

	me->_pic_order_cnt_type											= from->_pic_order_cnt_type;
	me->_log2_max_pic_order_cnt_lsb_minus4			= from->_log2_max_pic_order_cnt_lsb_minus4;
	me->_delta_pic_order_always_zero_flag				= from->_delta_pic_order_always_zero_flag;
	me->_offset_for_non_ref_pic									= from->_offset_for_non_ref_pic;
	me->_offset_for_top_to_bottom_field					= from->_offset_for_top_to_bottom_field;
	me->_num_ref_frames_in_pic_order_cnt_cycle	= from->_num_ref_frames_in_pic_order_cnt_cycle;
	/// Not initialised _offset_for_ref_frame[256];

	me->_num_ref_frames													= from->_num_ref_frames;
	me->_gaps_in_frame_num_value_allowed_flag		= from->_gaps_in_frame_num_value_allowed_flag;

	me->_pic_width_in_mbs_minus1								= from->_pic_width_in_mbs_minus1;
	me->_pic_height_in_map_units_minus1					= from->_pic_height_in_map_units_minus1;

	me->_frame_mbs_only_flag										= from->_frame_mbs_only_flag;
	me->_mb_adaptive_frame_field_flag						= from->_mb_adaptive_frame_field_flag;

	me->_direct_8x8_inference_flag							= from->_direct_8x8_inference_flag;

	me->_frame_cropping_flag										= from->_frame_cropping_flag;
	me->_frame_crop_left_offset									= from->_frame_crop_left_offset;
	me->_frame_crop_right_offset								= from->_frame_crop_right_offset;
	me->_frame_crop_top_offset									= from->_frame_crop_top_offset;
	me->_frame_crop_bottom_offset								= from->_frame_crop_bottom_offset;

	me->_vui_parameters_present_flag						= from->_vui_parameters_present_flag;

}//end CopyProxy.

int SeqParamSetH264::EqualsProxy(SeqParamSetH264* me, SeqParamSetH264* to)
{
	if( (me->_seq_parameter_set_id	              != to->_seq_parameter_set_id)||

	    (me->_profile_idc													!= to->_profile_idc)||
	    (me->_constraint_set0_flag								!= to->_constraint_set0_flag)||
	    (me->_constraint_set1_flag								!= to->_constraint_set1_flag)||
	    (me->_constraint_set2_flag								!= to->_constraint_set2_flag)||
	    (me->_constraint_set3_flag								!= to->_constraint_set3_flag)||
	    (me->_level_idc														!= to->_level_idc) )
        return(0);

	if( (me->_chroma_format_idc										!= to->_chroma_format_idc)||
	    (me->_residual_colour_transform_flag			!= to->_residual_colour_transform_flag)||
	    (me->_bit_depth_luma_minus8								!= to->_bit_depth_luma_minus8)||
	    (me->_bit_depth_chroma_minus8							!= to->_bit_depth_chroma_minus8)||
	    (me->_qpprime_y_zero_transform_bypass_flag != to->_qpprime_y_zero_transform_bypass_flag) )
        return(0);

	if(me->_seq_scaling_matrix_present_flag				!= to->_seq_scaling_matrix_present_flag)
    return(0);
	/// Not initialised _seq_scaling_list_present_flag[8]

	if(me->_log2_max_frame_num_minus4							!= to->_log2_max_frame_num_minus4)
    return(0);

	if( (me->_pic_order_cnt_type									!= to->_pic_order_cnt_type)||
	    (me->_log2_max_pic_order_cnt_lsb_minus4		!= to->_log2_max_pic_order_cnt_lsb_minus4)||
	    (me->_delta_pic_order_always_zero_flag		!= to->_delta_pic_order_always_zero_flag)||
	    (me->_offset_for_non_ref_pic							!= to->_offset_for_non_ref_pic)||
	    (me->_offset_for_top_to_bottom_field			!= to->_offset_for_top_to_bottom_field)||
	    (me->_num_ref_frames_in_pic_order_cnt_cycle	!= to->_num_ref_frames_in_pic_order_cnt_cycle) )
        return(0);
	/// Not initialised _offset_for_ref_frame[256];

	if( (me->_num_ref_frames												!= to->_num_ref_frames)||
	    (me->_gaps_in_frame_num_value_allowed_flag	!= to->_gaps_in_frame_num_value_allowed_flag)||

	    (me->_pic_width_in_mbs_minus1								!= to->_pic_width_in_mbs_minus1)||
	    (me->_pic_height_in_map_units_minus1				!= to->_pic_height_in_map_units_minus1)||

	    (me->_frame_mbs_only_flag										!= to->_frame_mbs_only_flag)||
	    (me->_mb_adaptive_frame_field_flag					!= to->_mb_adaptive_frame_field_flag)||

	    (me->_direct_8x8_inference_flag							!= to->_direct_8x8_inference_flag) )
        return(0);

	if( (me->_frame_cropping_flag										!= to->_frame_cropping_flag)||
	    (me->_frame_crop_left_offset								!= to->_frame_crop_left_offset)||
	    (me->_frame_crop_right_offset								!= to->_frame_crop_right_offset)||
	    (me->_frame_crop_top_offset									!= to->_frame_crop_top_offset)||
	    (me->_frame_crop_bottom_offset							!= to->_frame_crop_bottom_offset) )
        return(0);

	if(me->_vui_parameters_present_flag						  != to->_vui_parameters_present_flag)
    return(0);

  return(1);
}//end EqualsProxy.


