/**************************************************************************
 *
 * Copyright 2012 Jose Fonseca
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/


#include <windows.h>

#include <d3d8.h>

#include <wrl/client.h>

using Microsoft::WRL::ComPtr;


int
main(int argc, char *argv[])
{
    HRESULT hr;

    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX),
        CS_CLASSDC,
        DefWindowProc,
        0,
        0,
        hInstance,
        NULL,
        NULL,
        NULL,
        NULL,
        "SimpleDX8",
        NULL
    };
    RegisterClassEx(&wc);

    const int WindowWidth = 250;
    const int WindowHeight = 250;
    BOOL Windowed = TRUE;

    DWORD dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW;

    RECT rect = {0, 0, WindowWidth, WindowHeight};
    AdjustWindowRect(&rect, dwStyle, FALSE);

    HWND hWnd = CreateWindow(wc.lpszClassName,
                             "Simple example using DirectX8",
                             dwStyle,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             rect.right - rect.left,
                             rect.bottom - rect.top,
                             NULL,
                             NULL,
                             hInstance,
                             NULL);
    if (!hWnd) {
        return 1;
    }

    ComPtr<IDirect3D8> pD3D;
    pD3D.Attach(Direct3DCreate8(D3D_SDK_VERSION));
    if (!pD3D) {
        return 1;
    }

    D3DCAPS8 caps;
    hr = pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
    if (FAILED(hr)) {
       return EXIT_SKIP;
    }

    DWORD dwBehaviorFlags;
    if ((caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
        caps.VertexShaderVersion < D3DVS_VERSION(1, 1)) {
       dwBehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    } else {
       dwBehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    }

    D3DDISPLAYMODE Mode;
    hr = pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &Mode);
    if (FAILED(hr)) {
        return 1;
    }

    D3DPRESENT_PARAMETERS PresentationParameters;
    ZeroMemory(&PresentationParameters, sizeof PresentationParameters);
    PresentationParameters.Windowed = Windowed;
    if (Windowed) {
        PresentationParameters.BackBufferWidth = WindowWidth;
        PresentationParameters.BackBufferHeight = WindowHeight;
    } else {
        PresentationParameters.BackBufferWidth = Mode.Width;
        PresentationParameters.BackBufferHeight = Mode.Height;
    }
    PresentationParameters.BackBufferFormat = Mode.Format;
    PresentationParameters.BackBufferCount = 1;
    PresentationParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    if (!Windowed) {
        PresentationParameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }
    PresentationParameters.hDeviceWindow = hWnd;

    PresentationParameters.EnableAutoDepthStencil = FALSE;

    ComPtr<IDirect3DDevice8> pDevice;
    hr = pD3D->CreateDevice(D3DADAPTER_DEFAULT,
                            D3DDEVTYPE_HAL,
                            hWnd,
                            dwBehaviorFlags,
                            &PresentationParameters,
                            &pDevice);
    if (FAILED(hr)) {
        return 1;
    }

    struct Vertex {
        float x, y, z;
        DWORD color;
    };


    D3DCOLOR clearColor = D3DCOLOR_COLORVALUE(0.3f, 0.1f, 0.3f, 1.0f);
    pDevice->Clear(0, NULL, D3DCLEAR_TARGET, clearColor, 1.0f, 0);
    pDevice->BeginScene();

    pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    static const Vertex vertices[] = {
        { -0.9f, -0.9f, 0.5f, D3DCOLOR_COLORVALUE(0.8f, 0.0f, 0.0f, 0.1f) },
        {  0.9f, -0.9f, 0.5f, D3DCOLOR_COLORVALUE(0.0f, 0.9f, 0.0f, 0.1f) },
        {  0.0f,  0.9f, 0.5f, D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.7f, 0.1f) },
    };

    pDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE);
    pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, vertices, sizeof(Vertex));

    pDevice->EndScene();

    pDevice->Present(NULL, NULL, NULL, NULL);


    DestroyWindow(hWnd);

    return 0;
}

