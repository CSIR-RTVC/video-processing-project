REM Set this to VC8 or VC9 depending on which VS version you want to use
@set VS_VERSION=VC10

@IF %VS_VERSION% EQU VC7 GOTO SETUP_VC7_FOLDER
@IF %VS_VERSION% EQU VC8 GOTO SETUP_VC8_FOLDER
@IF %VS_VERSION% EQU VC9 GOTO SETUP_VC9_FOLDER
@IF %VS_VERSION% EQU VC10 GOTO SETUP_VC10_FOLDER
@IF %VS_VERSION% EQU VC11 GOTO SETUP_VC11_FOLDER

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

:SET_VC_PATH
@IF exist "C:\Program Files (x86)\%VisualStudioFolder%" GOTO SET_C_X86
  
@IF exist "C:\Program Files\%VisualStudioFolder%" GOTO SET_C

@IF exist "D:\Program Files (x86)\%VisualStudioFolder%" GOTO SET_D_X86

@IF exist "D:\Program Files\%VisualStudioFolder%" GOTO SET_D

:SET_VISUAL_STUDIO
  @set VisualStudioRoot=%InstallRoot%\%VisualStudioFolder%
  @set VisualStudio=%VisualStudioRoot%\Common7\IDE\devenv.exe
  @goto END

:END
@echo -- VisualStudioRoot set to %VisualStudioRoot%
@echo -- VisualStudio set to %VisualStudio%
