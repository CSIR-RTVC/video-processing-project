#pragma once

// {5A70FE90-6D23-40BE-B94D-365DCC0436EA}
static const GUID CLSID_CSIR_VPP_RtspSinkFilter =
{ 0x5a70fe90, 0x6d23, 0x40be, { 0xb9, 0x4d, 0x36, 0x5d, 0xcc, 0x4, 0x36, 0xea } };

// TODO: LJ: copied from amr_types.h ... rather include it somewhere?
static const GUID MEDIASUBTYPE_AMR =
{ 0x726D6173, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

// Taken from Monogram
// {000000FF-0000-0010-8000-00AA00389B71}
static const GUID MEDIASUBTYPE_AAC =
{ 0x000000ff, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

// Taken from Monogram
// {000001FF-0000-0010-8000-00AA00389B71}
static const GUID MEDIASUBTYPE_LATM_AAC =
{ 0x000001ff, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

// Taken from Monogram
// {00001602-0000-0010-8000-00aa00389b71}
static const GUID MEDIASUBTYPE_LATM_AAC_2 =
{ 0x00001602, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

// Taken from Monogram
// {1365BE7A-C86A-473C-9A41-C0A6E82C9FA3}
static const GUID CLSID_MediaPlayerClassicDemux =
{ 0x1365BE7A, 0xC86A, 0x473C, { 0x9A, 0x41, 0xC0, 0xA6, 0xE8, 0x2C, 0x9F, 0xA3 } };

enum RTSP_MediaType
{
  MT_NOT_SET,
  MT_VIDEO,
  MT_AUDIO
};

enum RTSP_MediaSubtype
{
  MST_NOT_SET,
  MST_H264,
  MST_AMR,
  MST_AAC
};
