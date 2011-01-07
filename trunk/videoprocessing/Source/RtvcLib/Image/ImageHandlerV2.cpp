/** @file

MODULE						: ImageHandlerV2

TAG								: IH

FILE NAME					: ImageHandlerV2.cpp

DESCRIPTION				: Implementation of a ImageHandlerV2 class that holds images 
										in DIB .bmp format and provides functions for colour 
										space conversion and the like.

REVISION HISTORY	:	

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "ImageHandlerV2.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
// RGB to YUV colour conversion matrix.
#define RtoY_00   (double)(0.299)
#define RtoY_01   (double)(0.587)
#define RtoY_02   (double)(0.114)
#define RtoY_10   (double)(-0.169)
#define RtoY_11   (double)(-0.331)
#define RtoY_12   (double)(0.5)
#define RtoY_20   (double)(0.5)
#define RtoY_21   (double)(-0.419)
#define RtoY_22   (double)(-0.081)

// YUV to RGB colour conversion matrix.
#define YtoR_00   (double)(1.00)
#define YtoR_01   (double)(-0.000927)
#define YtoR_02   (double)(1.401687)
#define YtoR_10   (double)(1.00)
#define YtoR_11   (double)(-0.343695)
#define YtoR_12   (double)(-0.714169)
#define YtoR_20   (double)(1.00)
#define YtoR_21   (double)(1.772160)
#define YtoR_22   (double)(0.00099)

/*
---------------------------------------------------------------------------
	Public class methods.
---------------------------------------------------------------------------
*/
ImageHandlerV2::ImageHandlerV2()
{
	_paletteSize		= 0;
	_pBmp						= NULL;
	_bmih						= NULL;
	_bmic						= NULL;
	_bmptr					= NULL;
	_errorStr				= "ImageHandlerV2: No error";
	_extraColorType = BMP_INFO_SPECIFIES;
	_Y							= NULL;
	_U							= NULL;
	_V							= NULL;
	_lumWidth				= 0;
	_lumHeight			= 0;
	_chrWidth				= 0;
	_chrHeight			= 0;
 _bmpFilename		= "default.bmp";

}//end ImageHandlerV2 Constructor.

ImageHandlerV2::~ImageHandlerV2()
{
  //Don't let the object go without cleaning up.
  DestroyImage();
}//end ImageHandlerV2 Destructor.

// Create memory space and copy the bitmap info
// header data.
// Return : 0 = error, 1 = OK.
int ImageHandlerV2::CreateImage(BITMAPINFOHEADER* pbmpih)
{
	if(pbmpih == NULL)
	{
		_errorStr = "ImageHandlerV2: Cannot create from empty info header";
		return(0);
	}//end if !pbmpih...

	// Clear out any old images.
	if(_pBmp != NULL)
		DestroyImage();
	
	if( (pbmpih->biCompression != BI_RGB) &&
		  (pbmpih->biCompression != BI_BITFIELDS) )
	  {
			_errorStr = "ImageHandlerV2: Image is not valid BMP format";
		  return(0);
	  }//end if biCompression...
	//Determine the image size.
	if(pbmpih->biSizeImage == 0)
	{
		pbmpih->biSizeImage = (DWORD)(((((pbmpih->biWidth * pbmpih->biBitCount)
																	+ 31)/32) * 4) * pbmpih->biHeight);
	}//end if biSizeImage...
	//Determine the palette colour size.
	if(pbmpih->biCompression == BI_RGB)
	{
		if((pbmpih->biClrUsed == 0)&&(pbmpih->biBitCount<=8))
			_paletteSize = (UINT)((1 << pbmpih->biBitCount) * sizeof(RGBQUAD));
		else
			_paletteSize = (UINT)(pbmpih->biClrUsed * sizeof(RGBQUAD));
	}
	else //if pbmpih->biCompression == BI_BITFIELDS
	{
		_paletteSize = (UINT)(3 * sizeof(DWORD));
	}//end else...

	// Allocate memory for the image and all headers as contiguous mem.
	_pBmp = (void *) new char[sizeof(BITMAPINFOHEADER) + _paletteSize + pbmpih->biSizeImage];
	if(!_pBmp)
	{
		_errorStr = "ImageHandlerV2: Insufficient memory for image";
	  return(0);
	}//end if !_pBmp...
	_bmih = (BITMAPINFOHEADER *)(_pBmp);

	// Load the bitmap info header.
	*(_bmih) = *(pbmpih);
	// Determine the pointers that are offset into the memory.
	_bmic		= (RGBQUAD *)((LPSTR)_bmih + (DWORD)sizeof(BITMAPINFOHEADER));
	_bmptr	= (LPVOID)((LPSTR)_bmic + _paletteSize);
	_extraColorType = BMP_INFO_SPECIFIES;

	return(1);
}//end CreateImage.

void ImageHandlerV2::DestroyImage(void)
{
	if(_pBmp != NULL)
		delete[] _pBmp;
	_pBmp					= NULL;
	_bmih					= NULL;
	_bmic					= NULL;
	_bmptr				= NULL;
	_paletteSize	= 0;
 _bmpFilename	= "default.bmp";

	DestroyYUVMem();
}//end DestroyImage.

int ImageHandlerV2::ImageMemIsAlloc(void)
{
  if(_pBmp==NULL)
    return(0);
  return(1);
}//end ImageMemIsAlloc.

