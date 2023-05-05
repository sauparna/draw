@echo off

mkdir ..\build
pushd ..\build
cl -Zi -D "_UNICODE" -D "UNICODE" ..\src\draw.cpp ..\src\kd2dsurface.cpp ..\src\kd2dwin.cpp ..\src\kwin.cpp -link user32.lib d2d1.lib dwrite.lib gdi32.lib
popd
