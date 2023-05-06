#include "k_graphics.h"
#include "k_d2d_window.h"
#include "k_d2d_surface.h"

using namespace std;

KD2DWindow::KD2DWindow(unsigned w, unsigned h) : KWindow{}
{
    create_window(0, L"KD2DWindow", L"Direct2D surface", WS_OVERLAPPEDWINDOW,
		  CW_USEDEFAULT, CW_USEDEFAULT, w, h, NULL, NULL);
    k_d2d_surface_ = std::make_unique<KD2DSurface>(hwnd_, D2D1_SIZE_U{w, h});
}

void KD2DWindow::on_resize(unsigned w, unsigned h)
{
    if (!k_d2d_surface_) return;
    k_d2d_surface_->resize(D2D1_SIZE_U{w, h});
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
	on_resize((UINT)LOWORD(lparam), (UINT)HIWORD(lparam));
	return 0;
    }
    case WM_DESTROY:
    {
	PostQuitMessage(0);
	return 0;
    }
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);    
}
