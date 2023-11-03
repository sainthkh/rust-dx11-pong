
// #pragma comment(lib, "user32")
// #pragma comment(lib, "d3d11")
// #pragma comment(lib, "d3dcompiler")

///////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>

#include <vector>

#include <math.h>  // sin, cos
#include "xdata.h" // 3d model

///////////////////////////////////////////////////////////////////////////////////////////////////

#define TITLE "Minimal D3D11 by d7samurai"

typedef void(*PFN)(void);
PFN pfnLoopBottom;
PFN pfnLoopTop;

///////////////////////////////////////////////////////////////////////////////////////////////////

struct float3 { float x, y, z; };
struct matrix { float m[4][4]; };

matrix operator*(const matrix& m1, const matrix& m2);

///////////////////////////////////////////////////////////////////////////////////////////////////

ID3D11Device1* device;
ID3D11DeviceContext1* deviceContext;

ID3D11RenderTargetView* frameBufferView;
ID3D11DepthStencilView* depthBufferView;
IDXGISwapChain1* swapChain;

int wndWidth;
int wndHeight;

void InitD3D()
{
    WNDCLASSA wndClass = { 0, DefWindowProcA, 0, 0, 0, 0, 0, 0, 0, TITLE };

    RegisterClassA(&wndClass);

    HWND window = CreateWindowExA(0, TITLE, TITLE, WS_POPUP | WS_MAXIMIZE | WS_VISIBLE, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);

    ///////////////////////////////////////////////////////////////////////////////////////////////

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    
    ID3D11Device* baseDevice;
    ID3D11DeviceContext* baseDeviceContext;

    D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &baseDevice, nullptr, &baseDeviceContext);

    ///////////////////////////////////////////////////////////////////////////////////////////////

    baseDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&device));

    baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&deviceContext));

    ///////////////////////////////////////////////////////////////////////////////////////////////

    IDXGIDevice1* dxgiDevice;

    device->QueryInterface(__uuidof(IDXGIDevice1), reinterpret_cast<void**>(&dxgiDevice));

    IDXGIAdapter* dxgiAdapter;

    dxgiDevice->GetAdapter(&dxgiAdapter);

    IDXGIFactory2* dxgiFactory;

    dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory));

    ///////////////////////////////////////////////////////////////////////////////////////////////

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
    swapChainDesc.Width              = 0; // use window width
    swapChainDesc.Height             = 0; // use window height
    swapChainDesc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    swapChainDesc.Stereo             = FALSE;
    swapChainDesc.SampleDesc.Count   = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount        = 2;
    swapChainDesc.Scaling            = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD; // prefer DXGI_SWAP_EFFECT_FLIP_DISCARD, see Minimal D3D11 pt2 
    swapChainDesc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags              = 0;

    dxgiFactory->CreateSwapChainForHwnd(device, window, &swapChainDesc, nullptr, nullptr, &swapChain);

    ///////////////////////////////////////////////////////////////////////////////////////////////

    ID3D11Texture2D* frameBuffer;

    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&frameBuffer));

    device->CreateRenderTargetView(frameBuffer, nullptr, &frameBufferView);

    ///////////////////////////////////////////////////////////////////////////////////////////////

    D3D11_TEXTURE2D_DESC depthBufferDesc;

    frameBuffer->GetDesc(&depthBufferDesc); // copy from framebuffer properties

    depthBufferDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    wndWidth = depthBufferDesc.Width;
    wndHeight = depthBufferDesc.Height;

    ID3D11Texture2D* depthBuffer;

    device->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);

    device->CreateDepthStencilView(depthBuffer, nullptr, &depthBufferView);
}

struct VertexShaderHandle {
    ID3DBlob* blob;
    ID3D11VertexShader* shader;
};

#include <stdio.h>

VertexShaderHandle CreateVertexShader(LPCWSTR path, LPCSTR entryPoint) {
    ID3DBlob* blob;
    D3DCompileFromFile(path, nullptr, nullptr, entryPoint, "vs_5_0", 0, 0, &blob, nullptr);

    ID3D11VertexShader* shader;
    device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader);

    return { blob, shader };
}

