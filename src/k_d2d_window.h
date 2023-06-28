#pragma once

#include <memory>
#include "k_win32_window.h"
#include "k_d2d_surface.h"

class KD2DWindow : public KWindow
{
public:
    KD2DWindow(D2D1_SIZE_U surface_sz);
    LRESULT window_procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
    void on_resize(D2D1_SIZE_U surface_sz);
    void on_key_down(WPARAM wparam, LPARAM lparam);
    void on_draw(HDC hdc);
    void on_quit();
    
    std::unique_ptr<KD2DSurface> k_d2d_surface_;
};
