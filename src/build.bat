@echo off

mkdir ..\build
pushd ..\build
cl -Zi ..\src\draw.cpp ..\src\k_d2d_surface.cpp ..\src\k_d2d_window.cpp ..\src\k_win32_window.cpp -link user32.lib d2d1.lib dwrite.lib gdi32.lib ole32.lib dxgi.lib
popd
