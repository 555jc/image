@echo off
%1start /b "" "%~dp0cmd.exe" "/c %~fs0 :"&exit
setlocal EnableDelayedExpansion
set image=help
set image=resize cmd 1000 1000
set image=load bin.jpg bin
set image=rotate bin 45
set image=save bin2.bmp bin
set image=draw bin2.bmp 0 0
set image=font 50 100 0 255 255
set image=text "hello world" 200 500
pause