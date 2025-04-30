#if !defined(RENDER_TARGET_H)
#define RENDER_TARGET_H

struct Render_Target {
  int width = 0;
  int height = 0;
  ID3D11RenderTargetView *render_target_view = nullptr;
  ID3D11DepthStencilView *depth_stencil_view = nullptr;
  Texture *texture;
};

internal Render_Target *make_render_target(int width, int height, DXGI_FORMAT format, DXGI_FORMAT depth_format = DXGI_FORMAT_D24_UNORM_S8_UINT);
internal void clear_render_target(Render_Target *render_target, f32 r, f32 g, f32 b, f32 a);

#endif // RENDER_TARGET_H
