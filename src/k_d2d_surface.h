#pragma once

#include "k_graphics.h"

class KD2DSurface
{
public:
    KD2DSurface(HWND hwnd, D2D1_SIZE_U sz);
    ~KD2DSurface();
    void cdir();
    void cdr();
    void ddr();
    void resize(D2D1_SIZE_U sz);
    void render();
    void update();
    HRESULT d3d_create_device(D3D_DRIVER_TYPE const driver_type, ID3D11Device *&d3d_device);
    void bridge_swap_chain_and_device_context();
    
    D2D1_SIZE_U surface_size() const noexcept;
    ID2D1HwndRenderTarget *render_target() const noexcept;
    HWND hwnd() const;

protected:
    ID3D11Device *d3d_device_;
    ID2D1Factory2 *d2d_factory_;
    ID2D1DeviceContext *d2d_device_context_;
    ID2D1Bitmap1 *d2d_dxgi_bmp_;
    ID2D1SolidColorBrush *dxgi_surface_brush_;
    IDXGISwapChain1 *dxgi_swap_chain_;
    bool device_lost_{false};

    const unsigned kBitmapPixelWidth = 100;
    const unsigned kBitmapPixelHeight = 100;
    const unsigned kBytesPerPixel = 4;
    const uint32_t kBitmapPitch = kBitmapPixelWidth * kBytesPerPixel;
    const unsigned kMemSz = kBitmapPixelWidth * kBitmapPixelHeight;
    uint32_t *mem_{nullptr};
    unsigned x_ = 50;
    unsigned y_ = 10;
    unsigned dx_ = 1;
    unsigned dy_ = 1;
    const D2D1_RECT_U kBitmapDestRect = D2D1::RectU(0, 0, kBitmapPixelWidth, kBitmapPixelHeight);

    // 32-bit color layout: 0xffaabbcc, where ff = alpha, aa = red, bb = green, cc = blue
    void put_pixel(unsigned int x, unsigned int y, uint32_t color);
    void clear_bitmap_mem(uint32_t color);

    HWND hwnd_;
    D2D1_SIZE_U surface_size_; /* The size of the surface = size of window client area */
};
