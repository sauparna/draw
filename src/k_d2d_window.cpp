#include "k_graphics.h"
#include "k_d2d_window.h"
#include "k_d2d_surface.h"

using namespace std;

KD2DWindow::KD2DWindow(D2D1_SIZE_U surface_sz) : KWindow{}
{
    create_window(L"KD2DWindow",
                  L"Direct2D surface",
                  surface_sz.width,
                  surface_sz.height);
    k_d2d_surface_ = std::make_unique<KD2DSurface>(hwnd_, surface_sz);
}

void KD2DWindow::on_resize(D2D1_SIZE_U surface_sz)
{
    if (!k_d2d_surface_) return;
    k_d2d_surface_->resize(surface_sz);
    k_d2d_surface_->render();
    InvalidateRect(hwnd_, NULL, FALSE);
}

void KD2DWindow::on_key_down(WPARAM wparam, LPARAM lparam)
{
    if (wparam == VK_ESCAPE) PostMessage(hwnd_, WM_DESTROY, 0, 0);
}

void KD2DWindow::on_draw(HDC hdc)
{
    if (!k_d2d_surface_) return;
    k_d2d_surface_->render();
    InvalidateRect(hwnd_, NULL, FALSE);
}

void KD2DWindow::on_quit()
{
    if (!k_d2d_surface_) return;
}

LRESULT KD2DWindow::window_procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
    {
        on_key_down(wparam, lparam);
        return 0;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hwnd_, &ps);
        on_draw(ps.hdc);
        EndPaint(hwnd_, &ps);
        return 0;
    }
    case WM_SIZE:
    {
        on_resize(D2D1::SizeU((UINT32)LOWORD(lparam), (UINT32)HIWORD(lparam)));
        return 0;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        on_quit();
        return 0;
    }
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);    
}