struct InputLayoutHandle {
    ID3D11InputLayout* layout;
};

struct InputElementDesc {
    LPCSTR SemanticName;
    DXGI_FORMAT Format;
};

InputLayoutHandle CreateInputLayout(const InputElementDesc* inputElementDesc, UINT numElements, const VertexShaderHandle* vertexShader) {
    ID3D11InputLayout* inputLayout;

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescs;

    for (UINT i = 0; i < numElements; i++) {
        inputElementDescs.push_back({
            inputElementDesc[i].SemanticName,
            0,
            inputElementDesc[i].Format,
            0,
            i == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        });
    }
    
    device->CreateInputLayout(inputElementDescs.data(), numElements, vertexShader->blob->GetBufferPointer(), vertexShader->blob->GetBufferSize(), &inputLayout);

    return { inputLayout };
}

struct PixelShaderHandle {
    ID3DBlob* blob;
    ID3D11PixelShader* shader;
};

PixelShaderHandle CreatePixelShader(LPCWSTR path, LPCSTR entryPoint) {
    ID3DBlob* blob;
    D3DCompileFromFile(path, nullptr, nullptr, entryPoint, "ps_5_0", 0, 0, &blob, nullptr);

    ID3D11PixelShader* shader;
    device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader);

    return { blob, shader };
}

struct RasterizerStateHandle {
    ID3D11RasterizerState1* state;
};

RasterizerStateHandle CreateRasterizerState() {
    D3D11_RASTERIZER_DESC1 rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;

    ID3D11RasterizerState1* rasterizerState;

    device->CreateRasterizerState1(&rasterizerDesc, &rasterizerState);

    return { rasterizerState };
}

struct SamplerStateHandle {
    ID3D11SamplerState* state;
};

SamplerStateHandle CreateSamplerState() {
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    ID3D11SamplerState* samplerState;

    device->CreateSamplerState(&samplerDesc, &samplerState);

    return { samplerState };
}

struct DepthStencilStateHandle {
    ID3D11DepthStencilState* state;
};

DepthStencilStateHandle CreateDepthStencilState() {
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable    = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc      = D3D11_COMPARISON_LESS;

    ID3D11DepthStencilState* depthStencilState;

    device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);

    return { depthStencilState };
}

struct BufferHandle {
    ID3D11Buffer* buffer;
};

struct Constants
{
    matrix Transform;
    matrix Projection;
    float3 LightVector;
};

BufferHandle CreateConstantBuffer() {
    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.ByteWidth      = sizeof(Constants) + 0xf & 0xfffffff0; // round constant buffer size to 16 byte boundary
    constantBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer* constantBuffer;

    device->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);

    return { constantBuffer };
}

BufferHandle CreateVertexBuffer(const void* data, UINT size) {
    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = size;
    vertexBufferDesc.Usage     = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexData = { data };

    ID3D11Buffer* vertexBuffer;

    device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

    return { vertexBuffer };
}

BufferHandle CreateIndexBuffer(const void* data, UINT size) {
    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = size;
    indexBufferDesc.Usage     = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexData = { data };

    ID3D11Buffer* indexBuffer;

    device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

    return { indexBuffer };
}

struct TextureViewHandle {
    ID3D11ShaderResourceView* view;
};

TextureViewHandle CreateTextureView(const void* data, UINT width, UINT height) {
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width              = width;
    textureDesc.Height             = height;
    textureDesc.MipLevels          = 1;
    textureDesc.ArraySize          = 1;
    textureDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    textureDesc.SampleDesc.Count   = 1;
    textureDesc.Usage              = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA textureData = {};
    textureData.pSysMem            = data;
    textureData.SysMemPitch        = width * sizeof(UINT); // 4 bytes per pixel

    ID3D11Texture2D* texture;

    device->CreateTexture2D(&textureDesc, &textureData, &texture);

    ID3D11ShaderResourceView* textureView;

    device->CreateShaderResourceView(texture, nullptr, &textureView);

    return { textureView };
}

