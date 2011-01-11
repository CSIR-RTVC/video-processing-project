@call master.bat

@set SolutionFile="%SolutionDir%\RTVC.sln"
@echo -- SolutionFile set to %SolutionFile%

@rem 
@rem Start development tools
@rem Arguments:
@rem %1 = vs (visual studio)
@rem %2 = cmd (commandline)

@echo -- Exec Visual Studio
@call "%VisualStudioPath%\%VC_EXE%" /UseEnv %SolutionFile%

@echo -- Exec CMD
@call "%SystemRoot%\system32\cmd.exe"

@:END