// Create memory to store Y,U and V colour data. Convert
// from the current colour space to YUV and force
// the displayable bmp to reflect the new image
// in its current bmp colour format.
int ImageHandlerV2::ConvertToYUV(int YUVFormat)
{
	if(_extraColorType != BMP_INFO_SPECIFIES)
  {
    // Invalidate the YUV memory first.
  	DestroyYUVMem();
  }//end if _extraColorType...

	if(_bmih->biBitCount == 24)//RGB 24 bit.
	{
		if(!CreateYUVMem(YUVFormat))
		{
			_errorStr = "ImageHandlerV2: YUV memory unavailable";
			return(0);
		}//end if !CreateYUVMem...
		if( YUVFormat == YUV411 )
		  ConvertRGB24BITtoYUV411();
		else if( YUVFormat == YUV422 )
		  ConvertRGB24BITtoYUV422();
		else //if( YUVFormat == YUV444 )
		  ConvertRGB24BITtoYUV444();
	}//end else if biBitCount...
	else
	{
		_errorStr = "ImageHandlerV2: Colour conversion not available";
		return(0);
  }//end else...

	_extraColorType = YUVFormat;
	UpdateBmp();
	return(1);
}//end ConvertToYUV.

void ImageHandlerV2::UpdateBmp(void)
{
	if(_bmih->biBitCount == 24)
	{
		if(_extraColorType == YUV411)
			ConvertYUV411toRGB24BIT();
	  else if(_extraColorType == YUV422)
			ConvertYUV422toRGB24BIT();
	  else if(_extraColorType == YUV444)
			ConvertYUV444toRGB24BIT();
	}//end if biBitCount...
}//end UpdateBmp.

int ImageHandlerV2::GetYWidth(void)
{
	return(_lumWidth);
}//end GetYWidth.

int ImageHandlerV2::GetYHeight(void)
{
	return(_lumHeight);
}//end GetYHeight.

int ImageHandlerV2::GetUVWidth(void)
{
	return(_chrWidth);
}//end GetUVWidth.

int ImageHandlerV2::GetUVHeight(void)
{
	return(_chrHeight);
}//end GetUVHeight.

void ImageHandlerV2::SaveBmp(string filename)
{
  _bmpFilename = filename;
  SaveBmp();
}//end SaveBmp.

void ImageHandlerV2::SaveBmp(void)
{
	//FILE* Fl;
  FILE* CheckFile;

	CheckFile = fopen(_bmpFilename.c_str(),"wb");
  if(!CheckFile)
    return;

	// Make the file header.
  BITMAPFILEHEADER bmfh;
	bmfh.bfType = 0x4D42;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) +
		sizeof(BITMAPINFOHEADER) +
		_paletteSize + 
		_bmih->biSizeImage; 
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) +
		sizeof(BITMAPINFOHEADER) +
		_paletteSize;

  
  fwrite(&bmfh,1,sizeof(BITMAPFILEHEADER),CheckFile);
  fwrite(_bmih,1,sizeof(BITMAPINFOHEADER),CheckFile);
  fwrite(_bmic,1,_paletteSize,CheckFile);
  fwrite(_bmptr,1,_bmih->biSizeImage,CheckFile);

 

 fclose(CheckFile);

}//end SaveBmp.

int ImageHandlerV2::LoadBmp(string filename)
{
  _bmpFilename = filename;
  return(LoadBmp());
}//end LoadBmp.

int ImageHandlerV2::LoadBmp(void)
{
  BITMAPFILEHEADER	bmfh;
  BITMAPINFOHEADER	BitmapHead;
  UINT							ReadSize;
	//FILE	*						Fl;
	//int								CheckFile;
  FILE  *            CheckFile;

  // Make sure image space is cleared.
  string Temp = _bmpFilename;
  DestroyImage();
  _bmpFilename = Temp;

  CheckFile = fopen(_bmpFilename.c_str(),"rb");
  if(!CheckFile)
  {
		_errorStr = "ImageHandlerV2: Unable to open Bmp file";
    return(0);
  }//end if !CheckFile...

	//Read bitmap file header.
	ReadSize = fread((void *)(&bmfh),1,sizeof(BITMAPFILEHEADER),CheckFile);
	if(ReadSize != sizeof(BITMAPFILEHEADER))
	{
		_errorStr = "ImageHandlerV2: File is too short";
    fclose(CheckFile);
		return(0);
	}//end if ReadSize...
	if(bmfh.bfType != 0x4D42)
	{
		_errorStr = "ImageHandlerV2: File is not in BMP file format";
    fclose(CheckFile);
		return(0);
	}//end if bfType...
	//Read bitmap info header.
	ReadSize = fread((void *)(&BitmapHead),1,sizeof(BITMAPINFOHEADER),CheckFile);
	if(ReadSize != sizeof(BITMAPINFOHEADER))
	{
		_errorStr = "ImageHandlerV2: Unable to read from Bmp file";
    fclose(CheckFile);
		return(0);
	}//end if ReadSize...

  //Construct the new image.
  if(!CreateImage(&BitmapHead))
  {
   fclose(CheckFile);
    return(0);
  }//end if !CreateImage...

  //Read the colour palette.
	ReadSize = fread((void *)(_bmic),1,_paletteSize,CheckFile);
	if(ReadSize != _paletteSize)
	{
		_errorStr = "ImageHandlerV2: Unable to read palette from Bmp file";
    fclose(CheckFile);
		return(0);
	}//end if ReadSize...

  //Read the image bits.
	ReadSize = fread((void *)(_bmptr) ,1,_bmih->biSizeImage,CheckFile);
	if(ReadSize != _bmih->biSizeImage)
	{
		_errorStr = "ImageHandlerV2: Unable to read pixel bits from Bmp file";
    fclose(CheckFile);
		return(0);
	}//end if ReadSize...

fclose(CheckFile);

  return(1);
}//end LoadBmp.

