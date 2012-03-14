/** @file

MODULE				: MtRGB24toYUV420Converter

FILE NAME			: MtRGB24toYUV420Converter.cpp

DESCRIPTION			: Double precision floating point RGB 24 bit to YUV420 colour 
					convertion derived from the RGBtoYUV420Converter base class.
					YUV is represented with 8 bpp. Use this implementation as 
					the reference.

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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "MtRGB24toYUV420Converter.h"

#ifdef USE_MULTI_THREADED

#include <boost/bind.hpp>
/*
===========================================================================
	Constants.
===========================================================================
*/
#define RRGB24YUVCI2_00		 0.299
#define RRGB24YUVCI2_01		 0.587
#define RRGB24YUVCI2_02		 0.114
#define RRGB24YUVCI2_10		-0.147
#define RRGB24YUVCI2_11		-0.289
#define RRGB24YUVCI2_12		 0.436
#define RRGB24YUVCI2_20		 0.615
#define RRGB24YUVCI2_21		-0.515
#define RRGB24YUVCI2_22		-0.100

const int iPrecision = 12;
const int iRRGB24YUVCI2_00 = static_cast<int>(0.299 * (2 << iPrecision));
const int iRRGB24YUVCI2_01 = static_cast<int>(0.587 * (2 << iPrecision));
const int iRRGB24YUVCI2_02 = static_cast<int>(0.114 * (2 << iPrecision));
const int iRRGB24YUVCI2_10 = static_cast<int>(-0.147 * (2 << iPrecision));
const int iRRGB24YUVCI2_11 = static_cast<int>(-0.289 * (2 << iPrecision));
const int iRRGB24YUVCI2_12 = static_cast<int>(0.436 * (2 << iPrecision));
const int iRRGB24YUVCI2_20 = static_cast<int>(0.615 * (2 << iPrecision));
const int iRRGB24YUVCI2_21 = static_cast<int>(-0.515 * (2 << iPrecision));
const int iRRGB24YUVCI2_22 = static_cast<int>(-0.100 * (2 << iPrecision));


#define RRGB24YUVCI2_RANGECHECK_0TO255(x) ( (((x) <= 255)&&((x) >= 0))?((x)):( ((x) > 255)?(255):(0) ) )
#define RRGB24YUVCI2_RANGECHECK_N128TO127(x) ( (((x) <= 127)&&((x) >= -128))?((x)):( ((x) > 127)?(127):(-128) ) )

void MtRGB24toYUV420Converter::init()
{
  for (std::size_t i = 0; i < 2; ++i)
    threads.create_thread(boost::bind(&boost::asio::io_service::run, boost::ref(io_service)));
}

/*
===========================================================================
	Interface Methods.
===========================================================================
*/
/** Double precision reference implementation.
The full real matix equation is used. The YUV output is represented with
8 bits per pel and the UV components are adjusted from their -128..127 range
to 0..255.
@param pRgb	: Packed RGB 888 format.
@param pY		: Lum plane.
@param pU		: Chr U plane.
@param pV		: Chr V plane.
@return			: none.
*/
void MtRGB24toYUV420Converter::Convert(void* pRgb, void* pY, void* pU, void* pV)
{
  boost::unique_lock<boost::mutex> lock(mutex);
  
  // post work to io_services
  unsigned uiPixels = _width * _height;
  unsigned uiYuvSize = uiPixels >> 2;

  unsigned uiRgbOffset = (_width * _height * 3) >> 1;

  yuvType*	py = (yuvType *)pY;
  yuvType*	pu = (yuvType *)pU;
  yuvType*	pv = (yuvType *)pV;
  unsigned char* src = (unsigned char *)pRgb;

  finished1 = false;
  finished2 = false;
  io_service.post(boost::bind(&MtRGB24toYUV420Converter::doConvert, this, src, py, pu, pv, 0));
  io_service.post(boost::bind(&MtRGB24toYUV420Converter::doConvert, this, src + ((uiPixels*3)>> 1), py + (uiPixels>>1), pu + (uiYuvSize>>1), pv+ (uiYuvSize>>1), 1));

  //while(!finished1 && !finished2)
  //{
  //  condvar.wait(lock);
  //}
  while(!finished1 || !finished2)
  {
    condvar.wait(lock);
  }

  if (uiConverted1 != uiConverted2)
  {
    int t = 0;
  }
}//end Convert.

