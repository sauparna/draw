#include "k_graphics.h"
#include "k_d2d_window.h"
#include "k_d2d_surface.h"

using namespace std;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmd, int nShow)
{
    unique_ptr<KD2DWindow> win = make_unique<KD2DWindow>(320, 240);
    win->show_window(nShow);
    win->update_window();
    return win->message_loop();
}
