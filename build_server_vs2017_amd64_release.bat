pushd %CD%
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
popd

md build-server
pushd %CD%
cd build-server
cmake -G"Visual Studio 15 2017 Win64" -DENABLE_LUA_SCRIPTING=OFF -DENABLE_USB_XPRESSNET_INTERFACE=OFF ..\server\
cmake --build . --config Release
popd
pause