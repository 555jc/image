@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=load bin.bmp bin
set image=target bin
set image=draw bin 100 100 alpha 50
set image=target cmd
set image=draw bin 0 0
pause