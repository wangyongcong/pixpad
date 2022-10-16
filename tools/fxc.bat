@echo off
@REM read config from "./config" file
@REM config content:
@REM ----------------------------------------------------------------
@REM fxc=c:\fxc.exe  # path of fxc
@REM ----------------------------------------------------------------
for /f "tokens=1,2 delims==" %%i in (config) do (
    set %%i=%%j
)
set shader=..\assets\shader\%1.hlsl
set output=..\saved\shader
if NOT exist %output% mkdir %output%
set shader_bin=%output%\%1.cso
set shader_asm=%output%\%1.asm

%fxc% %shader% /T vs_5_0 /E "VS" /Fo %shader_bin% /Fc %shader_asm%
