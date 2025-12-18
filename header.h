
#pragma region includes
#define NOMINMAX
#include <audioclient.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <hidusage.h>
#include <mmdeviceapi.h>
#include <tiny_gltf.h>
#include <wrl/client.h>

#include <SimpleMath.h>
#include <SpriteFont.h>
// #include <WaveFrontReader.h>

#include <array>
#include <cstdint>
#include <map>
#include <numbers>
#include <print>
#include <random>
#include <set>
#include <thread>
#include <vector>
#pragma endregion

#undef near

using namespace std;
// using namespace DX;
using namespace DirectX;
using namespace SimpleMath;

#define null nullptr
#define CAT(left, right) left##right
#define SELECT(name, num) CAT(name##_, num)
#define GET_COUNT(_1, _2, _3, _4, _5, _6, count, ...) count
#define VA_SIZE(...) GET_COUNT(__VA_ARGS__, 6, 5, 4, 3, 2, 1)
#define VA_SELECT(name, ...) SELECT(name, VA_SIZE(__VA_ARGS__))(__VA_ARGS__)
#define range(...) VA_SELECT(range, __VA_ARGS__)
#define range_2(i, n) for (int i = 0; i < n; i++)
#define range_3(i, a, b) for (int i = a; i < b; i++)
#define irange(i, a, b) for (int i = a; i <= b; i++)
#define self (*this)
#define pi 3.1415927f

// name is either attributes["x"] or indices
#define PRIMITIVE(m, index) m.meshes[index].primitives[0]
#define ACCESSOR(m, index, name) m.accessors[PRIMITIVE(m, index).name]
#define BUFFER_VIEW(m, index, name) m.bufferViews[ACCESSOR(m, index, name).bufferView]
#define BUFFER(m, index, name) m.buffers[BUFFER_VIEW(m, index, name).buffer]

#define DATA(m, index, name) (BUFFER(m, index, name).data.data() + BUFFER_VIEW(m, index, name).byteOffset)
#define LEN(m, index, name) u32(BUFFER_VIEW(m, index, name).byteLength)
#define COUNT(m, index, name) ACCESSOR(m, index, name).count

#define BIT_WIDTH(x) u32(bit_width(u32(x)))

template <class T>
T* operator&(T&& x)
{
   return addressof(x);
}

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

template <class T>
struct ComPtr : public Microsoft::WRL::ComPtr<T>
{
   operator T*()
   {
      return this->Get();
   }
};

#include "math.h"
#include "shared.h"

struct Texture2D
{
   ComPtr<ID3D11Texture2D> tex;
   ComPtr<ID3D11RenderTargetView> rtv;
   ComPtr<ID3D11DepthStencilView> dsv;
   ComPtr<ID3D11ShaderResourceView> srv;
   operator ID3D11Texture2D*()
   {
      return tex;
   }
   operator ID3D11RenderTargetView*()
   {
      return rtv;
   }
   operator ID3D11DepthStencilView*()
   {
      return dsv;
   }
   operator ID3D11ShaderResourceView*()
   {
      return srv;
   }
};

ComPtr<ID3D11Device> dev;
ComPtr<ID3D11DeviceContext> ctx;
ComPtr<IDXGISwapChain> sc;

void setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY top)
{
   ctx->IASetPrimitiveTopology(top);
}

void clearRenderTargetView(ID3D11RenderTargetView* rtv, vec3 color)
{
   ctx->ClearRenderTargetView(rtv, vec4{color[0], color[1], color[2], 1}.data);
}

void clearDepthStencilView(ID3D11DepthStencilView* dsv, float depth)
{
   ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, depth, 0);
}

void setRenderTargets(vector<ID3D11RenderTargetView*> rtvs, ID3D11DepthStencilView* dsv)
{
   ctx->OMSetRenderTargets(rtvs.size(), rtvs.data(), dsv);
}

void setViewports(vector<D3D11_VIEWPORT> vps)
{
   ctx->RSSetViewports(vps.size(), vps.data());
}

void setVertexConstants(int start, vector<ID3D11Buffer*> cbs)
{
   ctx->VSSetConstantBuffers(start, cbs.size(), cbs.data());
}

void setPixelConstants(int start, vector<ID3D11Buffer*> cbs)
{
   ctx->PSSetConstantBuffers(start, cbs.size(), cbs.data());
}

void setVertexResources(int start, vector<ID3D11ShaderResourceView*> srvs)
{
   ctx->VSSetShaderResources(start, srvs.size(), srvs.data());
}

