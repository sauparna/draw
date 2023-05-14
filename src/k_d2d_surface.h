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
    void clear_surface();
    void render();
    void draw_text(const wchar_t *text);
    void draw_bitmap_to_window(FLOAT x, FLOAT y);
    void draw_bitmap(const wchar_t *text);
    void cdir_bitmap(std::wstring filename);
    void write_bitmap_file(std::wstring filename);
    
    D2D1_SIZE_U surface_size() const noexcept;
    ID2D1HwndRenderTarget *render_target() const noexcept;
    HWND hwnd() const;

protected:
    ID2D1Factory *d2d_factory_;
    IDWriteFactory *dwrite_factory_;
    IDWriteTextFormat *dwrite_text_format_;
    ID2D1HwndRenderTarget *win_rt_;
    ID2D1RenderTarget *bmp_rt_;
    ID2D1SolidColorBrush *win_brush_;
    ID2D1SolidColorBrush *win_text_brush_;
    ID2D1SolidColorBrush *bmp_text_brush_;
    ID2D1StrokeStyle *win_stroke_style_;

    IWICImagingFactory2 *wic_factory_;
    IWICFormatConverter *wic_converter_;
    ID2D1Bitmap *d2d_bitmap_;
    IWICBitmap *wic_bitmap_;

    ID2D1Bitmap *bmp_;
    uint32_t *mem_;

    D2D1_BITMAP_PROPERTIES bmp_prop_;
    const unsigned int kBitmapPixelWidth = 100;
    const unsigned int kBitmapPixelHeight = 100;
    const unsigned int kBytesPerPixel = 4;
    unsigned int x_ = 50;
    unsigned int y_ = 10;
    unsigned int dx_ = 1;
    unsigned int dy_ = 1;

    /* 32-bit color integer layout: 0xffaabbcc, where ff = alpha, aa = red, bb = green, cc = blue */
    void put_pixel(unsigned int x, unsigned int y, uint32_t color);
    void clear_bitmap(uint32_t color);
    
    HWND hwnd_;

    /* size of the D2D drawing rectangle (which must match the size of
     * the window's client area.) */
    D2D1_SIZE_U surface_size_; 
};

