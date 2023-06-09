#pragma once

class KWindow
{
public:
    KWindow(void);
    virtual ~KWindow(void);
    bool register_window_class(LPCTSTR class_name, HINSTANCE hinst);
    virtual bool create_window(LPCTSTR class_name,
                               LPCTSTR window_title,
                               int w = 600,
                               int h = 400,
                               int x = CW_USEDEFAULT,
                               int y = CW_USEDEFAULT,
                               DWORD style = WS_OVERLAPPEDWINDOW,
                               HWND parent = NULL,
                               HMENU hmenu = NULL);
    virtual WPARAM message_loop(void);
    BOOL show_window(int cmd_show) const;
    BOOL update_window(void) const;
    HWND hwnd() const;

protected:
    virtual void on_draw(HDC hdc);
    virtual void on_key_down(WPARAM wparam, LPARAM lparam);
    virtual LRESULT window_procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    static LRESULT CALLBACK Win32StaticWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    HWND hwnd_;
};
