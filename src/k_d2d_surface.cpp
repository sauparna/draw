#include "k_graphics.h"
#include "k_d2d_surface.h"

using namespace std;
using namespace kdx;

KD2DSurface::KD2DSurface(HWND hwnd, D2D1_SIZE_U sz)
    : hwnd_{hwnd}, surface_size_{sz}
{
    cdir();
    cddr();
}

KD2DSurface::~KD2DSurface()
{
}

void KD2DSurface::cdir()
{
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory_);
    assert(SUCCEEDED(hr));
}

void KD2DSurface::cddr()
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
    dxgi_device->Release();
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

    // Create the Direct2D render target.

    D2D1_RENDER_TARGET_PROPERTIES d2d_rt_properties = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
        96.0,
        96.0);
    IDXGISurface *dxgi_surface;
    hr = dxgi_swap_chain_->GetBuffer(0, IID_PPV_ARGS(&dxgi_surface));
    assert(SUCCEEDED(hr));
    hr = d2d_factory_->CreateDxgiSurfaceRenderTarget(
        dxgi_surface,
        &d2d_rt_properties,
        &d2d_rt_);
    dxgi_surface->Release();
    assert(SUCCEEDED(hr));

    hr = d2d_rt_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &dxgi_surface_brush_);
    assert(SUCCEEDED(hr));

    device_lost_ = false;
}

void KD2DSurface::ddr()
{
    dxgi_surface_brush_->Release();
    d2d_rt_->Release();
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

void KD2DSurface::resize(D2D1_SIZE_U sz)
{
    surface_size_ = sz;
    HRESULT hr = dxgi_swap_chain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr))
        ddr();
}

void KD2DSurface::render()
{
    HRESULT hr = S_OK;
    if (device_lost_)
    {
        cddr();
    }

    d2d_rt_->BeginDraw();
    D2D1_RECT_F draw_rectangle = D2D1::RectF(120, 10, 120 + 100, 10 + 100);
    d2d_rt_->DrawRectangle(&draw_rectangle, dxgi_surface_brush_);
    hr = d2d_rt_->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
    {
        ddr();
        return;
    }
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

void KD2DSurface::put_pixel(unsigned int x, unsigned int y, uint32_t color, BYTE* mem)
{
    mem[y * surface_size_.width + x] = color;
}

void KD2DSurface::clear_framebuffer(uint32_t color, BYTE* mem)
{
    for (int y = 0; y < surface_size_.height; y++)
    {
        for (int x = 0; x < surface_size_.width; x++)
        {
            mem[y * surface_size_.width + x] = color;
        }
    }
}

void KD2DSurface::update(BYTE *mem)
{
    put_pixel(x_, y_, 0x00000000, mem); // Wipe the pixel.

    x_ += dx_;
    y_ += dy_;

    if (x_ == surface_size_.width - 1 || x_ == 0) { dx_ = -dx_; }
    if (y_ == surface_size_.height - 1 || y_ == 0) { dy_ = -dy_; }

    put_pixel(x_, y_, 0xffffffff, mem); // Draw a new pixel.
}
