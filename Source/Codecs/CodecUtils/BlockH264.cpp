/** @file

MODULE				: BlockH264

TAG						: BH264

FILE NAME			: BlockH264.cpp

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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
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

	/// The widthxheight block mem.
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
	if(buff == NULL)
		return;
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

