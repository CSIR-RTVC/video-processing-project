# CMakeLists.txt in H264v2 dir

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_BUILD_FOR_SHORT")

message("CodecIncludes: ${vppCodecIncludes}")

INCLUDE_DIRECTORIES(
${vppIncludes}
${vppCodecIncludes}
${vppUtilIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(H264v2_LIB_HDRS
H264v2.h
../H264v2Codec/H264v2Codec.h
../H264v2Codec/H264v2CodecHeader.h
StdAfx.h
)

SET(H264v2_LIB_SRCS 
H264v2.cpp
../H264v2Codec/H264v2Codec.cpp
../H264v2Codec/H264v2CodecHeader.cpp
StdAfx.cpp
)

ADD_LIBRARY( H264v2 SHARED ${H264v2_LIB_SRCS} ${H264v2_LIB_HDRS})

TARGET_LINK_LIBRARIES(
H264v2
RtvcCodecUtils
vpp
) 