int ImageHandlerV2::SaveRaw(string filename)
{
  //Save the displayed image as a raw data format.
  //The header is as follows:
  //  1. No. of colour components - int32.
  //  2. 1st colour comp. X size - int 32.
  //  3. 1st colour comp. Y size - int 32.
  //  4. 1st colour comp. data[X rows *Y cols.] - int32.
  //  5. 2nd colour comp. X, Y, data[X*Y].
  //  6. etc. etc. for all colour components.

  //Currently work on YUV image spaces only.
  if(_extraColorType == BMP_INFO_SPECIFIES)
  {
		_errorStr = "ImageHandlerV2: Operation only valid for YUV formats";
    return(0);
  }//end if !_extraColorType...
  
	//FILE* CheckFile;
	FILE* CheckFile;

	CheckFile = fopen(filename.c_str(),"wb");
  if(!CheckFile)
  {
		_errorStr = "ImageHandlerV2: Unable to open Raw file";
    return(0);
  }//end if !CheckFile...

  // Write out no. of colour components.
  int ColourComponents = 3;
  fwrite(&ColourComponents,1,sizeof(int),CheckFile);

  // Write out luminance component.
  int x = GetYWidth();
  int y = GetYHeight();

  fwrite(&x,1,sizeof(int),CheckFile);
  fwrite(&y,1,sizeof(int),CheckFile);
  fwrite(_Y,1, x * y * sizeof(int),CheckFile);

 

  // Write out U and V chrominance component.
  x = GetUVWidth();
  y = GetUVHeight();
 
  fwrite(&x,1,sizeof(int),CheckFile);
  fwrite(&y,1,sizeof(int),CheckFile);
  fwrite(_U,1, x * y * sizeof(int),CheckFile);

  fwrite(&x,1,sizeof(int),CheckFile);
  fwrite(&y,1,sizeof(int),CheckFile);
  fwrite(_V,1,x * y * sizeof(int),CheckFile);

 fclose(CheckFile);
  return(1);
}//end SaveRaw.

/*
---------------------------------------------------------------------------
	Operator overloads.
---------------------------------------------------------------------------
*/
ImageHandlerV2& ImageHandlerV2::operator=(ImageHandlerV2& I)
{
	if( &I == NULL)
		return *this;

	LPSTR Src,Dst;
	int Limit;

  if(!EqualMemFormat(I))
  {
    //Destroy old image.
    DestroyImage();

    // Create new image space.
    if(!CreateImage(I._bmih))
 	    return *this;

  	// Create extra memory if neccessary.
	  if(I._extraColorType != BMP_INFO_SPECIFIES)
		{
		  if(!CreateYUVMem(I._extraColorType))
  	    return *this;
    }//end if _extraColorType...
  }//end if !EqualMemFormat...

	// Copy palette and image data.
	Src = (LPSTR)(I._bmic);
	Dst = (LPSTR)(_bmic);
	Limit = (int)(I._paletteSize + I._bmih->biSizeImage);
	memcpy(Dst, Src, Limit);
	_paletteSize = I._paletteSize;

	// Deal with extra colour types.
	_extraColorType = I._extraColorType;
	// Copy YUV if neccessary.
	if(_extraColorType != BMP_INFO_SPECIFIES)
	{
		// YUV is store as contiguous mem.
		int size = ((_lumWidth * _lumHeight) + 2*(_chrWidth * _chrHeight)) * sizeof(int);
		memcpy((void *)_Y, (const void *)(I._Y), size);
	}//end if _extraColorType...

	return *this;
}//end operator=.

ImageHandlerV2& ImageHandlerV2::operator-=(ImageHandlerV2& I)
{
  // The images must be of the same type and size.
	if(!EqualMemFormat(I))
  {
   	_errorStr = "ImageHandlerV2: Images have different formats";
 	  return *this;
  }//end if !EqualMemFormat...

	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
    _errorStr = "ImageHandlerV2: Currently only implemented for YUV formats";
  	return *this;
	}//end if _extraColorType...

  // Determine the data sizes.
	int size = (_lumWidth * _lumHeight) + 2*(_chrWidth * _chrHeight);

  // Do the subtraction.
  int* Dst = _Y;
  int* Src = I._Y;
  for(int i = 0; i < size; i++)
  {
    *Dst = *Dst - *Src;
    Src++;
    Dst++;
  }//end for i...

	return *this;
}//end operator-=.

ImageHandlerV2& ImageHandlerV2::operator+=(ImageHandlerV2& I)
{
  // The images must be of the same type and size.
	if(!EqualMemFormat(I))
  {
   	_errorStr = "ImageHandlerV2: Images have different formats!";
 	  return *this;
  }//end if !EqualMemFormat...

	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
		_errorStr = "ImageHandlerV2: Currently only implemented for YUV formats";
    return *this;
	}//end if _extraColorType...

  // Determine the data sizes.
	int size = (_lumWidth * _lumHeight) + 2*(_chrWidth * _chrHeight);

  // Do the addition.
  int* Dst = _Y;
  int* Src = I._Y;
  for(int i = 0; i < size; i++)
  {
    *Dst = *Dst + *Src;
    Src++;
    Dst++;
  }//end for i...

	return *this;
}//end operator+=.

