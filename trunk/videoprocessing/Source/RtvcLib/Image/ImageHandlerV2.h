/** @file

MODULE						: ImageHandlerV2

TAG								: IH

FILE NAME					: ImageHandlerV2.h

DESCRIPTION				: Definition of a CImage class that holds images in
										DIB .bmp format and provides functions for colour 
										space conversion and the like.

REVISION HISTORY	:	

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#include <math.h>
#include <string>
#include <Windows.h>

using std::string;

#ifndef _IMAGEHANDLERV2_H
#define _IMAGEHANDLERV2_H

#pragma once

class ImageHandlerV2
{

  // Implementation
public:
	ImageHandlerV2();
	virtual ~ImageHandlerV2();

	int						CreateImage(BITMAPINFOHEADER *pbmpih);
	void					DestroyImage(void);
  int						ImageMemIsAlloc(void);
	int						ConvertToYUV(int YUVFormat);
  void					UpdateBmp(void);
	int						GetUVHeight(void);
	int						GetUVWidth(void);
	int						GetYHeight(void);
	int						GetYWidth(void);
  virtual void	SaveBmp(string filename);
  virtual void	SaveBmp(void);
  virtual int		LoadBmp(string filename);
  virtual int		LoadBmp(void);
  int						SaveRaw(string filename);
	string        GetErrorStr(void) { return(_errorStr); }
  

// Operations
public:
	ImageHandlerV2 &operator=(ImageHandlerV2 &I);
	ImageHandlerV2 &operator-=(ImageHandlerV2 &I);
	ImageHandlerV2 &operator+=(ImageHandlerV2 &I);

  int			Copy(ImageHandlerV2& fromImg);
  int			EqualMemFormat(ImageHandlerV2& img);
  void		Absolute(void);
  double	PSNR(ImageHandlerV2& noiseImg);
	int			IncreaseLum(int By);
	int			DecreaseLum(int By);
  void		MirrorXLeft(int PixPoint);
  void		MirrorXRight(int PixPoint);
  void		MirrorYUp(int PixPoint);
  void		MirrorYDown(int PixPoint);
  void		KillColor(void);

protected:
	int						CreateYUVMem(int YUVType);
  void					DestroyYUVMem(void);
  void					ConvertRGB24BITtoYUV411(void);
  void					ConvertYUV411toRGB24BIT(void);
  void					ConvertRGB24BITtoYUV422(void);
  void					ConvertYUV422toRGB24BIT(void);
  void					ConvertRGB24BITtoYUV444(void);
  void					ConvertYUV444toRGB24BIT(void);

	void					RGBtoYUV(double R,double G,double B,
												 double &Y,double &U,double &V);
	void					RGBtoYUV(int R,int G,int B,
												 int &Y,int &U,int &V);
	void					YUVtoRGB(int Y,int U,int V,
												 int &R,int &G,int &B);

// Extra colour types.
public:
	static const int BMP_INFO_SPECIFIES = 0;
	static const int YUV411             = 1;
	static const int YUV422             = 2;
	static const int YUV444             = 3;

// Attributes
public:
	unsigned int			_paletteSize;
	void*							_pBmp;
	BITMAPINFOHEADER*	_bmih;
	RGBQUAD*					_bmic;
	void*							_bmptr;

	string						  _errorStr;
	int								_extraColorType;
	int*							_Y;
	int*							_U;
	int*							_V;
	int								_lumWidth;
	int								_lumHeight;
	int								_chrWidth;
	int								_chrHeight;

protected:
  string						_bmpFilename;

};//ImageHandlerV2 class.


#endif	//_IMAGEHANDLERV2_H
