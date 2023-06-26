#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <cassert>

static bool gWindowResized = false;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;
    switch (msg)
    {
    case WM_KEYDOWN:
    {
        if (wparam == VK_ESCAPE)
            DestroyWindow(hwnd);
        break;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }
    case WM_SIZE:
    {
        gWindowResized = true;
        break;
    }
    default:
        result = DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    return result;
}

int WINPAI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // Open a window.
    HWND hwnd;
    {
        WNDCLASSEXW winClass = {};
        winClass.cbSize = sizeof(WNDCLASSEXW);
        winClass.style = CS_HREDRAW | CS_VREDRAW;
        winClass.lpfnWndProc = &WndProc;
        winClass.hInstance = hInstance;
        winClass.hIcon = LoadIconW(0, IDI_APPLICATION);
        winClass.hCursor = LoadCursorW(0, IDC_ARROW);
        winClass.lpszClassName = L"MyWindowClass";
        winClass.hIconSm = LoadIconW(0, IDI_APPLICATION);

        if(!RegisterClassExW(&winClass)) {
            MessageBoxA(0, "RegisterClassEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }

        RECT initialRect = { 0, 0, 1024, 768 };
        AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
        LONG initialWidth = initialRect.right - initialRect.left;
        LONG initialHeight = initialRect.bottom - initialRect.top;

        hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
                               winClass.lpszClassName,
                               L"Direct3D",
                               WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               initialWidth, 
                               initialHeight,
                               0, 0, hInstance, 0);

        if(!hwnd) {
            MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }
    }

    // Create D3D11 Device and Context.
    ID3D11Device1 *d3d11Device;
    ID3D11DeviceContext1 *d3d11DeviceContext;
    {
        ID3D11Device *baseDevice;
        ID3D11DeviceContext *baseDeviceContext;
        D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0};
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG_BUILD)
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        HRESULT hr = D3D11CreateDevice(0,
                                       D3D_DRIVER_TYPE_HARDWARE,
                                       0,
                                       creationFlags,
                                       featureLevels,
                                       ARRAYSIZE(featureLevels),
                                       D3D11_SDK_VERSION,
                                       &baseDevice,
                                       0,
                                       &baseDeviceContext);

        if (FAILED(hr))
        {
            MessageBoxA(0, "D3D11CreateDevice() failed", "Fatal Error", MB_OK);
            return GetLastError();            
        }

        // Retrieve the 1.1 interfaces.
        hr = baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&d3d11Device);
        assert(SUCCEEDED(hr));
        baseDeviceContext->Release();

        hr = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&d3d11DeviceContext);
        assert(SUCCEEDED(hr));
        baseDeviceContext->Release();
    }

#ifdef DEBUG_BUILD
    // Set up debug layer to break on D3D11 errors.
    ID3D11Debug *d3dDebug = nullptr;
    d3d11Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
    if (d3dDebug)
    {
        ID3D11InfoQueue *d3dInfoQueue = nullptr;
        if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
        {
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
            d3dInfoQueue->Release();
        }
        d3dDebug->Release();
    }