int ImageHandlerV2::Copy(ImageHandlerV2& fromImg)
{
	if(!EqualMemFormat(fromImg))
  {
   	_errorStr = "ImageHandlerV2: Images have different formats";
    return(0);
  }//end if !EqualMemFormat...

	// Copy image data.
	LPSTR S = (LPSTR)(fromImg._bmic);
	LPSTR D = (LPSTR)(_bmic);
	int Limit = (long int)(fromImg._bmih->biSizeImage);
	memcpy(D, S, Limit);

	if(_extraColorType != BMP_INFO_SPECIFIES)
	{
		// YUV is store as contiguous mem.
		int size = ((_lumWidth * _lumHeight) + 2*(_chrWidth * _chrHeight)) * sizeof(int);
		memcpy((void *)_Y, (const void *)(fromImg._Y), size);
	}//end if _extraColorType...

  return(1);
}//end Copy.

int ImageHandlerV2::EqualMemFormat(ImageHandlerV2& img)
{
  if( _bmih == NULL )
    return(0);

  if( (_extraColorType					== img._extraColorType) &&
      (_bmih->biSize						== img._bmih->biSize) &&
      (_bmih->biWidth						== img._bmih->biWidth) &&
      (_bmih->biHeight					== img._bmih->biHeight) &&
      (_bmih->biPlanes					== img._bmih->biPlanes) &&
      (_bmih->biBitCount				== img._bmih->biBitCount) &&
      (_bmih->biCompression			== img._bmih->biCompression) &&
      (_bmih->biSizeImage				== img._bmih->biSizeImage) &&
      (_bmih->biXPelsPerMeter		== img._bmih->biXPelsPerMeter) &&
      (_bmih->biYPelsPerMeter		== img._bmih->biYPelsPerMeter) &&
      (_bmih->biClrUsed					== img._bmih->biClrUsed) &&
      (_bmih->biClrImportant		== img._bmih->biClrImportant) &&
      ImageMemIsAlloc() )
    return(1);

  return(0);
}//end EqualMemFormat.

void ImageHandlerV2::Absolute(void)
{
	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = "ImageHandlerV2: Currently only implemented for YUV formats";
    return;
	}//end if _extraColorType...

  // Determine the data sizes.
	int size = (_lumWidth * _lumHeight) + 2*(_chrWidth * _chrHeight);

  // Calculate abs. on contiguous mem.
  int* Src = _Y;
  for(int i = 0; i < size; i++)
  {
    *Src = abs(*Src);
    Src++;
  }//end for i...
}//end Absolute.

double ImageHandlerV2::PSNR(ImageHandlerV2& noiseImg)
{
  double psnr;

	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = "ImageHandlerV2: Currently only implemented for YUV formats";
    return(0.0);
	}//end if _extraColorType...

	// Determine the total size of contiguous mem to operate on.
	int size = (_lumWidth * _lumHeight) + 2*(_chrWidth * _chrHeight);

  // Calculate the total absolute noise.
  double accnoise;
  accnoise	= 0.0000001;
  int* OSrc	= _Y;
  int* NSrc	= noiseImg._Y;
  for(int i = 0; i < size; i++)
  {
    int t = (*OSrc) - (*NSrc);
    accnoise += (double)(t * t);
    OSrc++;
    NSrc++;
  }//end for i...

  // Determine the peak signal to noise ratio.
  double noise = sqrt(accnoise / ((double)(size)));

  psnr = 20.0 * log10((double)255.0/noise);

  return(psnr);
}//end PSNR.

int ImageHandlerV2::IncreaseLum(int By)
{
	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = "ImageHandlerV2: Currently only implemented for YUV formats";
    return(0);
	}//end if _extraColorType...

  // Determine the data sizes.
	int ySize = _lumWidth * _lumHeight;
  for(int i = 0; i < ySize; i++)
    _Y[i] = _Y[i] + By;

  UpdateBmp();
  return(1);
}//end IncreaseLum.

int ImageHandlerV2::DecreaseLum(int By)
{
	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = "ImageHandlerV2: Currently only implemented for YUV formats";
    return(0);
	}//end if _extraColorType...

  //Determine the data sizes.
	int ySize = _lumWidth * _lumHeight;
  for(int i = 0; i < ySize; i++)
    _Y[i] = _Y[i] - By;

  UpdateBmp();
  return(1);
}//end DecreaseLum.

// Take the pixels to the right of PixPoint and write them
// to the left. Continue until an image boundary is reached.
void ImageHandlerV2::MirrorXLeft(int PixPoint)
{
	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = "ImageHandlerV2: Currently only implemented for YUV formats";
    return;
	}//end if _extraColorType...

  int i,j;
  int X,Y;
  // Do the Luminance.
  X = GetYWidth();
  Y = GetYHeight();
  int* P;
  for(i = 0; i < Y; i++)
  {
    P = (int *)(_Y + i*X + PixPoint);
    for(j = 0; (j <= PixPoint)&&((PixPoint+j)<X); j++)
      *(P - j) = *(P + j);
  }//end for i...

  // Do the Chrominance.
  X = GetUVWidth();
  Y = GetUVHeight();
  int* P_U;
  int* P_V;
  // NOTE: No allowance for odd width images.
  if(_extraColorType != YUV444)
    PixPoint = PixPoint/2;
  for(i = 0; i < Y; i++)
  {
    P_U = (int *)(_U + i*X + PixPoint);
    P_V = (int *)(_V + i*X + PixPoint);
    for(j = 0; (j <= PixPoint)&&((PixPoint+j)<X); j++)
    {
      *(P_U - j) = *(P_U + j);
      *(P_V - j) = *(P_V + j);
    }//end for j...
  }//end for i...

  UpdateBmp();
}//end MirrorXLeft.

