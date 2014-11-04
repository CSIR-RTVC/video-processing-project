# CMakelists.txt in Codecs dir

# Adds sub directories
ADD_SUBDIRECTORY( CodecUtils )
ADD_SUBDIRECTORY( H264v2 )
IF (OPUS_SUPPORT)
ADD_SUBDIRECTORY( Opus )
ENDIF (OPUS_SUPPORT)