/** @file

MODULE				: BlockH264

TAG						: BH264

FILE NAME			: BlockH264.h

DESCRIPTION		: A class to hold H.264 block data and define all block
								operations. This is a container class on which to "hang"
								the processing functions.

COPYRIGHT			:	(c)CSIR 2007-2009 all rights resevered

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
#ifndef _BLOCKH264_H
#define _BLOCKH264_H

#pragma once

#include "OverlayMem2Dv2.h"
#include "IForwardTransform.h"
#include "IInverseTransform.h"
#include "IContextAwareRunLevelCodec.h"
#include "IBitStreamReader.h"
#include "IBitStreamWriter.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class BlockH264
{
public:
	BlockH264(void);
	BlockH264(int width, int height);
	virtual ~BlockH264(void);
	int			Ready(void) { return(_ready); }

/// Block operations.
public:
	/** Forward and inverse integer transform.
	The calling function is responsible to set the modes and parameters of the
	transform that might include the quantisation or intra/inter switch. The 
	context of the call is outside the scope of this class.
	@param	pIT	: The transform to operate on this block.
	@return			: None.
	*/
	void ForwardTransform(IForwardTransform* pIT) { pIT->Transform(_pBlk); }
	void InverseTransform(IInverseTransform* pIIT) { pIIT->InverseTransform(_pBlk); }

	/** Quantisation and inverse quantisation of block coeffs.
	The transform classes are assumed to implement scaling and quantisation on
	the block as a mode setting. Therefore the current mode is stored and replaced
	after the quantisation only process.
	@param pQ	: A forward transform object to use.
	@param q	: Quantisation parameter.
	@return		: None.
	*/
	void Quantise(IForwardTransform* pQ, int q);
	void InverseQuantise(IInverseTransform* pQ, int q);

	/** Run-Level encode and decode to and from a stream.
	The set up of the run-level codec must be done outside of
	the scope of this block.
	@param rlc		: The run-level codec to use.
	@param pBsw/r	: Stream reference to use.
	@return				: No. of bits consumed.
	*/
	int RleEncode(IContextAwareRunLevelCodec* rlc, IBitStreamWriter* pBsw) {	rlc->Encode((void *)_pBlk, (void *)pBsw); 
																																						_numCoeffs = rlc->GetParameter(rlc->NUM_TOT_COEFF_ID); } 
	int RleDecode(IContextAwareRunLevelCodec* rlc, IBitStreamReader* pBsr) {  rlc->Decode((void *)pBsr, (void *)_pBlk); 
																																						_numCoeffs = rlc->GetParameter(rlc->NUM_TOT_COEFF_ID); } 

/// Member access.
public:
	OverlayMem2Dv2*	GetBlkOverlay(void)								{ return(_blk); }
	short*					GetBlk(void)											{ return(_pBlk); }
	int							IsCoded(void)											{ return(_coded); }
	void						SetCoded(int coded)								{ _coded = coded; }
	int							GetDC(void)												{ return(_pBlk[0]); }
	void						SetDC(int dc)											{ _pBlk[0] = (short)dc; }
	void						SetDim(int width, int height);
	int							GetWidth(void)										{ return(_width); }
	int							GetHeight(void)										{ return(_height); }
	void						SetNumCoeffs(int numCoeffs)				{ _numCoeffs = numCoeffs; }
	int							GetNumCoeffs(void)								{ return(_numCoeffs); }

	void						Copy(void* buff);
	void						CopyRow(int row, void* buff);
	void						CopyCol(int col, void* buff);
	void						Zero(void);

/// Private methods.
protected:
	void DeleteMem(void);

/// Public data block members.
public:
	/// Location aware members.
	int _offX;				///< Block coord offset from top left of macroblock.
	int _offY;
	/// Neighborhood access for determining context-aware variables.
	BlockH264* _blkAbove;
	BlockH264* _blkLeft;

/// Private data block members.
protected:
	/// Code/no code flag. Inverse of the Macroblock level skip flag, but for this block 
	/// only. Is used to help determine the MB type.
	int							_coded;
	/// The block variables.
	short*					_pBlk;
	int							_width;
	int							_height;
	int							_length;		///< = _width * _height.
	/// Status or temp storage variables.
	int							_numCoeffs;	///< Temp holder for tot no. of coeffs after an encode or decode.
	/// The block overlay for block functions.
	OverlayMem2Dv2* _blk;
  
	/// Mem status.
	int	_ready;

};// end class BlockH264.

#endif	//_BLOCKH264_H
