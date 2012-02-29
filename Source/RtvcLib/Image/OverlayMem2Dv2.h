/** @file

MODULE				: OverlayMem2Dv2

TAG						: OM2DV2

FILE NAME			: OverlayMem2Dv2.h

DESCRIPTION		: A class to overlay a two-dimensional mem structure onto
								a contiguous block (usually larger) of memory and provide
								several operations on 2-D blocks where the data type is
								short (16-bit variables). Static proxies are used	to reduce 
								the memory footprint of the class.

COPYRIGHT			: (c)CSIR 2007-2010 all rights resevered

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

===========================================================================
*/
#ifndef _OVERLAYMEM2DV2_H
#define _OVERLAYMEM2DV2_H

class OverlayMem2Dv2
{
	/// Construction and destruction.
public:
	OverlayMem2Dv2(void* srcPtr, int srcWidth, int srcHeight, int width, int height);
	virtual ~OverlayMem2Dv2();

protected:
	void	ResetMembers(void);
	void	Destroy(void);
	int   SetMem(void* srcPtr, int srcWidth, int srcHeight);

	/// Member access.
public:
	int		GetWidth(void)	{ return(_width); }
	int		GetHeight(void)	{ return(_height); }
	void	SetOverlayDim(int width, int height) { _width = width; _height = height; }

	int		GetOriginX(void) { return(_xPos); }
	int		GetOriginY(void) { return(_yPos); }
	/// Overridable methods.
	virtual void	SetOrigin(int x, int y);

	short**	Get2DSrcPtr(void) { return(_pBlock); }

	/// Interface: Input/output functions.
public:

	/// Write a single value.
	void Write(int toCol, int toRow, int value) 
		{ _pBlock[_yPos + toRow][_xPos + toCol] = (short)value; }

	/// All of the source to part of the block.
	int	Write(OverlayMem2Dv2&	srcBlock, int	toCol, int toRow)
		{ return(Write(*this, srcBlock, toCol, toRow)); }
	static int Write(OverlayMem2Dv2& me, OverlayMem2Dv2&	srcBlock, int	toCol, int toRow);

	/// Part of the source to part of the block.
	int	Write(OverlayMem2Dv2&	srcBlock,	int fromCol,	int	fromRow,
																		int toCol,		int	toRow,
																		int cols,			int	rows)
		{ return(Write(*this, srcBlock,	fromCol, fromRow, toCol, toRow, cols, rows)); }
	static int Write(OverlayMem2Dv2& me, OverlayMem2Dv2&	srcBlock,	
									 int fromCol,			 int	fromRow,
									 int toCol,				 int	toRow,
									 int cols,				 int	rows);

	/// All of the source to all of the block. Must be equal mem footprint.
	int	Write(OverlayMem2Dv2&	srcBlock)
		{ return(Write(*this, srcBlock)); }
	static int Write(OverlayMem2Dv2& me, OverlayMem2Dv2&	srcBlock);
	int	Write4x4(OverlayMem2Dv2&	srcBlock)
		{ return(Write4x4(*this, srcBlock)); }
	static int Write4x4(OverlayMem2Dv2& me, OverlayMem2Dv2&	srcBlock);
	int	Write8x8(OverlayMem2Dv2&	srcBlock)
		{ return(Write8x8(*this, srcBlock)); }
	static int Write8x8(OverlayMem2Dv2& me, OverlayMem2Dv2&	srcBlock);
	int	Write16x16(OverlayMem2Dv2&	srcBlock)
		{ return(Write16x16(*this, srcBlock)); }
	static int Write16x16(OverlayMem2Dv2& me, OverlayMem2Dv2&	srcBlock);

	/// Read a single value.
	int	Read(int fromCol, int fromRow) 
		{ return(_pBlock[_yPos + fromRow][_xPos + fromCol]); }

	/// Part of the block to part/all of the destination.
	void Read(OverlayMem2Dv2&	dstBlock, int toCol,		int toRow,
																			int fromCol,	int fromRow,
																			int cols,			int rows)
		{ Read(*this, dstBlock, toCol, toRow, fromCol, fromRow, cols, rows); }
	static void Read(OverlayMem2Dv2& me, OverlayMem2Dv2&	dstBlock, 
									 int toCol,				 int toRow,
									 int fromCol,			 int fromRow,
									 int cols,				 int rows);

	/// All of the block to all of the destination. Must be equal footprint.
	int Read(OverlayMem2Dv2&	dstBlock)
		{ return(Read(*this, dstBlock)); }
	static int Read(OverlayMem2Dv2& me, OverlayMem2Dv2&	dstBlock);

