REM Set this to VC8, VC9 or VC10 depending on which VS version you want to use
@set VS_VERSION=VC10
REM On a 64 bit OS -> set the path to something like "C:\Program Files (x86)\Microsoft Visual Studio 9.0"

REM Set this to devenv.exe or VCExpress.exe
@set VC_EXE=VCExpress.exe

IF %VS_VERSION% EQU VC8 GOTO SETUP_VC8_ENV
IF %VS_VERSION% EQU VC9 GOTO SETUP_VC9_ENV
IF %VS_VERSION% EQU VC10 GOTO SETUP_VC10_ENV

:SETUP_VC8_ENV
@set VisualStudioRoot=C:\Program Files\Microsoft Visual Studio 8
@set VisualStudio=%VisualStudioRoot%\Common7\IDE\%VC_EXE%
@set VisualCDir=VC
@GOTO END

:SETUP_VC9_ENV
@set VisualStudioRoot=C:\Program Files\Microsoft Visual Studio 9.0
@set VisualStudio=%VisualStudioRoot%\Common7\IDE\%VC_EXE%
@set VisualCDir=VC
@GOTO END

:SETUP_VC10_ENV
@set VisualStudioRoot=D:\Program Files\Microsoft Visual Studio 10.0
@set VisualStudio=%VisualStudioRoot%\Common7\IDE\%VC_EXE%
@set VisualCDir=VC
@GOTO END

:END
@echo -- VisualStudioRoot set to %VisualStudioRoot%
@echo -- VisualStudio set to %VisualStudio%
@echo -- VS_SOL_EXT set to %VS_SOL_EXT%
