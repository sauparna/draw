#include "k_graphics.h"
#include "k_d2d_surface.h"

using namespace std;
using namespace kdx;

KD2DSurface::KD2DSurface(HWND hwnd, D2D1_SIZE_U sz)
    : hwnd_{hwnd}, surface_size_{sz}
{
    mem_ = new uint32_t[kMemSz];
    clear_bitmap_mem(0x00000000);
    put_pixel(x_, y_, 0xffffffff);
    
    cdir();
    cdr();
}

KD2DSurface::~KD2DSurface()
{
    delete [] mem_;
}

void KD2DSurface::cdir()
{
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory_);
    assert(SUCCEEDED(hr));
}

void KD2DSurface::cdr()
{
    HRESULT hr = S_OK;

    // Create the Direct3D device.

    hr = d3d_create_device(D3D_DRIVER_TYPE_HARDWARE, d3d_device_);
    if (hr == DXGI_ERROR_UNSUPPORTED)
    {
        hr = d3d_create_device(D3D_DRIVER_TYPE_WARP, d3d_device_);
    }
    assert(SUCCEEDED(hr));

    // Create the swap chain.

    IDXGIDevice1 *dxgi_device;
    hr = d3d_device_->QueryInterface(__uuidof(IDXGIDevice1), reinterpret_cast<void**>(&dxgi_device));
    assert(SUCCEEDED(hr));

    IDXGIAdapter *dxgi_adapter;
    hr = dxgi_device->GetAdapter(&dxgi_adapter);
    assert(SUCCEEDED(hr));

    IDXGIFactory2 *dxgi_factory;
    hr = dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory));
    dxgi_adapter->Release();
    assert(SUCCEEDED(hr));

    DXGI_SWAP_CHAIN_DESC1 dxgi_swap_chain_desc = {0};
    dxgi_swap_chain_desc.Width = surface_size_.width;
    dxgi_swap_chain_desc.Height = surface_size_.height;
    dxgi_swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    dxgi_swap_chain_desc.Stereo = false;
    dxgi_swap_chain_desc.SampleDesc.Count = 1;
    dxgi_swap_chain_desc.SampleDesc.Quality = 0;
    dxgi_swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    dxgi_swap_chain_desc.BufferCount = 2;
    dxgi_swap_chain_desc.Scaling = DXGI_SCALING_NONE;
    dxgi_swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    dxgi_swap_chain_desc.Flags = 0;
    hr = dxgi_factory->CreateSwapChainForHwnd(d3d_device_,
                                              hwnd_,
                                              &dxgi_swap_chain_desc,
                                              nullptr,
                                              nullptr,
                                              &dxgi_swap_chain_);
    dxgi_factory->Release();
    assert(SUCCEEDED(hr));

    // Create the Direct2D device-context.

    ID2D1Device1 *d2d_device;
    hr = d2d_factory_->CreateDevice(dxgi_device, &d2d_device);
    dxgi_device->Release();
    assert(SUCCEEDED(hr));

    hr =  d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d_device_context_);
    d2d_device->Release();
    assert(SUCCEEDED(hr));

    hr = d2d_device_context_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &dxgi_surface_brush_);
    assert(SUCCEEDED(hr));

    device_lost_ = false;
}

void KD2DSurface::ddr()
{
    dxgi_surface_brush_->Release();
    d2d_dxgi_bmp_->Release();
    d2d_device_context_->Release();
    dxgi_swap_chain_->Release();
    d3d_device_->Release();
    device_lost_ = true;
}

HRESULT KD2DSurface::d3d_create_device(D3D_DRIVER_TYPE const driver_type, ID3D11Device *&d3d_device)
{
    UINT kD3DCreationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    kD3DCreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL kD3DFeatureLevels[7] =
	{
	    D3D_FEATURE_LEVEL_11_1,
	    D3D_FEATURE_LEVEL_11_0,
	    D3D_FEATURE_LEVEL_10_1,
	    D3D_FEATURE_LEVEL_10_0,
	    D3D_FEATURE_LEVEL_9_3,
	    D3D_FEATURE_LEVEL_9_2,
	    D3D_FEATURE_LEVEL_9_1
	};
    ID3D11DeviceContext *d3d_context;
    HRESULT hr = D3D11CreateDevice(nullptr,
                                   driver_type,
                                   0,
                                   kD3DCreationFlags,
                                   kD3DFeatureLevels,
                                   ARRAYSIZE(kD3DFeatureLevels),
                                   D3D11_SDK_VERSION,
                                   &d3d_device,
                                   NULL,
                                   &d3d_context);
    d3d_context->Release();
    return hr;
}