	/// All of the block at 1/2, 1/4 and 1/8 pel position to all of the destination.
	void HalfRead(OverlayMem2Dv2& dstBlock, int halfColOff, int halfRowOff)
		{ HalfRead(*this, dstBlock, halfColOff, halfRowOff); }
	static void HalfRead(OverlayMem2Dv2& me, OverlayMem2Dv2& dstBlock, int halfColOff, int halfRowOff);
	void HalfReadv2(OverlayMem2Dv2& dstBlock, int halfColOff, int halfRowOff)
		{ HalfReadv2(*this, dstBlock, halfColOff, halfRowOff); }
	static void HalfReadv2(OverlayMem2Dv2& me, OverlayMem2Dv2& dstBlock, int halfColOff, int halfRowOff);
	void QuarterRead(OverlayMem2Dv2& dstBlock, int quarterColOff, int quarterRowOff)
		{ QuarterRead(*this, dstBlock, quarterColOff, quarterRowOff); }
	static void QuarterRead(OverlayMem2Dv2& me, OverlayMem2Dv2& dstBlock, int quarterColOff, int quarterRowOff);
	void EighthRead(OverlayMem2Dv2& dstBlock, int eighthColOff, int eighthRowOff)
		{ EighthRead(*this, dstBlock, eighthColOff, eighthRowOff); }
	static void EighthRead(OverlayMem2Dv2& me, OverlayMem2Dv2& dstBlock, int eighthColOff, int eighthRowOff);

	/// Interface: Operation functions.
public:

	/// Set block values to zero.
	void Clear(void)
		{ Clear(*this); }
	static void Clear(OverlayMem2Dv2& me);
	/// Set block values to a value.
	void Fill(int value)
		{ Fill(*this, value); }
	static void Fill(OverlayMem2Dv2& me, int value);
	/// Sum all the elements in the block.
	int Sum(void)
		{ return(Sum(*this)); }
	static int Sum(OverlayMem2Dv2& me);
	/// Subtract the input block from this.
	int Sub(OverlayMem2Dv2& b)
		{ return( Sub(*this, b) ); }
	static int Sub(OverlayMem2Dv2& me, OverlayMem2Dv2& b);
	/// Subtract the 16x16 input block from this.
	int Sub16x16(OverlayMem2Dv2& b)
		{ return( Sub16x16(*this, b) ); }
	static int Sub16x16(OverlayMem2Dv2& me, OverlayMem2Dv2& b);
	/// Subtract the 8x8 input block from this.
	int Sub8x8(OverlayMem2Dv2& b)
		{ return( Sub8x8(*this, b) ); }
	static int Sub8x8(OverlayMem2Dv2& me, OverlayMem2Dv2& b);
	/// Subtract the input constant from this.
	int Sub(short z)
		{ return( Sub(*this, z) ); }
	static int Sub(OverlayMem2Dv2& me, short z);
	/// Add the input block to this.
	int Add(OverlayMem2Dv2& b)
		{ return( Add(*this, b) ); }
	static int Add(OverlayMem2Dv2& me, OverlayMem2Dv2& b);
	int AddWithClip255(OverlayMem2Dv2& b)
		{ return( AddWithClip255(*this, b) ); }
	static int AddWithClip255(OverlayMem2Dv2& me, OverlayMem2Dv2& b);
	/// Add the 16x16 input block to this.
	int Add16x16(OverlayMem2Dv2& b)
		{ return( Add16x16(*this, b) ); }
	static int Add16x16(OverlayMem2Dv2& me, OverlayMem2Dv2& b);
	int Add16x16WithClip255(OverlayMem2Dv2& b)
		{ return( Add16x16WithClip255(*this, b) ); }
	static int Add16x16WithClip255(OverlayMem2Dv2& me, OverlayMem2Dv2& b);
	/// Add the 8x8 input block to this.
	int Add8x8(OverlayMem2Dv2& b)
		{ return( Add8x8(*this, b) ); }
	static int Add8x8(OverlayMem2Dv2& me, OverlayMem2Dv2& b);
	int Add8x8WithClip255(OverlayMem2Dv2& b)
		{ return( Add8x8WithClip255(*this, b) ); }
	static int Add8x8WithClip255(OverlayMem2Dv2& me, OverlayMem2Dv2& b);
	/// Scale block values down.
	int ScaleDown(short z)
		{ return( ScaleDown(*this, z) ); }
	static int ScaleDown(OverlayMem2Dv2& me, short z);
	///  Scale block values up.
	int ScaleUp(short z)
		{ return( ScaleUp(*this, z) ); }
	static int ScaleUp(OverlayMem2Dv2& me, short z);
	/// Multiply block values by a scalar.
	int Multiply(short z)
		{ return( Multiply(*this, z) ); }
	static int Multiply(OverlayMem2Dv2& me, short z);
	/// Divide block values by a scalar.
	int Divide(short z)
		{ return( Divide(*this, z) ); }
	static int Divide(OverlayMem2Dv2& me, short z);

	/// Calc total square difference with the input block.
	int Tsd(OverlayMem2Dv2& b)
		{ return( Tsd(*this, b) ); }
	static int Tsd(OverlayMem2Dv2& me, OverlayMem2Dv2& b);
	int Tsd4x4(OverlayMem2Dv2& b)
		{ return( Tsd4x4(*this, b) ); }
	static int Tsd4x4(OverlayMem2Dv2& me, OverlayMem2Dv2& b);			///< Fast for 4x4 blocks.
	int Tsd8x8(OverlayMem2Dv2& b)
		{ return( Tsd8x8(*this, b) ); }
	static int Tsd8x8(OverlayMem2Dv2& me, OverlayMem2Dv2& b);			///< Fast for 8x8 blocks.
	int Tsd16x16(OverlayMem2Dv2& b)
		{ return( Tsd16x16(*this, b) ); }
	static int Tsd16x16(OverlayMem2Dv2& me, OverlayMem2Dv2& b);		///< Fast for 16x16 blocks.

