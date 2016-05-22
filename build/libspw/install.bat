set INSTALL_DIR=%1
set TARGET_FILE=%2
set TARGET_PATH=%INSTALL_DIR%\%TARGET_FILE%
if not exist %INSTALL_DIR% (mkdir %INSTALL_DIR%)
if not exist %TARGET_PATH% (goto COPY_LIB) else (goto UPDATE_LIB)
:COPY_LIB
copy /Y $(TargetPath) %TARGET_PATH%
goto COPY_LIB_END
:UPDATE_LIB
xcopy /Y /D $(TargetPath) %TARGET_PATH%
goto COPY_LIB_END
:COPY_LIB_END

