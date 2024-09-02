:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::  FILE        :LIN.BAT                                                             ::
::  DATE        :Thu, Jan 15, 2018                                                   ::
::  AUTHOR      :jarribas@bexencardio.com                                            ::
::  DESCRIPTION :Batch file that executes PC-Lint app lint-nt command, with          ::
::               PC-Lint configuration settings, available at GenPC-Lint_app dir.    ::
::               It also creates PC-Lint output .txt file                            ::
::               The arguments for this batch file:                                  ::
::               %1: The path to the project root folder                             ::
::  TARGET      :Windows CLI, call this batch file from the Eclipse project settings ::
::               through its Build Phase Lint                                        ::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@rem Path to the project folder
SET PROJ_PATH=%1

@rem Path to project source files folder
SET PROJ_SOURCE_PATH=%PROJ_PATH%\src

@rem Path to lint-nt.exe (NO SPACES!!!)
SET LINT_EXE=C:\PC-lint_9.0\lint-nt.exe

:: Compose the output file dir in txt format 
SET LINT_DOC="%PROJ_PATH%\doc\PC-Lint\R100_S3A7.lnt"

@rem Path to local lint folder inside the project with the lint files
SET PROJ_LINT_PATH=%PROJ_PATH%\lint

@rem Path to lint configuration files folder
SET LOCAL_LNT_FILES=%PROJ_LINT_PATH%\config

@rem Lint configuration files and includes
SET LNT_INCLUDES=-i"%PROJ_LINT_PATH%" -i%LOCAL_LNT_FILES%

@rem -------- Search .c files inside the project ------------
del "%PROJ_LINT_PATH%\Tem.txt"
del "%PROJ_LINT_PATH%\R100_S3A7_files.lnt"
for /R %PROJ_SOURCE_PATH% %%f in (*.c) do (
echo %%~f | findstr "\synergy_gen" >NUL: || echo %%~f >> %PROJ_LINT_PATH%\Tem.txt
)
copy "%PROJ_LINT_PATH%\Tem.txt" "%PROJ_LINT_PATH%\R100_S3A7_files.lnt"
del "%PROJ_LINT_PATH%\Tem.txt"

@rem --------------- Run PC-lint ---------------------------
%LINT_EXE% %LNT_INCLUDES% %PROJ_LINT_PATH%\eclipse_msg.lnt %PROJ_LINT_PATH%\R100_S3A7_options.lnt %PROJ_LINT_PATH%\R100_S3A7_files.lnt -vf

@rem --------------- Run PC-lint ---------------------------
%LINT_EXE% +v %LNT_INCLUDES% %PROJ_LINT_PATH%\R100_S3A7_options.lnt -os(%LINT_DOC%) %PROJ_LINT_PATH%\R100_S3A7_files.lnt

@rem ------- Execute PowerShell to replace file ------------
Powershell.exe -executionpolicy remotesigned -File %PROJ_LINT_PATH%\replace_R100_S3A7_files.ps1 %PROJ_PATH% %PROJ_LINT_PATH%