	/// The total square difference with the input to improve on an input value.
	int TsdLessThan(OverlayMem2Dv2& b, int min)
		{ return( TsdLessThan(*this, b, min) ); }
	static int TsdLessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min);
	int Tsd4x4LessThan(OverlayMem2Dv2& b, int min)
		{ return( Tsd4x4LessThan(*this, b, min) ); }
	static int Tsd4x4LessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min);		///< Fast for 4x4 blocks.
	int Tsd4x4PartialLessThan(OverlayMem2Dv2& b, int min)
		{ return( Tsd4x4PartialLessThan(*this, b, min) ); }
	static int Tsd4x4PartialLessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min);	///< Fast for 4x4 blocks.
	int Tsd8x8LessThan(OverlayMem2Dv2& b, int min)
		{ return( Tsd8x8LessThan(*this, b, min) ); }
	static int Tsd8x8LessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min);		///< Fast for 8x8 blocks.
	int Tsd8x8PartialLessThan(OverlayMem2Dv2& b, int min)
		{ return( Tsd8x8PartialLessThan(*this, b, min) ); }
	static int Tsd8x8PartialLessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min);	///< Fast for 8x8 blocks.
	int Tsd16x16LessThan(OverlayMem2Dv2& b, int min)
		{ return( Tsd16x16LessThan(*this, b, min) ); }
	static int Tsd16x16LessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min);	///< Fast for 16x16 blocks.
	int Tsd16x16PartialLessThan(OverlayMem2Dv2& b, int min)
		{ return( Tsd16x16PartialLessThan(*this, b, min) ); }
	static int Tsd16x16PartialLessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min);	///< Fast for 16x16 blocks.

	/// Calc total absolute difference with the input block.
	int Tad(OverlayMem2Dv2& b)
		{ return( Tad(*this, b) ); }
	static int Tad(OverlayMem2Dv2& me, OverlayMem2Dv2& b);
	int Tad4x4(OverlayMem2Dv2& b)
		{ return( Tad4x4(*this, b) ); }
	static int Tad4x4(OverlayMem2Dv2& me, OverlayMem2Dv2& b);			///< Fast for 4x4 blocks.
	int Tad8x8(OverlayMem2Dv2& b)
		{ return( Tad8x8(*this, b) ); }
	static int Tad8x8(OverlayMem2Dv2& me, OverlayMem2Dv2& b);			///< Fast for 8x8 blocks.
	int Tad16x16(OverlayMem2Dv2& b)
		{ return( Tad16x16(*this, b) ); }
	static int Tad16x16(OverlayMem2Dv2& me, OverlayMem2Dv2& b);		///< Fast for 16x16 blocks.

	/// The total abs difference with the input to improve on an input value.
	int TadLessThan(OverlayMem2Dv2& b, int min)
		{ return( TadLessThan(*this, b, min) ); }
	static int TadLessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min);
	int Tad4x4LessThan(OverlayMem2Dv2& b, int min)
		{ return( Tad4x4LessThan(*this, b, min) ); }
	static int Tad4x4LessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min);		///< Fast for 4x4 blocks.
	int Tad8x8LessThan(OverlayMem2Dv2& b, int min)
		{ return( Tad8x8LessThan(*this, b, min) ); }
	static int Tad8x8LessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min);		///< Fast for 8x8 blocks.
	int Tad16x16LessThan(OverlayMem2Dv2& b, int min)
		{ return( Tad16x16LessThan(*this, b, min) ); }
	static int Tad16x16LessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min);	///< Fast for 16x16 blocks.

	/// Block test operators.
	int Equals(OverlayMem2Dv2& b)
		{ return( Equals(*this, b) ); }
	static int Equals(OverlayMem2Dv2& me, OverlayMem2Dv2& b);

	/// Static independent helper functions.
public:
	/// Sub sample the src by half into another 2D mem block with possible offset.
	static void Half(void**	srcPtr, int srcWidth,			int srcHeight,
									 void** dstPtr, int widthOff = 0, int heightOff = 0);

protected:
	/// Class constants
	static int OM2DV2_Sp[17];
	static int OM2DV2_Tp[17];

protected:
	int					_width;				///< Overlay mem width and height.
	int					_height;
	int					_srcWidth;		///< Underlying mem width, height and ptr
	int					_srcHeight;		///< initialised in the constructor.
	short*			_pMem;

	/// Mem is 2-D with row address array for speed.
	short**			_pBlock;		///< Rows within _srcPtr;

	///< Origin of the top left corner of the overlay block within the mem.
	int					_xPos;
	int					_yPos;

};// end class OverlayMem2Dv2.

#endif	//end _OVERLAYMEM2DV2_H.
