#pragma once

#include "k_graphics.h"

class KD2DSurface
{
public:
    KD2DSurface(HWND hwnd, D2D1_SIZE_U sz);
    ~KD2DSurface();
    void cdir();
    void cddr();
    void ddr();
    void resize(D2D1_SIZE_U sz);
    void render();
    HRESULT d3d_create_device(D3D_DRIVER_TYPE const driver_type, ID3D11Device *&d3d_device);
    void bridge_swap_chain_and_device_context();
    
    D2D1_SIZE_U surface_size() const noexcept;
    ID2D1HwndRenderTarget *render_target() const noexcept;
    HWND hwnd() const;

protected:
    ID3D11Device *d3d_device_;
    ID2D1Factory2 *d2d_factory_;
    ID2D1DeviceContext *d2d_device_context_;
    ID2D1SolidColorBrush *dxgi_surface_brush_;
    IDXGISwapChain1 *dxgi_swap_chain_;

    HWND hwnd_;
    D2D1_SIZE_U surface_size_; /* The size of the surface = size of window client area */
};