void MtRGB24toYUV420Converter::doConvert(unsigned char* pRgb, yuvType* pY, yuvType* pU, yuvType* pV, unsigned uiId)
{
  if (uiId == 0)
  {
    {
      boost::lock_guard<boost::mutex> lock(mutex);
      finished1 = false;
    }
  }
  else
  {
    {
      boost::lock_guard<boost::mutex> lock(mutex);
      finished2 = false;
    }
  }

//#define USE_INTEGER_ARITHMETIC
//#define USE_LOOKUP_TABLE
#define USE_ORIGINAL

#ifdef USE_LOOKUP_TABLE
  yuvType*	py = (yuvType *)pY;
  yuvType*	pu = (yuvType *)pU;
  yuvType*	pv = (yuvType *)pV;
  unsigned char* src = (unsigned char *)pRgb;

  /// Y have range 0..255, U & V have range -128..127.
  double	u,v;
  int	r,g,b;

  /// Step in 2x2 pel blocks. (4 pels per block).
  int xBlks = _width >> 1;
  int yBlks = _height >> 2; // FOR MT
  for(int yb = 0; yb < yBlks; ++yb)
    for(int xb = 0; xb < xBlks; ++xb)
    {
      int							chrOff	= yb*xBlks + xb;
      int							lumOff	= (yb*_width + xb) << 1;
      unsigned char*	t				= src + lumOff*3;

      /// Top left pel.
      b = (*t++);
      g = (*t++);
      r = (*t++);
      py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(/*(int)*/(0.5 + m_RRGB24YUVCI2_00[r] + m_RRGB24YUVCI2_01[g] + m_RRGB24YUVCI2_02[b]));

      u = m_RRGB24YUVCI2_10[r] + m_RRGB24YUVCI2_11[g] + m_RRGB24YUVCI2_12[b];
      v = m_RRGB24YUVCI2_20[r] + m_RRGB24YUVCI2_21[g] + m_RRGB24YUVCI2_22[b];

      /// Top right pel.
      b = (*t++);
      g = (*t++);
      r = (*t++);
      py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(/*(int)*/(0.5 + m_RRGB24YUVCI2_00[r] + m_RRGB24YUVCI2_01[g] + m_RRGB24YUVCI2_02[b]));

      u += m_RRGB24YUVCI2_10[r] + m_RRGB24YUVCI2_11[g] + m_RRGB24YUVCI2_12[b];
      v += m_RRGB24YUVCI2_20[r] + m_RRGB24YUVCI2_21[g] + m_RRGB24YUVCI2_22[b];

      lumOff += _width;
      t = t + _width*3 - 6;
      /// Bottom left pel.
      b = (*t++);
      g = (*t++);
      r = (*t++);
      py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(/*(int)*/(0.5 + m_RRGB24YUVCI2_00[r] + m_RRGB24YUVCI2_01[g] + m_RRGB24YUVCI2_02[b]));

      u += m_RRGB24YUVCI2_10[r] + m_RRGB24YUVCI2_11[g] + m_RRGB24YUVCI2_12[b];
      v += m_RRGB24YUVCI2_20[r] + m_RRGB24YUVCI2_21[g] + m_RRGB24YUVCI2_22[b];

      /// Bottom right pel.
      b = (*t++);
      g = (*t++);
      r = (*t++);
      py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(/*(int)*/(0.5 + m_RRGB24YUVCI2_00[r] + m_RRGB24YUVCI2_01[g] + m_RRGB24YUVCI2_02[b]));
      //py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((int)(0.5 + RRGB24YUVCI2_00*r + RRGB24YUVCI2_01*g + RRGB24YUVCI2_02*b));

      u += m_RRGB24YUVCI2_10[r] + m_RRGB24YUVCI2_11[g] + m_RRGB24YUVCI2_12[b];
      v += m_RRGB24YUVCI2_20[r] + m_RRGB24YUVCI2_21[g] + m_RRGB24YUVCI2_22[b];

      /// Average the 4 chr values.
      int iu = (int)u;
      int iv = (int)v;
      if(iu < 0)	///< Rounding.
        iu -= 2;
      else
        iu += 2;
      if(iv < 0)	///< Rounding.
        iv -= 2;
      else
        iv += 2;

      pu[chrOff] = (yuvType)( _chrOff + RRGB24YUVCI2_RANGECHECK_N128TO127(iu >> 2) );
      pv[chrOff] = (yuvType)( _chrOff + RRGB24YUVCI2_RANGECHECK_N128TO127(iv >> 2) );
    }//end for xb & yb...
#endif


#ifdef USE_INTEGER_ARITHMETIC
    yuvType*	py = (yuvType *)pY;
    yuvType*	pu = (yuvType *)pU;
    yuvType*	pv = (yuvType *)pV;
    unsigned char* src = (unsigned char *)pRgb;

    /// Y have range 0..255, U & V have range -128..127.
    int	u,v;
    int	r,g,b;

    /// Step in 2x2 pel blocks. (4 pels per block).
    int xBlks = _width >> 1;
    int yBlks = _height >> 2; // FOR MT
    for(int yb = 0; yb < yBlks; yb++)
      for(int xb = 0; xb < xBlks; xb++)
      {
        int							chrOff	= yb*xBlks + xb;
        int							lumOff	= (yb*_width + xb) << 1;
        unsigned char*	t				= src + lumOff*3;

        /// Top left pel.
        b = (int)(*t++);
        g = (int)(*t++);
        r = (int)(*t++);
        py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(((iRRGB24YUVCI2_00*r + iRRGB24YUVCI2_01*g + iRRGB24YUVCI2_02*b)>> (iPrecision + 1)));

        u = iRRGB24YUVCI2_10*r + iRRGB24YUVCI2_11*g + iRRGB24YUVCI2_12*b;
        v = iRRGB24YUVCI2_20*r + iRRGB24YUVCI2_21*g + iRRGB24YUVCI2_22*b;

        /// Top right pel.
        b = (int)(*t++);
        g = (int)(*t++);
        r = (int)(*t++);
        py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(((iRRGB24YUVCI2_00*r + iRRGB24YUVCI2_01*g + iRRGB24YUVCI2_02*b)>> (iPrecision + 1)));

        u += iRRGB24YUVCI2_10*r + iRRGB24YUVCI2_11*g + iRRGB24YUVCI2_12*b;
        v += iRRGB24YUVCI2_20*r + iRRGB24YUVCI2_21*g + iRRGB24YUVCI2_22*b;

        lumOff += _width;
        t = t + _width*3 - 6;
        /// Bottom left pel.
        b = (int)(*t++);
        g = (int)(*t++);
        r = (int)(*t++);
        py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(((iRRGB24YUVCI2_00*r + iRRGB24YUVCI2_01*g + iRRGB24YUVCI2_02*b)>> (iPrecision + 1)));

        u += iRRGB24YUVCI2_10*r + iRRGB24YUVCI2_11*g + iRRGB24YUVCI2_12*b;
        v += iRRGB24YUVCI2_20*r + iRRGB24YUVCI2_21*g + iRRGB24YUVCI2_22*b;

        /// Bottom right pel.
        b = (int)(*t++);
        g = (int)(*t++);
        r = (int)(*t++);
        py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(((iRRGB24YUVCI2_00*r + iRRGB24YUVCI2_01*g + iRRGB24YUVCI2_02*b)>> (iPrecision + 1)));

        u += iRRGB24YUVCI2_10*r + iRRGB24YUVCI2_11*g + iRRGB24YUVCI2_12*b;
        v += iRRGB24YUVCI2_20*r + iRRGB24YUVCI2_21*g + iRRGB24YUVCI2_22*b;

        /// Average the 4 chr values.
        int iu = u >> (iPrecision + 1);
        int iv = v >> (iPrecision + 1);
        if(iu < 0)	///< Rounding.
          iu -= 2;
        else
          iu += 2;
        if(iv < 0)	///< Rounding.
          iv -= 2;
        else
          iv += 2;

        pu[chrOff] = (yuvType)( _chrOff + RRGB24YUVCI2_RANGECHECK_N128TO127(iu >> 2) );
        pv[chrOff] = (yuvType)( _chrOff + RRGB24YUVCI2_RANGECHECK_N128TO127(iv >> 2) );
      }//end for xb & yb...
#endif

#ifdef USE_ORIGINAL
      yuvType*	py = (yuvType *)pY;
      yuvType*	pu = (yuvType *)pU;
      yuvType*	pv = (yuvType *)pV;
      unsigned char* src = (unsigned char *)pRgb;

      /// Y have range 0..255, U & V have range -128..127.
      double	u,v;
      double	r,g,b;

      /// Step in 2x2 pel blocks. (4 pels per block).
      int xBlks = _width >> 1;
      int yBlks = _height >> 2; // shift 2 since we're letting each thread only do half the work
      for(int yb = 0; yb < yBlks; yb++)
        for(int xb = 0; xb < xBlks; xb++)
        {
          int							chrOff	= yb*xBlks + xb;
          int							lumOff	= (yb*_width + xb) << 1;
          unsigned char*	t				= src + lumOff*3;

          /// Top left pel.
          b = (double)(*t++);
          g = (double)(*t++);
          r = (double)(*t++);
          py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((int)(0.5 + RRGB24YUVCI2_00*r + RRGB24YUVCI2_01*g + RRGB24YUVCI2_02*b));

          u = RRGB24YUVCI2_10*r + RRGB24YUVCI2_11*g + RRGB24YUVCI2_12*b;
          v = RRGB24YUVCI2_20*r + RRGB24YUVCI2_21*g + RRGB24YUVCI2_22*b;

          /// Top right pel.
          b = (double)(*t++);
          g = (double)(*t++);
          r = (double)(*t++);
          py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((int)(0.5 + RRGB24YUVCI2_00*r + RRGB24YUVCI2_01*g + RRGB24YUVCI2_02*b));

          u += RRGB24YUVCI2_10*r + RRGB24YUVCI2_11*g + RRGB24YUVCI2_12*b;
          v += RRGB24YUVCI2_20*r + RRGB24YUVCI2_21*g + RRGB24YUVCI2_22*b;

          lumOff += _width;
          t = t + _width*3 - 6;
          /// Bottom left pel.
          b = (double)(*t++);
          g = (double)(*t++);
          r = (double)(*t++);
          py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((int)(0.5 + RRGB24YUVCI2_00*r + RRGB24YUVCI2_01*g + RRGB24YUVCI2_02*b));

          u += RRGB24YUVCI2_10*r + RRGB24YUVCI2_11*g + RRGB24YUVCI2_12*b;
          v += RRGB24YUVCI2_20*r + RRGB24YUVCI2_21*g + RRGB24YUVCI2_22*b;

          /// Bottom right pel.
          b = (double)(*t++);
          g = (double)(*t++);
          r = (double)(*t++);
          py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((int)(0.5 + RRGB24YUVCI2_00*r + RRGB24YUVCI2_01*g + RRGB24YUVCI2_02*b));

          u += RRGB24YUVCI2_10*r + RRGB24YUVCI2_11*g + RRGB24YUVCI2_12*b;
          v += RRGB24YUVCI2_20*r + RRGB24YUVCI2_21*g + RRGB24YUVCI2_22*b;

          /// Average the 4 chr values.
          int iu = (int)u;
          int iv = (int)v;
          if(iu < 0)	///< Rounding.
            iu -= 2;
          else
            iu += 2;
          if(iv < 0)	///< Rounding.
            iv -= 2;
          else
            iv += 2;

          pu[chrOff] = (yuvType)( _chrOff + RRGB24YUVCI2_RANGECHECK_N128TO127(iu/4) );
          pv[chrOff] = (yuvType)( _chrOff + RRGB24YUVCI2_RANGECHECK_N128TO127(iv/4) );
        }//end for xb & yb...
#endif


  if (uiId == 0)
  {
    {
      boost::lock_guard<boost::mutex> lock(mutex);
      ++uiConverted1;
      finished1 = true;
    }
    condvar.notify_one();
  }
  else
  {
    {
      boost::lock_guard<boost::mutex> lock(mutex);
      ++uiConverted2;
      finished2 = true;
    }
    condvar.notify_one();
  }

}//end Convert.

#endif