// NOTE: The function name can be confusing; fundamentally this is a
// "create render-targets" call.
void KD2DSurface::bridge_swap_chain_and_device_context()
{
    HRESULT hr = S_OK;

    IDXGISurface *dxgi_surface;
    hr = dxgi_swap_chain_->GetBuffer(0, IID_PPV_ARGS(&dxgi_surface));
    assert(SUCCEEDED(hr));
    D2D1_BITMAP_PROPERTIES1 d2d_dxgi_bmp_prop = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
        96.0f,
        96.0f);
    hr = d2d_device_context_->CreateBitmapFromDxgiSurface(dxgi_surface,
                                                          &d2d_dxgi_bmp_prop,
                                                          &d2d_dxgi_bmp_);
    dxgi_surface->Release();
    assert(SUCCEEDED(hr));

    d2d_device_context_->SetTarget(d2d_dxgi_bmp_);
}

void KD2DSurface::resize(D2D1_SIZE_U sz)
{
    // NOTE: It's not efficient to call ResizeBuffers() and have it
    // fail to determine that we call ddr(), because we know that if
    // we are ready to call ResizeBuffers() we already know that we
    // need to release some resources.
    // REWRITE: Call ddr() before calling ResizeBuffers().
    surface_size_ = sz;
    d2d_device_context_->SetTarget(nullptr);
    HRESULT hr = dxgi_swap_chain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    if (SUCCEEDED(hr))
    {
        bridge_swap_chain_and_device_context();
    }
    else
    {
        ddr();
    }
}

void KD2DSurface::render()
{
    HRESULT hr = S_OK;
    if (device_lost_)
    {
        cdr();
        bridge_swap_chain_and_device_context();
    }

    D2D1_RECT_U bmp_rectangle = D2D1::RectU(10, 10, 10 + 100, 10 + 100);
    // D2D1_RECT_F draw_rectangle = D2D1::RectF(120, 10, 120 + 100, 10 + 100);

    hr = d2d_dxgi_bmp_->CopyFromMemory(&bmp_rectangle, mem_, kBitmapPitch);
    assert(SUCCEEDED(hr));

    // d2d_device_context_->BeginDraw();
    // d2d_device_context_->DrawRectangle(&draw_rectangle, dxgi_surface_brush_);
    // hr = d2d_device_context_->EndDraw();
    // if (hr == D2DERR_RECREATE_TARGET)
    // {
    //     ddr();
    //     return;
    // }
    hr = dxgi_swap_chain_->Present(1, 0);
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        ddr();
        return;
    }
    assert(SUCCEEDED(hr));
}

D2D1_SIZE_U KD2DSurface::surface_size() const noexcept
{
    return surface_size_;
}

HWND KD2DSurface::hwnd() const
{
    return hwnd_;
}

void KD2DSurface::put_pixel(unsigned int x, unsigned int y, uint32_t color)
{
    mem_[y * kBitmapPixelWidth + x] = color;
}

void KD2DSurface::clear_bitmap_mem(uint32_t color)
{
    for (int y = 0; y < kBitmapPixelHeight; y++)
    {
        for (int x = 0; x < kBitmapPixelWidth; x++)
        {
            mem_[y * kBitmapPixelWidth + x] = color;
        }
    }
}

void KD2DSurface::update()
{
    put_pixel(x_, y_, 0x00000000); // Wipe the pixel.

    x_ += dx_;
    y_ += dy_;

    if (x_ == kBitmapPixelWidth - 1 || x_ == 0) { dx_ = -dx_; }
    if (y_ == kBitmapPixelHeight - 1 || y_ == 0) { dy_ = -dy_; }

    put_pixel(x_, y_, 0xffffffff); // Draw a new pixel.
}