// Take the pixels from the left of PixPoint and write them
// to the right. Continue until an image boundary is reached.
void ImageHandlerV2::MirrorXRight(int PixPoint)
{
	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = "ImageHandlerV2: Currently only implemented for YUV formats";
    return;
	}//end if _extraColorType...

  int i,j;
  int X,Y;
  // Do the Luminance.
  X = GetYWidth();
  Y = GetYHeight();
  int* P;
  for(i = 0; i < Y; i++)
  {
    P = (int *)(_Y + i*X + PixPoint);
    for(j = 0; (j <= PixPoint)&&((PixPoint+j)<X); j++)
      *(P + j) = *(P - j);
  }//end for i...

  // Do the Chrominance.
  X = GetUVWidth();
  Y = GetUVHeight();
  int* P_U;
  int* P_V;
  // NOTE: No allowance for odd width images.
  if(_extraColorType != YUV444)
    PixPoint = PixPoint/2;
  for(i = 0; i < Y; i++)
  {
    P_U = (int *)(_U + i*X + PixPoint);
    P_V = (int *)(_V + i*X + PixPoint);
    for(j = 0; (j <= PixPoint)&&((PixPoint+j)<X); j++)
    {
      *(P_U + j) = *(P_U - j);
      *(P_V + j) = *(P_V - j);
    }//end for j...
  }//end for i...

  UpdateBmp();
}//end MirrorXRight.

// Take the pixels from below PixPoint and write them
// above. Note that for bmp images, 'up' is the end of
// the lines. Continue until an image boundary is reached.
void ImageHandlerV2::MirrorYUp(int PixPoint)
{
	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = "ImageHandlerV2: Currently only implemented for YUV formats";
    return;
	}//end if _extraColorType...

  int i,j;
  int X,Y;
  //Do the Luminance.
  X = GetYWidth();
  Y = GetYHeight();
  int* P;
  for(j = 0; j < X; j++)
  {
    P = (int *)(_Y + PixPoint*X + j);
    for(i = 0; (i <= PixPoint)&&((PixPoint+i)<Y); i++)
      *(P + i*X) = *(P - i*X);
  }//end for j...

  //Do the Chrominance.
  X = GetUVWidth();
  Y = GetUVHeight();
  int* P_U;
  int* P_V;
  // NOTE: No allowance for odd width images.
  if(_extraColorType == YUV411)
    PixPoint = PixPoint/2;
  for(j = 0; j < X; j++)
  {
    P_U = (int *)(_U + PixPoint*X + j);
    P_V = (int *)(_V + PixPoint*X + j);
    for(i = 0; (i <= PixPoint)&&((PixPoint+i)<Y); i++)
    {
      *(P_U + i*X) = *(P_U - i*X);
      *(P_V + i*X) = *(P_V - i*X);
    }//end for i...
  }//end for j...

  UpdateBmp();
}//end MirrorYUp.

// Take the pixels from above PixPoint and write them
// below. Note that for bmp images, 'down' is the beginning of
// the lines. Continue until an image boundary is reached.
void ImageHandlerV2::MirrorYDown(int PixPoint)
{
	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = "ImageHandlerV2: Currently only implemented for YUV formats";
    return;
	}//end if _extraColorType...

  int i,j;
  int X,Y;
  // Do the Luminance.
  X = GetYWidth();
  Y = GetYHeight();
  int* P;
  for(j = 0; j < X; j++)
  {
    P = (int *)(_Y + PixPoint*X + j);
    for(i = 0; (i <= PixPoint)&&((PixPoint+i)<Y); i++)
      *(P - i*X) = *(P + i*X);
  }//end for j...

  // Do the Chrominance.
  X = GetUVWidth();
  Y = GetUVHeight();
  int* P_U;
  int* P_V;
  // NOTE: No allowance for odd width images.
  if(_extraColorType == YUV411)
    PixPoint = PixPoint/2;
  for(j = 0; j < X; j++)
  {
    P_U = (int *)(_U + PixPoint*X + j);
    P_V = (int *)(_V + PixPoint*X + j);
    for(i = 0; (i <= PixPoint)&&((PixPoint+i)<Y); i++)
    {
      *(P_U - i*X) = *(P_U + i*X);
      *(P_V - i*X) = *(P_V + i*X);
    }//end for i...
  }//end for j...

  UpdateBmp();
}//end MirrorYDown.

void ImageHandlerV2::KillColor(void)
{
	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = "ImageHandlerV2: Currently only implemented for YUV formats";
    return;
	}//end if _extraColorType...

  int i;
  int X;
  //Do the Chrominance only.
  X = _chrWidth * _chrHeight;
  for(i = 0; i < X; i++)
  {
    *(_U + i) = 0;
    *(_V + i) = 0;
  }//end for i...

  UpdateBmp();
}//end KillColor.