#endif

    // Create swap chain.
    IDXGISwapChain1 *d3d11SwapChain;
    {
        IDXGIFactory2 *dxgiFactory;
        {
            IDXGIDevice1 *dxgiDevice;
            HRESULT hr = d3d11Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);
            assert(SUCCEEDED(hr));
            IDXGIAdapter *dxgiAdapter;
            hr = dxgiDevice->GetAdapter(&dxgiAdapter);
            assert(SUCCEEDED(hr));
            dxgiDevice->Release();
            DXGI_ADAPTER_DESC adapterDesc;
            dxgiAdapter->GetDesc(&adapterDesc);
            OutputDebugStrinA("Graphics Device: ");
            OutputDebugStringW(adapterDesc.Description);
            hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);
            assert(SUCCEEDED(hr));
            dxgiAdapter->Release();
        }

        DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc = {};
        dxgiSwapChainDesc.Width = 0;
        dxgiSwapChainDesc.Height = 0;
        dxgiSwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        dxgiSwapChainDesc.SampleDesc.Count = 1;
        dxgiSwapChainDesc.SampleDesc.Quality = 0;
        dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        dxgiSwapChainDesc.BufferCount = 2;
        dxgiSwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        dxgiSwapChainDesc.Flags = 0;

        HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(d3d11Device, hwnd, &d3d11SwapChainDesc, 0, 0, &d3d11SwapChain);
        assert(SUCCEEDED(hr));
        dxgiFactory->Release();
    }

    // Create render target.
    ID3D11RenderTargetView *d3d11FrameBufferView;
    {
        ID3D11Texture2D *d3d11FrameBuffer;
        HRESULT hr = d3d11SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer);
        assert(SUCCESS(hr));
        hr = d3d11Device->CreateRenderTargetView(d3d11FrameBuffer, 0, &d3d11FrameBufferView);
        assert(SUCCESS(hr));
        d3d11FrameBuffer->Release();
    }

    // Create vertex shader.
    ID3DBlob *vsBlob;
    ID3D11VertexShader *vertexShader;
    {
        ID3DBlob *shaderCompileErrorsBlob;
        HRESULT hr = D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, &shaderCompileErrorsBlob);
        if (FAILED(hr))
        {
            const char *errorString = nullptr;
            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                errorString = "Couldn't compile shader; file not found.";
            else if (shaderCompileErrorsBlob)
            {
                errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
                shaderCompileErrorsBlob->Release();
            }
            MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
            return 1;
        }
        hr = d3d11Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
        assert(SUCCESS(hr));
    }

    // Create pixel shader.
    ID3D11PixelShader* pixelShader;
    {
        ID3DBlob* psBlob;
        ID3DBlob* shaderCompileErrorsBlob;
        HRESULT hResult = D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, &shaderCompileErrorsBlob);
        if(FAILED(hResult))
        {
            const char* errorString = NULL;
            if(hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                errorString = "Could not compile shader; file not found";
            else if(shaderCompileErrorsBlob){
                errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
                shaderCompileErrorsBlob->Release();
            }
            MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
            return 1;
        }

        hResult = d3d11Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
        assert(SUCCEEDED(hResult));
        psBlob->Release();
    }
    
    // Create input layout.
    ID3D11InputLayout *inputLayout;
    {
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
        {
            { "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        HRESULT hr = d3d11Device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
        assert(SUCCESS(hr));
        vsBlob->Release();
    }

    // Create vertex buffer.
    ID3D11Buffer *vertexBuffer;
    UINT numVerts;
    UINT sride;
    UINT offset;
    {
        float vertexData[] = { // x, y, r, g, b, a
            0.0f,  0.5f, 0.f, 1.f, 0.f, 1.f,
            0.5f, -0.5f, 1.f, 0.f, 0.f, 1.f,
            -0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f
        };
        stride = 6 * sizeof(float);
        numVerts = sizeof(vertexData) / stride;
        offset = 0;
        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.ByteWidth = sizeof(vertexData);
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA vertexSubresourceData = {vertexData};
        HRESULT hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &vertexBuffer);
        assert(SUCCESS(hr));
    }

    // Main loop.
    bool running = true;
    while (running)
    {
        MSG msg = {};
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                running = false;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (gWindowResized)
        {
            d3d11DeviceContext->OMSetRenderTargets(0, 0, 0);
            d3d11FrameBufferView->Release();
            HRESULT hr = d3d11SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
            assert(SUCCEEDED(hr));
            ID3D11Texture2D *d3d11FrameBuffer;
            hr = d3d11SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer);
            assert(SUCCEEDED(hr));
            hr = d3d11Device->CreateRenderTargetView(d3d11FrameBuffer, nullptr, &d3d11FrameBufferView);
            assert(SUCCEEDED(hr));
            d3d11FrameBuffer->Release();
            gWindowResized = false;
        }

        FLOAT backgroundColor[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
        d3d11DeviceContext->ClearRenderTargetView(d3d11FrameBufferView, backgroundColor);
        RECT winRect;
        GetClient(hwnd, &winRect);
        D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)(winRect.right - winRect.left), (FLOAT)(winRect.bottom - winRect.top), 0.0f, 1.0f };
        d3d11DeviceContext->RSSetViewports(1, &viewport);
        d3d11DeviceContext->OMSetRenderTargets(1, &d3d11FrameBufferView, nullptr);
        d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        d3d11DeviceContext->IASetInputLayout(inputLayout);
        d3d11DeviceContext->VSSetShader(vertexShader, nullptr, 0);
        d3d11DeviceContext->PSSetShader(pixelShader, nullptr, 0);
        d3d11DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
        d3d11DeviceContext->Draw(numVerts, 0);
        d3d11SwapChain->Present(1, 0);
    }
    return 0;
}
