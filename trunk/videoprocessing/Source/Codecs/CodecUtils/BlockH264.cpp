/** @file

MODULE				: BlockH264

TAG						: BH264

FILE NAME			: BlockH264.cpp

DESCRIPTION		: A class to hold H.264 block data and define all block
								operations. This is a container class on which to "hang"
								the processing functions.

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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#include <string.h>
#endif

#include <memory.h>
#include "BlockH264.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
BlockH264::BlockH264(void)
{
	/// Dangerous to alloc mem in the constructor. Should be 2 phase 
	/// construction. Use ready flag to check against.
	_ready			= 0;
	_coded			= 0;
	_numCoeffs	= 0;
	_width			= 4;	///< Default to a 4x4 block.
	_height			= 4;
	_length			= 16;
	_colour			= LUM;
	_dcFlag			= 1;

	/// Location aware members.
	_offX				= 0;		///< Block coord offset from top left of macroblock.
	_offY				= 0;
	/// Neighborhood access for determining context-aware variables.
	_blkAbove		= NULL;
	_blkLeft		= NULL;

	/// The 4x4 block mem.
	_pBlk = NULL;
	_pBlk = new short[_length];

	/// The block overlay.
	_blk = NULL;
	_blk = new OverlayMem2Dv2(_pBlk, _width, _height, _width, _height);

	if( (_pBlk != NULL)&&(_blk != NULL) )
		_ready = 1;

}//end constructor.

BlockH264::BlockH264(int width, int height)
{
	/// Dangerous to alloc mem in the constructor. Should be 2 phase 
	/// construction. Use ready flag to check against.
	_ready			= 0;
	_coded			= 0;
	_numCoeffs	= 0;
	_width			= width;
	_height			= height;
	_length			= width * height;

	/// Location aware members.
	_offX				= 0;
	_offY				= 0;
	_blkAbove		= NULL;
	_blkLeft		= NULL;

	/// The width x height block mem.
	_pBlk = NULL;
	_pBlk = new short[_length];

	/// The block overlay.
	_blk = NULL;
	_blk = new OverlayMem2Dv2(_pBlk, _width, _height, _width, _height);

	if( (_pBlk != NULL)&&(_blk != NULL) )
		_ready = 1;

}//end constructor.

BlockH264::~BlockH264(void)
{
	DeleteMem();
}//end destructor.

void BlockH264::DeleteMem(void)
{
	if(_blk != NULL)
		delete _blk;
	_blk = NULL;

	if(_pBlk != NULL)
		delete[] _pBlk;
	_pBlk = NULL;

	_ready = 0;
}//end DeleteMem.

/*
---------------------------------------------------------------------------
	Interface Methods.
---------------------------------------------------------------------------
*/
/** Quantisation and inverse quantisation of block coeffs.
The transform classes are assumed to implement scaling and quantisation on
the block as a mode setting. Therefore the current mode is stored and replaced
after the quantisation only process.
@param pQ	: A forward transform object to use.
@param q	: Quantisation parameter.
@return		: None.
*/
void BlockH264::Quantise(IForwardTransform* pQ, int q)
{ 
	int mode = pQ->GetMode();					///< Store current mode. 
	pQ->SetParameter(pQ->QUANT_ID,q);	///< Set quant param.
	pQ->SetMode(pQ->QuantOnly);				///< Exclude the integer transform.
	pQ->Transform(_pBlk);							///< Do the quant. 
	pQ->SetMode(mode);								///< Restore previous mode.
}//end Quantise.

void BlockH264::InverseQuantise(IInverseTransform* pQ, int q)
{ 
	int mode = pQ->GetMode();					///< Store current mode. 
	pQ->SetParameter(pQ->QUANT_ID,q);	///< Set quant param.
	pQ->SetMode(pQ->QuantOnly);				///< Exclude the integer transform.
	pQ->InverseTransform(_pBlk);			///< Do the inverse quant. 
	pQ->SetMode(mode);								///< Restore previous mode.
}//end InverseQuantise.

/** Copy from another block.
Match the mem size and copy all members and block
data.
@param pBlk	: Block to copy from.
@return			: 1/0 = success/failure.
*/
int BlockH264::CopyBlock(BlockH264* pBlk)
{
	/// Set memory members.
	SetDim(pBlk->GetWidth(), pBlk->GetHeight());
	if(!_ready)
		return(0);
	pBlk->Copy( (void *)_pBlk );

	/// Private members.
	SetCoded(pBlk->IsCoded());
	SetNumCoeffs(pBlk->GetNumCoeffs());
	SetColour(pBlk->GetColour());
	SetDcFlag(pBlk->IsDc());
	/// Public members.
	_offX			= pBlk->_offX;
	_offY			= pBlk->_offY;
	_blkAbove	= pBlk->_blkAbove;
	_blkLeft	= pBlk->_blkLeft;

	return(1);
}//end CopyBlock.