void MapConstantBuffer(BufferHandle* constantBuffer, void* data, UINT size) {
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;

    deviceContext->Map(constantBuffer->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);

    // memcpy(mappedSubresource.pData, data, size);
    Constants* d = reinterpret_cast<Constants*>(data);
    Constants* constants = reinterpret_cast<Constants*>(mappedSubresource.pData);

    constants->Transform   = d->Transform;
    constants->Projection  = d->Projection;
    constants->LightVector = d->LightVector;

    deviceContext->Unmap(constantBuffer->buffer, 0);
}

void ClearView(float* backgroundColor) {
    deviceContext->ClearRenderTargetView(frameBufferView, backgroundColor);
    deviceContext->ClearDepthStencilView(depthBufferView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void IASetBuffers(InputLayoutHandle* inputlayout, BufferHandle* vertexBuffer, UINT stride, BufferHandle* indexBuffer) {
    UINT offset = 0;
    
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(inputlayout->layout);
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer->buffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(indexBuffer->buffer, DXGI_FORMAT_R32_UINT, 0);
}

void SetVertexShader(VertexShaderHandle* vertexShader, BufferHandle* constantBuffer) {
    deviceContext->VSSetShader(vertexShader->shader, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer->buffer);
}

void SetRasterizer(D3D11_VIEWPORT* viewport, RasterizerStateHandle* rasterizerState) {
    deviceContext->RSSetViewports(1, viewport);
    deviceContext->RSSetState(rasterizerState->state);
}

void SetPixelShader(PixelShaderHandle* pixelShader, TextureViewHandle* textureView, SamplerStateHandle* samplerState) {
    deviceContext->PSSetShader(pixelShader->shader, nullptr, 0);
    deviceContext->PSSetShaderResources(0, 1, &textureView->view);
    deviceContext->PSSetSamplers(0, 1, &samplerState->state);
}

void SetRenderTarget(DepthStencilStateHandle* depthStencilState) {
    deviceContext->OMSetRenderTargets(1, &frameBufferView, depthBufferView);
    deviceContext->OMSetDepthStencilState(depthStencilState->state, 0);
    deviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // use default blend mode (i.e. disable)
}

void DrawIndexed(UINT indexCount) {
    deviceContext->DrawIndexed(indexCount, 0, 0);
}

void Present() {
    swapChain->Present(1, 0);
}

bool MessageLoop() {
    MSG msg;

    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_KEYDOWN) return true;
        DispatchMessageA(&msg);
    }

    return false;
}

void GetTransform(float3 modelRotation, float3 modelScale, float3 modelTranslation, float** data) {
    matrix rotateX   = { 1, 0, 0, 0, 0, static_cast<float>(cos(modelRotation.x)), -static_cast<float>(sin(modelRotation.x)), 0, 0, static_cast<float>(sin(modelRotation.x)), static_cast<float>(cos(modelRotation.x)), 0, 0, 0, 0, 1 };
    matrix rotateY   = { static_cast<float>(cos(modelRotation.y)), 0, static_cast<float>(sin(modelRotation.y)), 0, 0, 1, 0, 0, -static_cast<float>(sin(modelRotation.y)), 0, static_cast<float>(cos(modelRotation.y)), 0, 0, 0, 0, 1 };
    matrix rotateZ   = { static_cast<float>(cos(modelRotation.z)), -static_cast<float>(sin(modelRotation.z)), 0, 0, static_cast<float>(sin(modelRotation.z)), static_cast<float>(cos(modelRotation.z)), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
    matrix scale     = { modelScale.x, 0, 0, 0, 0, modelScale.y, 0, 0, 0, 0, modelScale.z, 0, 0, 0, 0, 1 };
    matrix translate = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, modelTranslation.x, modelTranslation.y, modelTranslation.z, 1 };

    matrix r = rotateX * rotateY * rotateZ * scale * translate;

    for(int i = 0; i < 16; i++) {
        (*data)[i] = r.m[i / 4][i % 4];
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

matrix operator*(const matrix& m1, const matrix& m2)
{
    return
    {
        m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0],
        m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1],
        m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2],
        m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] + m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3],
        m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0],
        m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1],
        m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2],
        m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] + m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3],
        m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0],
        m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1],
        m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2],
        m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] + m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3],
        m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0],
        m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1],
        m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2],
        m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] + m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3],
    };
}

