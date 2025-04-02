#if !defined(RENDER_TARGET_H)
#define RENDER_TARGET_H

struct Render_Target {
  int width = 0;
  int height = 0;
  void *texture = nullptr;
  void *render_target_view = nullptr;
  void *depth_stencil_view = nullptr;
};

internal Render_Target *make_render_target(int width, int height, DXGI_FORMAT format, DXGI_FORMAT depth_format = DXGI_FORMAT_D24_UNORM_S8_UINT);

#endif // RENDER_TARGET_H
