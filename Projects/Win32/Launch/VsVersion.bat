REM Set this to VC8 or VC9 depending on which VS version you want to use
@set VS_VERSION=VC12

REM Set this to devenv.exe or VCExpress.exe, for >= VS2012 Windows Desktop Express editions use WDExpress.exe
REM @set VC_EXE=devenv.exe
REM @set VC_EXE=VCExpress.exe
@set VC_EXE=WDExpress.exe

@IF %VS_VERSION% EQU VC7 GOTO SETUP_VC7_FOLDER
@IF %VS_VERSION% EQU VC8 GOTO SETUP_VC8_FOLDER
@IF %VS_VERSION% EQU VC9 GOTO SETUP_VC9_FOLDER
@IF %VS_VERSION% EQU VC10 GOTO SETUP_VC10_FOLDER
@IF %VS_VERSION% EQU VC11 GOTO SETUP_VC11_FOLDER
@IF %VS_VERSION% EQU VC12 GOTO SETUP_VC12_FOLDER

:SETUP_VC7_FOLDER
@set VisualStudioFolder=Microsoft Visual Studio .NET 2003
@set VisualCDir=VC7
@GOTO SET_VC_PATH

:SETUP_VC8_FOLDER
@set VisualStudioFolder=Microsoft Visual Studio 8
@set VisualCDir=VC
@GOTO SET_VC_PATH

:SETUP_VC9_FOLDER
@set VisualStudioFolder=Microsoft Visual Studio 9.0
@set VisualCDir=VC
@GOTO SET_VC_PATH

:SETUP_VC10_FOLDER
@set VisualStudioFolder=Microsoft Visual Studio 10.0
@set VisualCDir=VC
@GOTO SET_VC_PATH

:SETUP_VC11_FOLDER
@set VisualStudioFolder=Microsoft Visual Studio 11.0
@set VisualCDir=VC
@GOTO SET_VC_PATH

:SETUP_VC12_FOLDER
@set VisualStudioFolder=Microsoft Visual Studio 12.0
@set VisualCDir=VC
@GOTO SET_VC_PATH

:SET_C_X86
  @set InstallRoot=C:\Program Files (x86)
  @echo -- Install root: %InstallRoot%
  @goto SET_VISUAL_STUDIO

:SET_C
  @set InstallRoot=C:\Program Files
  @echo -- Install root: %InstallRoot%
  @goto SET_VISUAL_STUDIO

:SET_D_X86
  @set InstallRoot=D:\Program Files (x86)
  @echo -- Install root: %InstallRoot%
  @goto SET_VISUAL_STUDIO

:SET_D
  @set InstallRoot=D:\Program Files
  @echo -- Install root: %InstallRoot%
  @goto SET_VISUAL_STUDIO

:SET_E_X86
  @set InstallRoot=E:\Program Files (x86)
  @echo -- Install root: %InstallRoot%
  @goto SET_VISUAL_STUDIO

:SET_E
  @set InstallRoot=E:\Program Files
  @echo -- Install root: %InstallRoot%
  @goto SET_VISUAL_STUDIO

:SET_VC_PATH

REM try the E drive first
@IF exist "E:\Program Files (x86)\%VisualStudioFolder%" GOTO SET_E_X86

@IF exist "E:\Program Files\%VisualStudioFolder%" GOTO SET_E

REM try the D drive first
@IF exist "D:\Program Files (x86)\%VisualStudioFolder%" GOTO SET_D_X86

@IF exist "D:\Program Files\%VisualStudioFolder%" GOTO SET_D

@IF exist "C:\Program Files (x86)\%VisualStudioFolder%" GOTO SET_C_X86
  
@IF exist "C:\Program Files\%VisualStudioFolder%" GOTO SET_C


:SET_VISUAL_STUDIO
  @set VisualStudioRoot=%InstallRoot%\%VisualStudioFolder%
  @set VisualStudio=%VisualStudioRoot%\Common7\IDE\%VC_EXE%
  @goto SET_TARGET

REM Set target to X86_64 here to target 64-bit
:SET_TARGET
  @set TARGET=X86
REM  @set TARGET=X86_64

  @IF %TARGET% EQU X86_64 GOTO SETUP_X86_64

REM Default: target native
  @GOTO SETUP_NATIVE


:SETUP_X86_64:
  @set VC_BIN=bin\x86_amd64
  @set VC_LIB=lib\amd64
  @set SDK_LIB=lib\x64
  @set TARGET_DIR=
  @goto END

:SETUP_NATIVE:
  @set VC_BIN=bin
  @set VC_LIB=lib
  @set SDK_LIB=lib

@goto END

:END
@echo -- VisualStudioRoot set to %VisualStudioRoot%
@echo -- VisualStudio set to %VisualStudio%
@echo -- VC_BIN set to %VC_BIN%
@echo -- VC_LIB set to %VC_LIB%
@echo -- SDK_LIB set to %SDK_LIB%
