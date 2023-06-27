#include "k_graphics.h"
#include "k_win32_window.h"

KWindow::KWindow() { hwnd_ = NULL; }

KWindow::~KWindow() {}

HWND KWindow::hwnd() const { return hwnd_; }

void KWindow::on_draw(HDC hdc) {}

void KWindow::on_key_down(WPARAM wparam, LPARAM lparam) {}

LRESULT KWindow::window_procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
    {
        on_key_down(wparam, lparam);
        return 0;
    } break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hwnd_, &ps);
        on_draw(ps.hdc);
        EndPaint(hwnd_, &ps);
        return 0;
    } break;
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    } break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK KWindow::Win32StaticWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    KWindow *ptr_window;
    if (msg == WM_NCCREATE)
    {
        assert(!IsBadReadPtr((void*)lparam, sizeof(CREATESTRUCT)));
        MDICREATESTRUCT *ptr_mdic = (MDICREATESTRUCT*)((LPCREATESTRUCT)lparam)->lpCreateParams;
        ptr_window = (KWindow*)(ptr_mdic->lParam);
        assert(!IsBadReadPtr(ptr_window, sizeof(KWindow)));
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ptr_window);
    }
    else
    {
        ptr_window = (KWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    if (ptr_window) return ptr_window->window_procedure(hwnd, msg, wparam, lparam);
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

bool KWindow::register_window_class(LPCTSTR class_name, HINSTANCE hinst)
{
    WNDCLASSEX wc = {};
    if (!GetClassInfoEx(hinst, class_name, &wc))
    {
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = 0;
        wc.lpfnWndProc = Win32StaticWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = NULL;
        wc.hIcon = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = NULL;
        wc.hIconSm = NULL;
        wc.hInstance = hinst;
        wc.lpszClassName = class_name;
        if (!RegisterClassEx(&wc)) return false;
    }
    return true;
}

bool KWindow::create_window(DWORD ex_style,
                            LPCTSTR class_name,
                            LPCTSTR window_title,
                            DWORD style,
                            int x,
                            int y,
                            int w,
                            int h,
                            HWND parent,
                            HMENU hmenu)
{
    HINSTANCE hinst = GetModuleHandle(NULL);
    if (!register_window_class(class_name, hinst)) return false;
    MDICREATESTRUCT mdic = {};
    mdic.lParam = (LPARAM)this;
    RECT window_rect{0, 0, w, h};
    AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);
    hwnd_ = CreateWindowEx(ex_style,
                           class_name,
                           window_title,
                           style,
                           x, y,
                           window_rect.right - window_rect.left,
                           window_rect.bottom - window_rect.top,
                           parent,
                           hmenu,
                           hinst,
                           &mdic);
    return hwnd_ != NULL;
}

WPARAM KWindow::message_loop(void)
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }
    return msg.wParam;
}

BOOL KWindow::show_window(int cmd_show) const { return ::ShowWindow(hwnd_, cmd_show); }

BOOL KWindow::update_window(void) const { return ::UpdateWindow(hwnd_); }
