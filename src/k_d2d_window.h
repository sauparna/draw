#include <memory>
#include "k_win32_window.h"
#include "k_d2d_surface.h"

class KD2DWindow : public KWindow
{
public:
    KD2DWindow(unsigned w, unsigned h);
    LRESULT window_procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
    void on_resize(unsigned w, unsigned h);
    void on_key_down(WPARAM wparam, LPARAM lparam);
    void on_draw(HDC hdc);
    
    std::unique_ptr<KD2DSurface> k_d2d_surface_;
};
