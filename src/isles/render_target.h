#if !defined(RENDER_TARGET_H)
#define RENDER_TARGET_H

struct Render_Target {
  int width;
  int height;
  void *texture;
  void *render_target_view;
  void *depth_stencil_view;
};

internal Render_Target *make_render_target(int width, int height, DXGI_FORMAT format);

#endif // RENDER_TARGET_H
