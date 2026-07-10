cd PinIn4Cpp
cmake -S . -B build -G "Ninja" -DBUILD_SHARED_LIBS=ON -D CMAKE_BUILD_TYPE=Release
cd build
ninja
cd ../../
pause
