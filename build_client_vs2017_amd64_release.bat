pushd %CD%
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
call "%QT_PATH%\msvc2017_64\bin\qtenv2.bat"
popd

set QMAKE_SPEC=win32-msvc

md build-client
pushd %CD%
cd build-client

qmake ..\client\traintastic-client.pro -spec %QMAKE_SPEC% CONFIG+="release" DESTDIR=../build-client
if %ERRORLEVEL% neq 0 exit %ERRORLEVEL%

"%QT_PATH%\..\Tools\QtCreator\bin\jom.exe" -j %NUMBER_OF_PROCESSORS% -f Makefile
if %ERRORLEVEL% neq 0 exit %ERRORLEVEL%

"%QT_PATH%\msvc2017_64\bin\windeployqt.exe" --release --no-translations -no-system-d3d-compiler --no-opengl-sw traintastic-client.exe
if %ERRORLEVEL% neq 0 exit %ERRORLEVEL%

popd
pause