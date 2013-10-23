/** @file

MODULE				: BlockH264

TAG						: BH264

FILE NAME			: BlockH264.h

DESCRIPTION		: A class to hold H.264 block data and define all block
								operations. This is a container class on which to "hang"
								the processing functions.

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
	virtual void Quantise(IForwardTransform* pQ, int q);
	virtual void InverseQuantise(IInverseTransform* pQ, int q);
	/** Simple quantisation and inverse quantisation of block coeffs.
	Assume the transform mode is set to QuantOnly and the quant parameter is set
	prior to calling these simplified methods. These are used for cases where the 
	quant parameter is const over multiple calls.
	@param pQ	: A forward transform object to use.
	@return		: None.
	*/
	virtual void Quantise(IForwardTransform* pQ) { pQ->Transform(_pBlk); }
	virtual void InverseQuantise(IInverseTransform* pQ) { pQ->InverseTransform(_pBlk); }

	/** Run-Level encode and decode to and from a stream.
	The set up of the run-level codec must be done outside of
	the scope of this block.
	@param rlc		: The run-level codec to use.
	@param pBsw/r	: Stream reference to use.
	@return				: No. of bits consumed.
	*/
	int RleEncode(IContextAwareRunLevelCodec* rlc, IBitStreamWriter* pBsw) {	int numBits = rlc->Encode((void *)_pBlk, (void *)pBsw); 
																																						_numCoeffs = rlc->GetParameter(rlc->NUM_TOT_COEFF_ID);
																																						return(numBits); } 
	int RleDecode(IContextAwareRunLevelCodec* rlc, IBitStreamReader* pBsr) {  int numBits = rlc->Decode((void *)pBsr, (void *)_pBlk); 
																																						_numCoeffs = rlc->GetParameter(rlc->NUM_TOT_COEFF_ID); 
																																						return(numBits); }
	/** Is the block all zeros.
	Included is the setting of the _coded flag to indicate the
	existence of non-zero values.
	@return : 1 = all zeros, 0 = non-zeros exist.
	*/
	int IsZero(void)
	{
    for(int i = 0; i < _length; i++)
    {
      if(_pBlk[i++] || _pBlk[i++] || _pBlk[i++] || _pBlk[i])
      {
				_coded = 1;
				return(0);	///< Early exit.
      }//end if _pBlk...
    }//end for i...

		_coded = 0;
		return(1);
	}//end IsZero.
	/// Version without setting _coded flag.
	int IsZero2(void)
	{
    for(int i = 0; i < _length; i++)
    {
      if(_pBlk[i++] || _pBlk[i++] || _pBlk[i++] || _pBlk[i])
				return(0);	///< Early exit.
    }//end for i...

		return(1);
	}//end IsZero2.

	/** Copy from another block.
	Match the mem size and copy all members and block
	data.
	@param pBlk	: Block to copy from.
	@return			: 1/0 = success/failure.
	*/
	int CopyBlock(BlockH264* pBlk);

/// Interface implementation.
public:
	/** Get the number of neighbourhood coeffs
	Assume that all the neighbourhood references above and left have
	been pre-defined before calling this method. Find the average if
	both neighbours exist or return the count for either if the other 
	does not exist.
	@param pBlk		: Block to operate on.
	@return				: Number of neighbourhood coeffs.
	*/
	static int GetNumNeighbourCoeffs(BlockH264* pBlk);

	/** Check for content equality with another block.
	Only check the members that relates to the state of the block
	and its contents not its position.
	@param pBlk	: Block to test against.
	@return			: 1/0 = equals/not equals.
	*/
	int Equals(BlockH264* pBlk) { return(EqualsProxy(this, pBlk)); }
	static int EqualsProxy(BlockH264* me, BlockH264* pBlk);

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
	void						SetColour(int colour)							{ _colour = colour; }
	int							GetColour(void)										{ return(_colour); }
	void						SetDcFlag(int dc)									{ _dcFlag = dc; }
	int							IsDc(void)												{ return(_dcFlag); }

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

/// Block constants.
public:
	static const int LUM		= 0;	///< Used for _colour values.
	static const int CB			= 1;
	static const int CR			= 2;

/// Private data block members.
protected:
	/// Code/no code flag. Inverse of the Macroblock level skip flag, but for this block 
	/// only. Is used to help determine the MB type and coded block pattern.
	int							_coded;
	/// The block variables.
	short*					_pBlk;
	int							_width;
	int							_height;
	int							_length;		///< = _width * _height.
	int							_colour;		///< = {LUM, CB, CR}
	int							_dcFlag;		///< Indicates that the block holds DC components only.
	/// Status or temp storage variables.
	int							_numCoeffs;	///< Temp holder for tot no. of coeffs after an RleEncode or RleDecode.
	/// The block overlay for block functions.
	OverlayMem2Dv2* _blk;
  
	/// Mem status.
	int	_ready;

};// end class BlockH264.

#endif	//_BLOCKH264_H
