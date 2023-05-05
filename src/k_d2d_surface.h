#pragma once

#include <wrl/client.h>
#include <d2d1.h>
#include <dwrite.h>

class KD2DSurface
{
public:
    KD2DSurface(HWND hwnd, D2D1_SIZE_U sz);
    ~KD2DSurface();
    void cdir();
    void cddr();
    void cwdr();
    void ddr();
    void handle_device_loss();
    void resize(D2D1_SIZE_U sz);
    void clear_surface();
    void render();
    void draw_text(const wchar_t *text);
    
    D2D1_SIZE_U surface_size() const noexcept;
    Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> render_target() const noexcept;
    HWND hwnd() const;

protected:

    Microsoft::WRL::ComPtr<ID2D1Factory> d2d_factory_;
    Microsoft::WRL::ComPtr<IDWriteFactory> dwrite_factory_;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> dwrite_text_format_;
    Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> d2d_render_target_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> d2d_brush_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> d2d_text_brush_;
    Microsoft::WRL::ComPtr<ID2D1StrokeStyle> d2d_stroke_style_;

    HWND hwnd_;
    /* size of the D2D drawing rectangle (which must match the size of
     * the window's client area.) */
    D2D1_SIZE_U surface_size_; 
};
