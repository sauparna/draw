#pragma once

#include "k_graphics.h"

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
    void render();
    void update();
    void write_bitmap_file(std::wstring filename);
    
    D2D1_SIZE_U surface_size() const noexcept;
    ID2D1HwndRenderTarget *render_target() const noexcept;
    HWND hwnd() const;

protected:
    ID2D1Factory *d2d_factory_;
    ID2D1HwndRenderTarget *win_rt_;

    /* Image file loader components. */
    IWICImagingFactory2 *wic_factory_;
    IWICFormatConverter *wic_converter_;
    IWICBitmap *wic_bitmap_;
    ID2D1Bitmap *d2d_bitmap_;
    std::wstring img_filename_{L"..\\data\\tintin_on_train.jpg"};
    unsigned kImageWidth = 0;
    unsigned kImageHeight = 0;

    /* In-memory bitmap (raster) components. */
    ID2D1Bitmap *d2d_bmp_;
    D2D1_BITMAP_PROPERTIES bmp_prop_;
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
    /* 32-bit color integer layout: 0xffaabbcc, where ff = alpha, aa = red, bb = green, cc = blue */
    void put_pixel(unsigned int x, unsigned int y, uint32_t color);
    void clear_bitmap_mem(uint32_t color);
    
    /* Text components. */
    IDWriteFactory *dwrite_factory_;
    IDWriteTextFormat *dwrite_text_format_;
    ID2D1SolidColorBrush *win_text_brush_;
    const D2D1_RECT_F kTextRect{0, 0, 50, 50};
    const wchar_t *kText = L"S";

    HWND hwnd_;
    D2D1_SIZE_U surface_size_; /* The size of the surface = size of window client area */
};
