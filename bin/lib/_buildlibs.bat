@echo off
echo Please check that directory ..\bin exists and contains nasmw.exe.
echo Also source files must be placed in .\src directory. Strike any key now to continue...
pause
echo Building libs... 
..\bin\nasmw -f win32 src/floating.asm -o floating.obj
if errorlevel 1 goto error
..\bin\nasmw -f win32 src/iolayer.asm -o iolayer.obj
if errorlevel 1 goto error
echo Done
goto exit

:error

echo Execution of command breaks. Errorcode = %ERRORLEVEL%

:exit
