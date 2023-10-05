@echo off
if exist %~dp0\SiPhON.exe (
    %~dp0\SiPhON -n %~dp0\..\examples;%~dp0 %*
) else (
    opp_run -l %~dp0\SiPhON -n %~dp0\..\examples;%~dp0 %*
)
