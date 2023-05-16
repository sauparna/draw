#include "k_graphics.h"
#include "k_d2d_surface.h"

using namespace std;
using namespace kdx;

KD2DSurface::KD2DSurface(HWND hwnd, D2D1_SIZE_U sz)
    : hwnd_{hwnd}, surface_size_{sz}
{
    bmp_prop_ = D2D1::BitmapProperties();
    bmp_prop_.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
					     D2D1_ALPHA_MODE_IGNORE);
    unsigned int mem_sz = kBitmapPixelWidth * kBitmapPixelHeight;
    mem_ = new uint32_t[mem_sz];
    memset(mem_, 0, mem_sz * kBytesPerPixel);
    clear_bitmap(0x00000000);
    put_pixel(x_, y_, 0xffffffff);
    cdir();
    cdir_bitmap(L"..\\data\\tintin_on_train.jpg");
    cddr();
    cwdr();
}

KD2DSurface::~KD2DSurface()
{
    delete [] mem_;
}

void KD2DSurface::cdir()
{
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory_);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&dwrite_factory_));
    dwrite_factory_->CreateTextFormat(L"Verdana",
				      NULL,
				      DWRITE_FONT_WEIGHT_NORMAL,
				      DWRITE_FONT_STYLE_NORMAL,
				      DWRITE_FONT_STRETCH_NORMAL,
				      72.0f, L"en-us",
				      &dwrite_text_format_);
    dwrite_text_format_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    dwrite_text_format_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    D2D1_STROKE_STYLE_PROPERTIES stroke_style_properties = D2D1::StrokeStyleProperties(D2D1_CAP_STYLE_FLAT,
										       D2D1_CAP_STYLE_FLAT,
										       D2D1_CAP_STYLE_FLAT,
										       D2D1_LINE_JOIN_MITER,
										       10.0f,
										       D2D1_DASH_STYLE_CUSTOM,
										       0.0f);

    /* A 'dash blank dash blank ...' pattern, where dash is a line of
     * length 5, and blank is a space of length 3. */
    float dashes[] = {5.0f, 3.0f};
    d2d_factory_->CreateStrokeStyle(stroke_style_properties, dashes, ARRAYSIZE(dashes), &win_stroke_style_);
}

void KD2DSurface::cddr()
{
    d2d_factory_->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
					 D2D1::HwndRenderTargetProperties(hwnd_, surface_size_),
					 &win_rt_);
    d2d_factory_->CreateWicBitmapRenderTarget(wic_bitmap_,
					      D2D1::RenderTargetProperties(),
					      &bmp_rt_);
    win_rt_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &win_brush_);
    win_rt_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &win_text_brush_);
    win_rt_->CreateBitmapFromWicBitmap(wic_converter_, nullptr, &d2d_bitmap_);
    bmp_rt_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &bmp_text_brush_);

    /* Create a D2D1Bitmap from a bitmap in memory */
    /*
    D2D1_BITMAP_PROPERTIES bmp_prop = D2D1::BitmapProperties();
    bmp_prop.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
					     D2D1_ALPHA_MODE_IGNORE);
    dx_assert(win_rt_->CreateBitmap(D2D1::SizeU(kBitmapPixelWidth, kBitmapPixelHeight),
				    mem_,
				    kBitmapPixelWidth * kBytesPerPixel,
				    bmp_prop,
				    &bmp_));
    */

    dx_assert(win_rt_->CreateBitmap(D2D1::SizeU(kBitmapPixelWidth, kBitmapPixelHeight),
				    mem_,
				    kBitmapPixelWidth * kBytesPerPixel,
				    bmp_prop_,
				    &bmp_));
}

void KD2DSurface::cwdr()
{
    win_rt_->Resize(surface_size_);
}

void KD2DSurface::ddr()
{
    dwrite_text_format_->Release();
    dwrite_factory_->Release();
    wic_converter_->Release();
    wic_factory_->Release();
    d2d_bitmap_->Release();
    bmp_->Release();
    win_rt_->Release();
    win_brush_->Release();
    win_text_brush_->Release();
    bmp_text_brush_->Release();
    win_stroke_style_->Release();
    d2d_factory_->Release();
}

void KD2DSurface::handle_device_loss()
{
}

void KD2DSurface::resize(D2D1_SIZE_U sz)
{
    if (surface_size_ == sz) return;
    surface_size_ = sz;
    cwdr();
}

void KD2DSurface::clear_surface()
{
    win_rt_->Clear(D2D1::ColorF(D2D1::ColorF::FloralWhite));
}

void KD2DSurface::render()
{
    win_rt_->BeginDraw();
    clear_surface();
    draw_text(L"S");
    draw_bitmap_to_window(50.0f, 10.0f);
    HRESULT hr = win_rt_->EndDraw();
    if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
    {
	// FINDOUT: Is this the right thing to do here?
	ddr();
    }
}

void KD2DSurface::draw_text(const wchar_t *text)
{
    static const D2D1_RECT_F kRect{0, 0, 50, 50};
    win_rt_->DrawText(text, wcslen(text), dwrite_text_format_, &kRect, win_text_brush_);
}

