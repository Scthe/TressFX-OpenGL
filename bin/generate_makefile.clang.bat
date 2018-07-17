@SETLOCAL

@set COMPILE_FLAGS=-target x86_64-w64-mingw -stdlib=libstdc++
@set COMPILE_FLAGS= %COMPILE_FLAGS% -isystem \"C:\programs\install\MinGW64\x86_64\lib\gcc\x86_64-w64-mingw32\7.1.0\include\c++\x86_64-w64-mingw32\"
@set COMPILE_FLAGS= %COMPILE_FLAGS% -isystem \"C:\programs\install\MinGW64\x86_64\lib\gcc\x86_64-w64-mingw32\7.1.0\include\c++\"
@set COMPILE_FLAGS= %COMPILE_FLAGS% -isystem \"C:\programs\install\LLVM\lib\clang\4.0.1\include\"
@set COMPILE_FLAGS= %COMPILE_FLAGS% -isystem \"C:\programs\install\MinGW64\x86_64\x86_64-w64-mingw32\include\"
@set COMPILE_FLAGS= %COMPILE_FLAGS% -fansi-escape-codes -fcolor-diagnostics

@rem -DSDL2_ROOT_DIR="C:/programs/libraries/SDL2-2.0.4/x86_64-w64-mingw32" ^
@rem -DGLM_ROOT_DIR="C:/programs/libraries/glm-0.9.7.5" ^

C:\programs\portable\cmake-3.6.1-win64-x64\bin\cmake .^
 -G "MSYS Makefiles" ^
 -DCMAKE_CXX_COMPILER="Clang" ^
 -DCMAKE_RC_COMPILER="c:/programs/install/MinGW64/x86_64/bin/windres.exe" ^
 -DM_ADDITIONAL_LIBS="stdc++" ^
 -DCMAKE_CXX_FLAGS="%COMPILE_FLAGS%" ^
 -DCMAKE_BUILD_TYPE="Debug" ^
 -DGLM_ROOT_DIR="C:/programs/libraries/glm-0.9.9.0" ^
 -DSDL2_ROOT_DIR="C:/programs/libraries/SDL2-2.0.8/x86_64-w64-mingw32" ^
 -DGLAD_ROOT_DIR="C:/programs/libraries/GLAD"

@rem OLD FLAG:
@rem -DCMAKE_CXX_FLAGS="-target x86_64-w64-mingw -isystem \"C:\programs\install\MinGW64\x86_64\include\" -isystem \"C:\programs\install\MinGW64\x86_64\x86_64-w64-mingw32\include\" -isystem \"C:\programs\install\MinGW64\x86_64\x86_64-w64-mingw32\include\c++\" -isystem \"C:\programs\install\MinGW64\x86_64\x86_64-w64-mingw32\include\c++\x86_64-w64-mingw32\"" ^

@rem -target x86_64-w64-mingw
@rem -isystem \"C:\programs\install\MinGW64\x86_64\include\"
@rem -isystem \"C:\programs\install\MinGW64\x86_64\x86_64-w64-mingw32\include\"
@rem -isystem \"C:\programs\install\MinGW64\x86_64\x86_64-w64-mingw32\include\c++\"
@rem -isystem \"C:\programs\install\MinGW64\x86_64\x86_64-w64-mingw32\include\c++\x86_64-w64-mingw32\"


@rem MISC
@rem -DCMAKE_CXX_FLAGS="-i686-w64-mingw32" ^
@rem -DSDL2_ROOT_DIR="C:/programs/libraries/SDL2-2.0.4/x86_64-w64-mingw32" ^
@rem -DCMAKE_RC_COMPILER="c:/programs/install/MinGW64/bin/windres.exe" ^
@rem -DCMAKE_CXX_COMPILER="Clang" ^
@rem -DCMAKE_BUILD_TYPE="Debug" ^
