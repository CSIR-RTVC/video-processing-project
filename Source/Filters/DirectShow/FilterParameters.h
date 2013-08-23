#pragma once

#include <string>

//const int BYTES_PER_PIXEL_RGB24       = 3;
//const int BYTES_PER_PIXEL_RGB32       = 4;
//const double BYTES_PER_PIXEL_YUV420P  = 1.5;
#define NOTIFYONIFRAME          "notifyoniframe"

#define IN_COLOUR_RGB24         "0"
#define IN_COLOUR_YUV420P       "16"
#define IN_COLOUR_YUV420P8      "17"
#define IMAGE_WIDTH             "width"
#define IMAGE_HEIGHT            "height"
#define QUALITY                 "quality"
#define IN_COLOUR               "incolour"
#define OUT_COLOUR              "outcolour"
#define TARGET_WIDTH            "targetwidth"
#define TARGET_HEIGHT           "targetheight"
#define TARGET_FILE				      "targetfile"
#define TOP_CROP                "top"
#define BOTTOM_CROP             "bottom"
#define LEFT_CROP               "left"
#define RIGHT_CROP              "right"
#define X_FRAMES_PER_SECOND     "xframespersecond"
#define FRAMES_PER_X_SECONDS    "framesperxseconds"
#define CONCAT_ORIENTATION      "concatorientation"
#define MODE_OF_OPERATION       "modeofoperation"
#define MODE_OF_OPERATION_H264  "mode of operation"
#define FRAME_BIT_LIMIT         "framebitlimit"	
#define STREAM_USING_TCP        "streamusingtcp"
#define	I_PICTURE_MULTIPLIER    "ipicturemultiplier"
#define	AUTO_IFRAME_DETECT_FLAG "autoiframedetectflag"
#define IFRAME_PERIOD           "iframeperiod"
#define H264_TYPE               "h264type"
#define PREFIX_START_CODE       "prefix start codes"
#define PREPEND_PARAMETER_SETS  "prepend parameter sets"
#define SPS                     "sps"
#define PPS                     "pps"
#define MAX_QP                  51

#define ORIENTATION			        "orientation"
#define ROTATION_MODE		        "rotationmode"
#define SUB_PICTURE_POSITION	  "position"
#define SUB_PIC_WIDTH			      "subpicwidth"
#define SUB_PIC_HEIGHT			    "subpicheight"
#define OFFSET_X				        "offsetx"
#define OFFSET_Y				        "offsety"

#define H264_VPP 0
#define H264_H264 1
#define H264_AVC1 2