void KD2DSurface::draw_bitmap_to_window(FLOAT x = 0.0f, FLOAT y = 0.0f)
{
    UINT w, h;
    wic_converter_->GetSize(&w, &h);
    win_rt_->DrawBitmap(d2d_bitmap_, D2D1::RectF(x, y, x + (FLOAT)w, y + (FLOAT)h));

    /*************************************************************************************/
    /* update bitmap memory */

    put_pixel(x_, y_, 0x00000000);

    x_ += dx_;
    y_ += dy_;

    if (x_ == kBitmapPixelWidth - 1 || x_ == 0) { dx_ = -dx_; }
    if (y_ == kBitmapPixelHeight - 1 || y_ == 0) { dy_ = -dy_; }

    put_pixel(x_, y_, 0xffffffff);

    /*************************************************************************************/
    /* recreate bitmap */

    /* FINDOUT: Keep this block of code and tag a release with it so
     * as to be able to compare performance, but before that make sure
     * you have refactored the code and added all necessary error
     * handling pieces. */

    /*
    bmp_->Release();
    dx_assert(win_rt_->CreateBitmap(D2D1::SizeU(kBitmapPixelWidth, kBitmapPixelHeight),
				    mem_,
				    kBitmapPixelWidth * kBytesPerPixel,
				    bmp_prop_,
				    &bmp_));
    */
    
    static const D2D1_RECT_U dst_rect = D2D1::RectU(0, 0, kBitmapPixelWidth, kBitmapPixelHeight);
    static const uint32_t pitch = kBitmapPixelWidth * kBytesPerPixel;
    bmp_->CopyFromMemory(&dst_rect, mem_, pitch);
    /*************************************************************************************/
       
    win_rt_->DrawBitmap(bmp_, D2D1::RectF(x + 130, y, x + 130.0f + 100.0f, y + 100.0f));
}

D2D1_SIZE_U KD2DSurface::surface_size() const noexcept
{
    return surface_size_;
}

HWND KD2DSurface::hwnd() const
{
    return hwnd_;
}

void KD2DSurface::cdir_bitmap(std::wstring filename)
{
    dx_assert(CoCreateInstance(CLSID_WICImagingFactory2,
			       nullptr,
			       CLSCTX_INPROC_SERVER,
			       IID_PPV_ARGS(&wic_factory_)));
    IWICBitmapDecoder *decoder;
    dx_assert(wic_factory_->CreateDecoderFromFilename(filename.c_str(),
						      nullptr,
						      GENERIC_READ,
						      WICDecodeMetadataCacheOnDemand,
						      &decoder));
    IWICBitmapFrameDecode *frame;
    dx_assert(decoder->GetFrame(0, &frame));
    dx_assert(wic_factory_->CreateFormatConverter(&wic_converter_));
    dx_assert(wic_converter_->Initialize(frame,
					 GUID_WICPixelFormat32bppPBGRA,
					 WICBitmapDitherTypeNone,
					 nullptr,
					 0.0f,
					 WICBitmapPaletteTypeCustom));
    dx_assert(wic_factory_->CreateBitmap(100, 100,
					 GUID_WICPixelFormat32bppBGR,
					 WICBitmapCacheOnLoad,
					 &wic_bitmap_));
    /* dx_assert(wic_converter_->GetResolution(&dpix, &dpiy)); */
}

void KD2DSurface::write_bitmap_file(std::wstring filename)
{
    IWICBitmapEncoder *wic_encoder;
    dx_assert(wic_factory_->CreateEncoder(GUID_ContainerFormatPng, NULL, &wic_encoder));
    IWICStream *wic_stream;
    dx_assert(wic_factory_->CreateStream(&wic_stream));
    dx_assert(wic_stream->InitializeFromFilename(filename.c_str(), GENERIC_WRITE));
    dx_assert(wic_encoder->Initialize(wic_stream, WICBitmapEncoderNoCache));
    IWICBitmapFrameEncode *wic_frame;
    dx_assert(wic_encoder->CreateNewFrame(&wic_frame, NULL));
    dx_assert(wic_frame->Initialize(NULL));
    dx_assert(wic_frame->SetSize(100, 100));
    WICPixelFormatGUID pixel_format = GUID_WICPixelFormatDontCare;
    dx_assert(wic_frame->SetPixelFormat(&pixel_format));
    dx_assert(wic_frame->WriteSource(wic_bitmap_, NULL));
    dx_assert(wic_frame->Commit());
    dx_assert(wic_encoder->Commit());    
}

void KD2DSurface::draw_bitmap(const wchar_t *text)
{
    static const D2D1_RECT_F kRect{0, 0, 50, 50};
    bmp_rt_->BeginDraw();
    bmp_rt_->Clear(D2D1::ColorF(D2D1::ColorF::FloralWhite));
    bmp_rt_->DrawText(text, wcslen(text), dwrite_text_format_, &kRect, bmp_text_brush_);
    dx_assert(bmp_rt_->EndDraw());
}

void KD2DSurface::put_pixel(unsigned int x, unsigned int y, uint32_t color)
{
    mem_[y * kBitmapPixelWidth + x] = color;
}

void KD2DSurface::clear_bitmap(uint32_t color)
{
    for (int y = 0; y < kBitmapPixelHeight; y++)
    {
	for (int x = 0; x < kBitmapPixelWidth; x++)
	{
	    mem_[y * kBitmapPixelWidth + x] = color;
	}
    }
}
