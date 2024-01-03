@echo off

set SRC_DIR="sources"
set INC_DIR="headers"
set BIN_DIR="bin"

set CC="D:\Applications\mingw64\bin\x86_64-w64-mingw32-gcc.exe"

if not exist %BIN_DIR% mkdir %BIN_DIR%

for %%f in (%SRC_DIR%/*.c) do (
    %CC% -o %BIN_DIR%/%%~nf.exe -I %INC_DIR% %SRC_DIR%/%%f -lgdi32
)