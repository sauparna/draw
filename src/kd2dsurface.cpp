#include <windows.h>
#include "kd2dsurface.h"

using namespace std;

KD2DSurface::KD2DSurface(HWND hwnd, D2D1_SIZE_U sz) : hwnd_{hwnd}, surface_size_{sz}
{
    cdir();
    cddr();
    cwdr();
}

KD2DSurface::~KD2DSurface()
{
}

void KD2DSurface::cdir()
{
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2d_factory_.GetAddressOf());
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(dwrite_factory_.GetAddressOf()));
    dwrite_factory_->CreateTextFormat(L"Verdana", NULL,
				      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
				      72.0f, L"en-us", dwrite_text_format_.GetAddressOf());
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

    d2d_factory_->CreateStrokeStyle(stroke_style_properties, dashes, ARRAYSIZE(dashes), d2d_stroke_style_.GetAddressOf());
}

void KD2DSurface::cddr()
{
    d2d_factory_->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
					 D2D1::HwndRenderTargetProperties(hwnd_, surface_size_),
					 &d2d_render_target_);
    d2d_render_target_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), d2d_brush_.GetAddressOf());
    d2d_render_target_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), d2d_text_brush_.GetAddressOf());
}

void KD2DSurface::cwdr()
{
    d2d_render_target_->Resize(surface_size_);
}

void KD2DSurface::ddr()
{
    d2d_render_target_->Release();
    d2d_brush_->Release();
    d2d_text_brush_->Release();
    d2d_stroke_style_->Release();
    dwrite_text_format_->Release();
    dwrite_factory_->Release();
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
    d2d_render_target_->Clear(D2D1::ColorF(D2D1::ColorF::FloralWhite));
}

void KD2DSurface::render()
{
    d2d_render_target_->BeginDraw();
    clear_surface();
    draw_text(L"S");
    HRESULT hr = d2d_render_target_->EndDraw();
    if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
    {
	// FINDOUT: Is this the right thing to do here?
	ddr();
    }
}

void KD2DSurface::draw_text(const wchar_t *text)
{
    static const D2D1_RECT_F kRect{0, 0, 50, 50};
    d2d_render_target_->DrawText(text, wcslen(text), dwrite_text_format_.Get(), &kRect, d2d_text_brush_.Get());
}

D2D1_SIZE_U KD2DSurface::surface_size() const noexcept
{
    return surface_size_;
}

HWND KD2DSurface::hwnd() const
{
    return hwnd_;
}