/*
---------------------------------------------------------------------------
	Colour conversion functions.
---------------------------------------------------------------------------
*/
int ImageHandlerV2::CreateYUVMem(int YUVType)
{
	// Check if mem needs to be removed first.
	if(_Y != NULL)
		delete[] _Y;
	_Y = NULL;
	_U = NULL;
	_V = NULL;
	_lumWidth		= 0;
	_lumHeight	= 0;
	_chrWidth		= 0;
	_chrHeight	= 0;

	_lumWidth		= _bmih->biWidth;
	_lumHeight	= abs(_bmih->biHeight);
	if(YUVType == YUV411)
	{
		_chrWidth		= _lumWidth/2;
		_chrHeight	= _lumHeight/2;
	}//end if YUV411...
	else if( YUVType == YUV422 )
	{
		_chrWidth		= _lumWidth/2;
		_chrHeight	= _lumHeight;
	}//end if YUV422...
	else //YUV444
	{
		_chrWidth		= _lumWidth;
		_chrHeight	= _lumHeight;
	}//end else...

	// Allocate contiguous memory for the extra image components.
	int size = (_lumWidth * _lumHeight) + 2*(_chrWidth * _chrHeight);
	_Y = new int[size];
	if(_Y == NULL)
		return(0);
	_U = &_Y[_lumWidth * _lumHeight];
	_V = &_Y[(_lumWidth * _lumHeight) + (_chrWidth * _chrHeight)];

	return(1);
}//end CreateYUVMem.

void ImageHandlerV2::DestroyYUVMem(void)
{
	if(_Y != NULL)
		delete[] _Y;
	_Y = NULL;
	_U = NULL;
	_V = NULL;
	_lumWidth		= 0;
	_lumHeight	= 0;
	_chrWidth		= 0;
	_chrHeight	= 0;

	_extraColorType = BMP_INFO_SPECIFIES;
}//end DestroyYUVMem.

void ImageHandlerV2::ConvertRGB24BITtoYUV411(void)
	{
	  // Y1, Y2, Y3 & Y4 have range 0..255,
	  // U & V have range -128..127.
		unsigned char *S;
		register int xb,yb,XB,YB;
		int xpix,y,u,v;
    int accu,accv;
		int r,g,b;

		xpix = _bmih->biWidth;
		S = (unsigned char *)_bmptr;
		//4 pix per Block.
		XB = _bmih->biWidth >> 1;
		YB = _bmih->biHeight >> 1;

		for(yb = 0; yb < YB; yb++)
  	 for(xb = 0; xb < XB; xb++)
		{
			//Top left pix.
			b = (int)( *(S + (yb*xpix + xb)*6) );
			g = (int)( *(S + (yb*xpix + xb)*6 + 1) );
			r = (int)( *(S + (yb*xpix + xb)*6 + 2) );
      RGBtoYUV(r,g,b,y,accu,accv);
			_Y[(yb*xpix + xb)*2] = (int)y;

			//Top right pix.
			b = (int)( *(S + (yb*xpix + xb)*6 + 3) );
			g = (int)( *(S + (yb*xpix + xb)*6 + 4) );
			r = (int)( *(S + (yb*xpix + xb)*6 + 5) );
      RGBtoYUV(r,g,b,y,u,v);
      accu += u;
      accv += v;
			_Y[(yb*xpix + xb)*2 + 1] = (int)y;

			//Bottom left pix.
			b = (int)( *(S + (yb*xpix + xb)*6 + xpix*3) );
			g = (int)( *(S + (yb*xpix + xb)*6 + xpix*3 + 1) );
			r = (int)( *(S + (yb*xpix + xb)*6 + xpix*3 + 2) );
      RGBtoYUV(r,g,b,y,u,v);
      accu += u;
      accv += v;
			_Y[(yb*xpix + xb)*2 + xpix] = (int)y;

			//Bottom right pix.
			b = (int)( *(S + (yb*xpix + xb)*6 + xpix*3 + 3) );
			g = (int)( *(S + (yb*xpix + xb)*6 + xpix*3 + 4) );
			r = (int)( *(S + (yb*xpix + xb)*6 + xpix*3 + 5) );
      RGBtoYUV(r,g,b,y,u,v);
      accu += u;
      accv += v;
			_Y[(yb*xpix + xb)*2 + xpix + 1] = (int)y;

			u = accu/4;
			v = accv/4;
			_U[yb*(xpix/2) + xb] = (int)u;
			_V[yb*(xpix/2) + xb] = (int)v;
 		}//end for xb & yb...

	}//end ConvertRGB24BITtoYUV411.

void ImageHandlerV2::ConvertYUV411toRGB24BIT(void)
{
	  // R, G & B have range 0..255,
		unsigned char *D;
		register int xb,yb,XB,YB;
		int xpix,r,g,b;
		int y,u,v;

		xpix = _bmih->biWidth;
		D = (unsigned char *)_bmptr;
		//4 pix per Block.
		XB = _bmih->biWidth >> 1;
		YB = _bmih->biHeight >> 1;

		for(yb = 0; yb < YB; yb++)
  	 for(xb = 0; xb < XB; xb++)
		{
			u = (int)_U[yb*(xpix/2) + xb];
			v = (int)_V[yb*(xpix/2) + xb];

			//Top left pix.
			y = (int)_Y[(yb*xpix + xb)*2];
      YUVtoRGB(y,u,v,r,g,b);
			*(D + (yb*xpix + xb)*6) = (unsigned char)b;
			*(D + (yb*xpix + xb)*6 + 1) = (unsigned char)g;
			*(D + (yb*xpix + xb)*6 + 2) = (unsigned char)r;

			//Top right pix.
			y = (int)_Y[(yb*xpix + xb)*2 + 1];
      YUVtoRGB(y,u,v,r,g,b);
			*(D + (yb*xpix + xb)*6 + 3) = (unsigned char)b;
			*(D + (yb*xpix + xb)*6 + 4) = (unsigned char)g;
			*(D + (yb*xpix + xb)*6 + 5) = (unsigned char)r;

			//Bottom left pix.
			y = (int)_Y[(yb*xpix + xb)*2 + xpix];
      YUVtoRGB(y,u,v,r,g,b);
			*(D + (yb*xpix + xb)*6 + xpix*3) = (unsigned char)b;
			*(D + (yb*xpix + xb)*6 + xpix*3 + 1) = (unsigned char)g;
			*(D + (yb*xpix + xb)*6 + xpix*3 + 2) = (unsigned char)r;

			//Bottom right pix.
			y = (int)_Y[(yb*xpix + xb)*2 + xpix + 1];
      YUVtoRGB(y,u,v,r,g,b);
			*(D + (yb*xpix + xb)*6 + xpix*3 + 3) = (unsigned char)b;
			*(D + (yb*xpix + xb)*6 + xpix*3 + 4) = (unsigned char)g;
			*(D + (yb*xpix + xb)*6 + xpix*3 + 5) = (unsigned char)r;
		}//end for xb & yb...

}//end ConvertYUV411toRGB24BIT.

