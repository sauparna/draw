@echo off

mkdir ..\build
pushd ..\build
cl -Zi ..\src\draw.cpp -link user32.lib d2d1.lib dwrite.lib gdi32.lib ole32.lib dxgi.lib d3d11.lib d3dcompiler.lib
popd