void setPixelResources(int start, vector<ID3D11ShaderResourceView*> srvs)
{
   ctx->PSSetShaderResources(start, srvs.size(), srvs.data());
}

void setSamplers(int start, vector<ID3D11SamplerState*> samplers)
{
   ctx->PSSetSamplers(start, samplers.size(), samplers.data());
}

void setIndexBuffer(ID3D11Buffer* ib)
{
   ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R16_UINT, 0);
}

void setDepthState(ID3D11DepthStencilState* ds)
{
   ctx->OMSetDepthStencilState(ds, 0);
}

void setBlendState(ID3D11BlendState* bs)
{
   ctx->OMSetBlendState(bs, null, 0xffffffff);
}

void setRasterState(ID3D11RasterizerState* rs)
{
   ctx->RSSetState(rs);
}

void setVertexShader(ID3D11VertexShader* vs)
{
   ctx->VSSetShader(vs, null, 0);
}

void setPixelShader(ID3D11PixelShader* ps)
{
   ctx->PSSetShader(ps, null, 0);
}

void draw(int count)
{
   ctx->Draw(count, 0);
}

void drawIndexed(int count)
{
   ctx->DrawIndexed(count, 0, 0);
}

struct Mesh
{
   ComPtr<ID3D11ShaderResourceView> points;
   ComPtr<ID3D11ShaderResourceView> normals;
   ComPtr<ID3D11ShaderResourceView> uvs;
   ComPtr<ID3D11Buffer> indices;
   int count;
   Texture2D diffuse;

   void bind()
   {
      setVertexResources(0, {points, normals, uvs});
      setPixelResources(0, {diffuse});
      setIndexBuffer(indices);
   }

   void draw()
   {
      drawIndexed(count);
   }
};

D3D11_VIEWPORT viewport(float width, float height)
{
   return {
      .Width = width,
      .Height = height,
      .MaxDepth = 1,
   };
}

ComPtr<ID3D11RenderTargetView> createRenderTargetView(ID3D11Texture2D* tex)
{
   ComPtr<ID3D11RenderTargetView> rtv;
   dev->CreateRenderTargetView(tex, null, &rtv);
   return rtv;
}
ComPtr<ID3D11RenderTargetView> createRenderTargetView(ID3D11Texture2D* tex, D3D11_RENDER_TARGET_VIEW_DESC desc)
{
   ComPtr<ID3D11RenderTargetView> rtv;
   dev->CreateRenderTargetView(tex, &desc, &rtv);
   return rtv;
}

ComPtr<ID3D11DepthStencilView> createDepthStencilView(ID3D11Texture2D* tex)
{
   ComPtr<ID3D11DepthStencilView> dsv;
   dev->CreateDepthStencilView(tex, null, &dsv);
   return dsv;
}

ComPtr<ID3D11DepthStencilView> createDepthStencilView(ID3D11Texture2D* tex, D3D11_DEPTH_STENCIL_VIEW_DESC desc)
{
   ComPtr<ID3D11DepthStencilView> dsv;
   dev->CreateDepthStencilView(tex, &desc, &dsv);
   return dsv;
}

ComPtr<ID3D11ShaderResourceView> createShaderResourceView(ID3D11Resource* res)
{
   ComPtr<ID3D11ShaderResourceView> srv;
   dev->CreateShaderResourceView(res, null, &srv);
   return srv;
}

ComPtr<ID3D11ShaderResourceView> createShaderResourceView(ID3D11Resource* res, D3D11_SHADER_RESOURCE_VIEW_DESC desc)
{
   ComPtr<ID3D11ShaderResourceView> srv;
   dev->CreateShaderResourceView(res, &desc, &srv);
   return srv;
}

ComPtr<ID3D11Buffer> createBuffer(D3D11_BUFFER_DESC desc)
{
   ComPtr<ID3D11Buffer> buf;
   dev->CreateBuffer(&desc, null, &buf);
   return buf;
}

ComPtr<ID3D11Buffer> createBuffer(D3D11_BUFFER_DESC desc, D3D11_SUBRESOURCE_DATA data)
{
   ComPtr<ID3D11Buffer> buf;
   dev->CreateBuffer(&desc, &data, &buf);
   return buf;
}