/** Get the number of neighbourhood coeffs
Assume that all the neighbourhood references above and left have
been pre-defined before calling this method. Find the average if
both neighbours exist or return the count for either if the other 
does not exist.
@param pBlk		: Block to operate on.
@return				: Number of neighbourhood coeffs.
*/
int BlockH264::GetNumNeighbourCoeffs(BlockH264* pBlk)
{
	int neighCoeffs = 0;

	if(pBlk->_blkAbove != NULL)
		neighCoeffs += (pBlk->_blkAbove)->GetNumCoeffs();
	if(pBlk->_blkLeft != NULL)
		neighCoeffs += (pBlk->_blkLeft)->GetNumCoeffs();
	if((pBlk->_blkAbove != NULL)&&(pBlk->_blkLeft != NULL))
		neighCoeffs = (neighCoeffs + 1)/2;

	return(neighCoeffs);
}//end GetNumNeighbourCoeffs.

/** Check for content equality with another block.
Only check the members that relates to the state of the block
and its contents not its position.
@param me		: This block.
@param pBlk	: Block to test against.
@return			: 1/0 = equals/not equals.
*/
int BlockH264::EqualsProxy(BlockH264* me, BlockH264* pBlk)
{
	for(int i = 0; i < me->_length; i++)
	{
		if(me->_pBlk[i] != pBlk->_pBlk[i])
			return(0);
	}//end for i...

	if( (me->_coded != pBlk->_coded) ||
			(me->_width != pBlk->_width) ||
			(me->_height != pBlk->_height) ||
			(me->_length != pBlk->_length) ||
			(me->_colour != pBlk->_colour) ||
			(me->_dcFlag != pBlk->_dcFlag) ||
			(me->_ready != pBlk->_ready) ||
			(me->_numCoeffs != pBlk->_numCoeffs) )
		return(0);

	return(1);
}//end Equals.

/*
---------------------------------------------------------------------------
	Access Methods.
---------------------------------------------------------------------------
*/

/** Set the block dimensions.
This should not be called often and typically after instantiation. The width
and height of the block are set and the memory is adjusted for these new
sizes. A call to Ready() will check the successful mem alloc.
@param	width		: Block width.
@param	height	: Block height.
@return					: None.
*/
void BlockH264::SetDim(int width, int height)
{
	/// No need to repeat mem allocation.
	if( (_width != width)||(_height != height) )
	{
		DeleteMem();		///< Clean the house out first.
		_width = width; 
		_height = height; 
		_length = width*height;
		/// The block mem.
		_pBlk = NULL;
		_pBlk = new short[_length];
		/// The block overlay.
		_blk = NULL;
		_blk = new OverlayMem2Dv2(_pBlk, _width, _height, _width, _height);
		/// Household check.
		if( (_pBlk != NULL)&&(_blk != NULL) )
			_ready = 1;
	}//end if width...
}//end SetDim.

/** Copy from the blk into the buffer.
No checking is done for overflow and is assumed to 
be _length * short in size.
@param buff	: Buffer to copy into.
@return			: none.
*/
void BlockH264::Copy(void* buff)
{
	if(buff != NULL)
  	memcpy(buff, (const void *)_pBlk, _length * sizeof(short));
}//end Copy.

/** Copy a row from the blk into the buffer.
No checking is done for overflow and is assumed to 
be _length * short in size.
@param row	: Block row to copy from.
@param buff	: Buffer to copy into.
@return			: none.
*/
void BlockH264::CopyRow(int row, void* buff)
{
	if(buff == NULL)
		return;
	short* pSrc = &(_pBlk[_width*row]);
	memcpy(buff, (const void *)pSrc, _width * sizeof(short));
}//end CopyRow.

/** Copy a col from the blk into the buffer.
No checking is done for overflow and is assumed to 
be _length * short in size.
@param col	: Block col to copy from.
@param buff	: Buffer to copy into.
@return			: none.
*/
void BlockH264::CopyCol(int col, void* buff)
{
	if(buff == NULL)
		return;
	short*	pDst	= (short *)buff;
	int			i			= col;
	for(int j = 0; j < _height; j++, i += _width)
		pDst[j] = _pBlk[i];
}//end CopyCol.

/** Zero all elements in the block.
No checking is done for overflow and is assumed to 
be _length * short in size.
@return			: none.
*/
void BlockH264::Zero(void)
{
	short* pSrc = &(_pBlk[0]);
	memset((void *)pSrc, 0, _length * sizeof(short));
}//end Zero.

