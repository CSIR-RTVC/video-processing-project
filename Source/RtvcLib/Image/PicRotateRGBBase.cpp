/** @file

MODULE                : PicRotateRGBBase

FILE NAME             : PicRotateRGBBase.cpp

DESCRIPTION           :
                     
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2012, CSIR
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the CSIR nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

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
#include "PicRotateRGBBase.h"
#include <DirectShow/CommonDefs.h>

PicRotateRGBBase::PicRotateRGBBase()
{
	m_eMode = ROTATE_NONE;
}

PicRotateRGBBase::~PicRotateRGBBase()
{;}

bool PicRotateRGBBase::Rotate( void* pInImg, void* pOutImg )
{
	if (!pInImg || !pOutImg) return false;

	int nBytesPerPixel = BytesPerPixel();
	switch (m_eMode)
	{
	case ROTATE_NONE:
		{
			memcpy(pOutImg, pInImg, m_nWidth * m_nHeight * nBytesPerPixel);
			return true;
		}
	case ROTATE_90_DEGREES_CLOCKWISE:
		{
			BYTE* pSrc = (BYTE*)pInImg;
			BYTE* pDest = NULL;
			BYTE* pSrcCol = NULL;
			for (int i = 0; i < m_nWidth; ++i, pSrc += nBytesPerPixel)
			{
				int nNewRow = (m_nWidth - i);//Index Base 1
				int nNewIndex = ((nNewRow - 1) * m_nHeight);// Base 1
				pSrcCol = pSrc;
				pDest = (BYTE*)pOutImg + (nNewIndex*nBytesPerPixel);
				for (int j = 0; j < m_nHeight; ++j, pSrcCol += (nBytesPerPixel*m_nWidth), pDest += nBytesPerPixel)
				{
					memcpy(pDest, pSrcCol, nBytesPerPixel);
				}
				// Copy pixels in same column to their destinations
			}
			return true;
		}
	case ROTATE_180_DEGREES_CLOCKWISE:
		{
			BYTE* pSrc = (BYTE*)pInImg;
			int nTotalPixels = m_nWidth * m_nHeight; 
			int nLastIndex = (nTotalPixels - 1) * nBytesPerPixel;
			BYTE* pDest = (BYTE*)pOutImg + nLastIndex;
			for (int i = 0; i < nTotalPixels; ++i, pSrc += nBytesPerPixel, pDest -= nBytesPerPixel)
			{
				memcpy(pDest, pSrc, sizeof(BYTE) *nBytesPerPixel);
			}
			return true;
		}

	case ROTATE_270_DEGREES_CLOCKWISE:
		{
			int iTargetRowLength = m_nHeight * nBytesPerPixel;
			BYTE* pSrc = (BYTE*)pInImg;
			BYTE* pDest = (BYTE*)pOutImg + iTargetRowLength - nBytesPerPixel;

			for ( int y = 0; y < m_nHeight; y++ )
			{
				BYTE* pDestPixel = pDest;
				for (int x = 0; x < m_nWidth; ++x, pSrc += nBytesPerPixel, pDestPixel += iTargetRowLength)
				{
					memcpy(pDestPixel, pSrc, nBytesPerPixel);
				}
				pDest -= nBytesPerPixel;
			}
			return true;
		}
	case ROTATE_FLIP_VERTICAL:
		{
			// Code to flip image
			int nRowLength = m_nWidth * nBytesPerPixel;
			BYTE* pSrc = (BYTE*)pInImg;
			//BYTE* pDest = (BYTE*)pOutImg + ((m_nHeight * nRowLength ) - nRowLength);
			BYTE* pDest = (BYTE*)pOutImg + ((m_nHeight - 1) * nRowLength);
			for (int i = 0; i < m_nHeight; ++i, pSrc += nRowLength, pDest -= nRowLength)
			{
				memcpy(pDest, pSrc, nRowLength);
			}
			return true;
		}
	case ROTATE_FLIP_HORIZONTAL:
		{
			int iRowLength = m_nWidth * nBytesPerPixel;
			BYTE* pSrc = (BYTE*)pInImg;
			BYTE* pDest = (BYTE*)pOutImg + iRowLength - nBytesPerPixel;

            for ( int y = 0; y < m_nHeight; y++ )
            {
				BYTE* pDestPixel = pDest;
                for ( int x = 0; x < m_nWidth; x++, pSrc += nBytesPerPixel, pDestPixel-=nBytesPerPixel )
                {
					memcpy(pDestPixel, pSrc, nBytesPerPixel);
                }
				pDest += iRowLength;
            }
			return true;
		}
	case ROTATE_FLIP_DIAGONALLY:
		{
			BYTE* pSrc = (BYTE*)pInImg;
			BYTE* pDest = NULL;
			BYTE* pSrcCol = NULL;
			for (int i = 0; i < m_nWidth; ++i, pSrc += nBytesPerPixel)
			{
				int nNewRow = (m_nWidth - i);//Index Base 1
				// Select target row to be one higher and subtract sizeof RGB block to be at last index of row
				++nNewRow;
				int nNewIndex = ((nNewRow - 1) * m_nHeight);// Base 1
				// Adjust index to previous row
				--nNewIndex;
				pSrcCol = pSrc;
				pDest = (BYTE*)pOutImg + (nNewIndex*nBytesPerPixel);
				for (int j = 0; j < m_nHeight; ++j, pSrcCol += (nBytesPerPixel*m_nWidth), pDest -= nBytesPerPixel)
				{
					memcpy(pDest, pSrcCol, nBytesPerPixel);
				}
				// Copy pixels in same column to their destinations
			}
			return true;
		}
	default:
		{
			// Unimplemented
			return false;
		}
	}
}
