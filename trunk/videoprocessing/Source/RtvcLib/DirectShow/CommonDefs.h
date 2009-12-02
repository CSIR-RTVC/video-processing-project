/** @file

MODULE				: CommonDefs

FILE NAME			: CommonDefs.h

DESCRIPTION			: 
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008, CSIR
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
#pragma once

#include <vector>

const int BYTES_PER_PIXEL_RGB24 = 3;
const int BYTES_PER_PIXEL_RGB32 = 4;
const double BYTES_PER_PIXEL_YUV420P = 1.5;

#define IN_COLOUR_RGB24		"0"
#define IN_COLOUR_YUV420P	"16"

#define IMAGE_WIDTH			"width"
#define IMAGE_HEIGHT		"height"

// Scale filter
#define TARGET_WIDTH		"targetwidth"
#define TARGET_HEIGHT		"targetheight"

// Crop filter
#define TOP_CROP			"topcrop"
#define BOTTOM_CROP			"bottomcrop"
#define LEFT_CROP			"leftcrop"
#define RIGHT_CROP			"rightcrop"

#define ORIENTATION			"orientation"

#define ROTATION_MODE		"rotationmode"

#define SUB_PICTURE_POSITION	"position"
#define SUB_PIC_WIDTH			"subpicwidth"
#define SUB_PIC_HEIGHT			"subpicheight"
#define OFFSET_X				"offsetx"
#define OFFSET_Y				"offsety"