extern "C" void dx_init_d3d() {
    InitD3D();
}

extern "C" int dx_wnd_width() {
    return wndWidth;
}

extern "C" int dx_wnd_height() {
    return wndHeight;
}

extern "C" VertexShaderHandle dx_create_vertex_shader(LPCWSTR path, LPCSTR entryPoint) {
    return CreateVertexShader(path, entryPoint);
}

extern "C" InputLayoutHandle dx_create_input_layout(const InputElementDesc* inputElementDesc, UINT numElements, const VertexShaderHandle* vertexShader) {
    return CreateInputLayout(inputElementDesc, numElements, vertexShader);
}

extern "C" PixelShaderHandle dx_create_pixel_shader(LPCWSTR path, LPCSTR entryPoint) {
    return CreatePixelShader(path, entryPoint);
}

extern "C" RasterizerStateHandle dx_create_rasterizer_state() {
    return CreateRasterizerState();
}

extern "C" SamplerStateHandle dx_create_sampler_state() {
    return CreateSamplerState();
}

extern "C" DepthStencilStateHandle dx_create_depth_stencil_state() {
    return CreateDepthStencilState();
}

extern "C" BufferHandle dx_create_constant_buffer() {
    return CreateConstantBuffer();
}

extern "C" BufferHandle dx_create_vertex_buffer(const void* data, UINT size) {
    return CreateVertexBuffer(data, size);
}

extern "C" BufferHandle dx_create_index_buffer(const void* data, UINT size) {
    return CreateIndexBuffer(data, size);
}

extern "C" TextureViewHandle dx_create_texture_view(const void* data, UINT width, UINT height) {
    return CreateTextureView(data, width, height);
}

extern "C" void dx_map_constant_buffer(BufferHandle* constantBuffer, void* data, UINT size) {
    MapConstantBuffer(constantBuffer, data, size);
}

extern "C" void dx_clear_view(float* backgroundColor) {
    ClearView(backgroundColor);
}

extern "C" void dx_ia_set_buffers(InputLayoutHandle* inputlayout, BufferHandle* vertexBuffer, UINT stride, BufferHandle* indexBuffer) {
    IASetBuffers(inputlayout, vertexBuffer, stride, indexBuffer);
}

extern "C" void dx_set_vertex_shader(VertexShaderHandle* vertexShader, BufferHandle* constantBuffer) {
    SetVertexShader(vertexShader, constantBuffer);
}

extern "C" void dx_set_rasterizer(D3D11_VIEWPORT* viewport, RasterizerStateHandle* rasterizerState) {
    SetRasterizer(viewport, rasterizerState);
}

extern "C" void dx_set_pixel_shader(PixelShaderHandle* pixelShader, TextureViewHandle* textureView, SamplerStateHandle* samplerState) {
    SetPixelShader(pixelShader, textureView, samplerState);
}

extern "C" void dx_set_render_target(DepthStencilStateHandle* depthStencilState) {
    SetRenderTarget(depthStencilState);
}

extern "C" void dx_draw_indexed(UINT indexCount) {
    DrawIndexed(indexCount);
}

extern "C" void dx_present() {
    Present();
}

extern "C" bool dx_message_loop() {
    return MessageLoop();
}

extern "C" void dx_get_transform(float3 modelRotation, float3 modelScale, float3 modelTranslation, float** data) {
    GetTransform(modelRotation, modelScale, modelTranslation, data);
}
