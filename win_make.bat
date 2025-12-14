cmake -S . -B build -DCMAKE_PREFIX_PATH="C:\Qt\6.10.1\mingw_64\lib\cmake" -G "MinGW Makefiles" -DCMAKE_C_COMPILER=C:/Qt/Tools/mingw1310_64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1310_64/bin/g++.exe
cmake --build build --config Release
# Install to a dist folder and run windeployqt
cmake --install build --config Release --prefix dist