void ImageHandlerV2::ConvertRGB24BITtoYUV422(void)
	{
	  // Y1 & Y2 have range 0..255,
	  // U & V have range -128..127.
		unsigned char *S;
		register int xb,yb,XB,YB;
		int xpix,y,u,v;
    int accu,accv;
		int r,g,b;

		xpix = _bmih->biWidth;
		S = (unsigned char *)_bmptr;
		//2 pix per Block.
		XB = _bmih->biWidth >> 1;
		YB = _bmih->biHeight;

		for(yb = 0; yb < YB; yb++)
  	 for(xb = 0; xb < XB; xb++)
		{
			//Left pix.
			b = (int)( *(S + (yb*xpix + xb*2)*3) );
			g = (int)( *(S + (yb*xpix + xb*2)*3 + 1) );
			r = (int)( *(S + (yb*xpix + xb*2)*3 + 2) );
      RGBtoYUV(r,g,b,y,accu,accv);
			_Y[yb*xpix + xb*2] = (int)y;

			//Right pix.
			b = (int)( *(S + (yb*xpix + xb*2)*3 + 3) );
			g = (int)( *(S + (yb*xpix + xb*2)*3 + 4) );
			r = (int)( *(S + (yb*xpix + xb*2)*3 + 5) );
      RGBtoYUV(r,g,b,y,u,v);
      accu += u;
      accv += v;
			_Y[yb*xpix + xb*2 + 1] = (int)y;

			u = accu/2;
			v = accv/2;
			_U[yb*(xpix/2) + xb] = (int)u;
			_V[yb*(xpix/2) + xb] = (int)v;
 		}//end for xb & yb...

	}//end ConvertRGB24BITtoYUV422.

void ImageHandlerV2::ConvertYUV422toRGB24BIT(void)
{
	// R, G & B have range 0..255,
	unsigned char *D;
	register int xb,yb,XB,YB;
	int xpix,r,b,g;
	int y,u,v;

	xpix = _bmih->biWidth;
	D = (unsigned char *)_bmptr;
	//2 pix per Block.
	XB = _bmih->biWidth >> 1;
	YB = _bmih->biHeight;

	for(yb = 0; yb < YB; yb++)
   for(xb = 0; xb < XB; xb++)
	{
		u = (int)_U[yb*(xpix/2) + xb];
		v = (int)_V[yb*(xpix/2) + xb];

		//Left pix.
		y = (int)_Y[yb*xpix + xb*2];
    YUVtoRGB(y,u,v,r,g,b);
		*(D + (yb*xpix + xb*2)*3) = (unsigned char)b;
		*(D + (yb*xpix + xb*2)*3 + 1) = (unsigned char)g;
		*(D + (yb*xpix + xb*2)*3 + 2) = (unsigned char)r;

		//Right pix.
		y = (int)_Y[yb*xpix + xb*2 + 1];
    YUVtoRGB(y,u,v,r,g,b);
		*(D + (yb*xpix + xb*2)*3 + 3) = (unsigned char)b;
		*(D + (yb*xpix + xb*2)*3 + 4) = (unsigned char)g;
		*(D + (yb*xpix + xb*2)*3 + 5) = (unsigned char)r;
	}//end for xb & yb...

}//end ConvertYUV422toRGB24BIT.

void ImageHandlerV2::ConvertRGB24BITtoYUV444(void)
{
  // Y has range 0..255,
  // U & V have range -128..127.
	unsigned char *S;
	register int xb,yb,XB,YB;
	int xpix;
	int r,g,b,y,u,v;

	xpix = _bmih->biWidth;
	S = (unsigned char *)_bmptr;
	//1 pix per Block.
	XB = _bmih->biWidth;
	YB = _bmih->biHeight;

	for(yb = 0; yb < YB; yb++)
	 for(xb = 0; xb < XB; xb++)
	{
		b = (int)( *(S + (yb*xpix + xb)*3) );
		g = (int)( *(S + (yb*xpix + xb)*3 + 1) );
		r = (int)( *(S + (yb*xpix + xb)*3 + 2) );

    RGBtoYUV(r,g,b,y,u,v);

		_Y[yb*xpix + xb] = (int)y;
		_U[yb*xpix + xb] = (int)u;
		_V[yb*xpix + xb] = (int)v;
	}//end for xb & yb...

}//end ConvertRGB24BITtoYUV444.