Texture2D createTexture(D3D11_TEXTURE2D_DESC desc)
{

   if (desc.MipLevels == 0)
      desc.MipLevels = 1;
   if (desc.ArraySize == 0)
      desc.ArraySize = 1;
   if (desc.SampleDesc.Count == 0)
      desc.SampleDesc.Count = 1;

   Texture2D tex;
   dev->CreateTexture2D(&desc, null, &tex.tex);

   if (desc.BindFlags & D3D11_BIND_RENDER_TARGET)
      tex.rtv = createRenderTargetView(tex);

   if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
   {
      if (desc.Format == DXGI_FORMAT_R32_TYPELESS)
         tex.dsv = createDepthStencilView(tex, {
                                                  .Format = DXGI_FORMAT_D32_FLOAT,
                                                  .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
                                               });
      else
         tex.dsv = createDepthStencilView(tex);
   }

   if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
   {

      if (desc.Format == DXGI_FORMAT_R32_TYPELESS)
         tex.srv = createShaderResourceView(tex, {
                                                    .Format = DXGI_FORMAT_R32_FLOAT,
                                                    .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
                                                    .Texture2D = {.MipLevels = desc.MipLevels},
                                                 });
      else
         tex.srv = createShaderResourceView(tex);
   }
   return tex;
}

Texture2D createTexture(D3D11_TEXTURE2D_DESC desc, D3D11_SUBRESOURCE_DATA data)
{
   if (desc.MipLevels == 0)
      desc.MipLevels = 1;
   if (desc.ArraySize == 0)
      desc.ArraySize = 1;
   if (desc.SampleDesc.Count == 0)
      desc.SampleDesc.Count = 1;

   if (desc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)
      desc.BindFlags |= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

   if (desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
      data.SysMemPitch = desc.Width * 4;

   Texture2D tex;
   dev->CreateTexture2D(&desc, vector<D3D11_SUBRESOURCE_DATA>(desc.MipLevels * desc.ArraySize, data).data(), &tex.tex);

   if (desc.BindFlags & D3D11_BIND_RENDER_TARGET)
      tex.rtv = createRenderTargetView(tex);

   if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
      tex.dsv = createDepthStencilView(tex);

   if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
      tex.srv = createShaderResourceView(tex);

   if (desc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)
      ctx->GenerateMips(tex);
   return tex;
}

ComPtr<ID3DBlob> readBlob(wstring path)
{
   ComPtr<ID3DBlob> blob;
   D3DReadFileToBlob(path.data(), &blob);
   return blob;
}

ComPtr<ID3D11VertexShader> createVertexShader(wstring path)
{
   auto blob = readBlob(path);
   ComPtr<ID3D11VertexShader> vs;
   dev->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), null, &vs);
   return vs;
}

ComPtr<ID3D11PixelShader> createPixelShader(wstring path)
{
   auto blob = readBlob(path);
   ComPtr<ID3D11PixelShader> ps;
   dev->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), null, &ps);
   return ps;
}

ComPtr<ID3D11SamplerState> createSamplerState(D3D11_SAMPLER_DESC desc)
{
   ComPtr<ID3D11SamplerState> ss;
   dev->CreateSamplerState(&desc, &ss);
   return ss;
}

ComPtr<ID3D11DepthStencilState> createDepthState(D3D11_DEPTH_STENCIL_DESC desc)
{
   ComPtr<ID3D11DepthStencilState> ds;
   dev->CreateDepthStencilState(&desc, &ds);
   return ds;
}

ComPtr<ID3D11RasterizerState> createRasterState(D3D11_RASTERIZER_DESC desc)
{
   ComPtr<ID3D11RasterizerState> rs;
   dev->CreateRasterizerState(&desc, &rs);
   return rs;
}

ComPtr<ID3D11BlendState> createBlendState(D3D11_BLEND_DESC desc)
{
   ComPtr<ID3D11BlendState> bs;
   dev->CreateBlendState(&desc, &bs);
   return bs;
}

template <class T>
struct cb
{
   ComPtr<ID3D11Buffer> buf;

   operator ID3D11Buffer*()
   {
      return buf;
   }

   cb()
   {
      buf = createBuffer({
         .ByteWidth = sizeof(T) + 15 & ~15,
         .Usage = D3D11_USAGE_DYNAMIC,
         .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
         .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
      });
   }

   void operator=(T data)
   {
      D3D11_MAPPED_SUBRESOURCE mapped;
      ctx->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
      *(T*)mapped.pData = data;
      ctx->Unmap(buf, 0);
   }
};

#include "audio.h"