void ImageHandlerV2::ConvertYUV444toRGB24BIT(void)
{
	// R, G & B have range 0..255,
	unsigned char *D;
	register int xb,yb,XB,YB;
	int xpix;
	int r,g,b,y,u,v;

	xpix = _bmih->biWidth;
	D = (unsigned char *)_bmptr;
	//1 pix per Block.
	XB = _bmih->biWidth;
	YB = _bmih->biHeight;

	for(yb = 0; yb < YB; yb++)
   for(xb = 0; xb < XB; xb++)
	{
		u = (int)_U[yb*xpix + xb];
		v = (int)_V[yb*xpix + xb];
		y = (int)_Y[yb*xpix + xb];

    YUVtoRGB(y,u,v,r,g,b);

		*(D + (yb*xpix + xb)*3) = (unsigned char)b;
		*(D + (yb*xpix + xb)*3 + 1) = (unsigned char)g;
		*(D + (yb*xpix + xb)*3 + 2) = (unsigned char)r;
	}//end for xb & yb...

}//end ConvertYUV444toRGB24BIT.

// Matrix conversion including rounding and bounds
// checking.
void ImageHandlerV2::RGBtoYUV(double R,double G,double B,
														double &Y,double &U,double &V)
{
	Y = RtoY_00*R + RtoY_01*G + RtoY_02*B;
  if(Y < 0.0) 
    Y = 0.0;
  else if(Y > 255.0)
    Y = 255.0;
  else
    Y += 0.5;

	U = RtoY_10*R + RtoY_11*G + RtoY_12*B;
  if(U < 0.0) 
    U -= 0.5;
  else 
    U += 0.5;
  if(U > 127.0)
    U = 127.0;
  else if(U < -128.0)
    U = -128.0;

	V = RtoY_20*R + RtoY_21*G + RtoY_22*B;
  if(V < 0.0) 
    V -= 0.5;
  else 
    V += 0.5;
  if(V > 127.0)
    V = 127.0;
  else if(V < -128.0)
    V = -128.0;

}//end RGBtoYUV.

//void RGBtoYUV(int R,int G,int B,
//              int &Y,int &U,int &V)
//{
//  double r = (double)R;
//  double g = (double)G;
//  double b = (double)B;
//	Y = (int)(RtoY_00*r + RtoY_01*g + RtoY_02*b + 0.5);
//	double u = RtoY_10*r + RtoY_11*g + RtoY_12*b;
//	double v = RtoY_20*r + RtoY_21*g + RtoY_22*b;
//
//  if(Y < 0) 
//    Y = 0;
//  else if(Y > 255)
//    Y = 255;
//
//  if(u < 0.0) 
//    u -= 0.5;
//  else 
//    u += 0.5;
//  U = (int)u;
//  if(U > 127)
//    U = 127;
//  else if(U < -128)
//    U = -128;
//
//  if(v < 0.0) 
//    v -= 0.5;
//  else 
//    v += 0.5;
//  V = (int)v;
//  if(V > 127)
//    V = 127;
//  else if(V < -128)
//    V = -128;
//
//}//end RGBtoYUV.

void ImageHandlerV2::RGBtoYUV(int R,int G,int B,
														int &Y,int &U,int &V)
{
  double r = (double)R;
  double g = (double)G;
  double b = (double)B;
	Y = (int)(0.299*r + 0.587*g + 0.114*b + 0.5);
	double u = 0.436*b - 0.147*r - 0.289*g;
	double v = 0.615*r - 0.515*g - 0.100*b;

  if(Y < 0) 
    Y = 0;
  else if(Y > 255)
    Y = 255;

  if(u < 0.0) 
    u -= 0.5;
  else 
    u += 0.5;
  U = (int)u;
  if(U > 127)
    U = 127;
  else if(U < -128)
    U = -128;

  if(v < 0.0) 
    v -= 0.5;
  else 
    v += 0.5;
  V = (int)v;
  if(V > 127)
    V = 127;
  else if(V < -128)
    V = -128;

}//end RGBtoYUV.

//void YUVtoRGB(int Y,int U,int V,
//              int &R,int &G,int &B)
//{
//  double y = (double)Y;
//  double u = (double)U;
//  double v = (double)V;
//	R = (int)(y + YtoR_01*u + YtoR_02*v + 0.5);
//	G = (int)(y + YtoR_11*u + YtoR_12*v + 0.5);
//	B = (int)(y + YtoR_21*u + YtoR_22*v + 0.5);
//	if( R > 255 )
//	  R = 255;
//	else if( R < 0 )
//	  R = 0;
//	if( G > 255 )
//	  G = 255;
//	else if( G < 0 )
//	  G = 0;
//	if( B > 255 )
//	  B = 255;
//	else if( B < 0 )
//	  B = 0;
//}//end YUVtoRGB.

void ImageHandlerV2::YUVtoRGB(int Y,int U,int V,
														int &R,int &G,int &B)
{
  double y = (double)Y;
  double u = (double)U;
  double v = (double)V;
	R = (int)(y + 1.140*v + 0.5);
	G = (int)(y - 0.394*u - 0.581*v + 0.5);
	B = (int)(y + 2.032*u + 0.5);
	if( R > 255 )
	  R = 255;
	else if( R < 0 )
	  R = 0;
	if( G > 255 )
	  G = 255;
	else if( G < 0 )
	  G = 0;
	if( B > 255 )
	  B = 255;
	else if( B < 0 )
	  B = 0;
}//end YUVtoRGB.


