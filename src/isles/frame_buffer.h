#if !defined(FRAME_BUFFER_H)
#define FRAME_BUFFER_H

struct Frame_Buffer {
  Vector2Int dimension;
  ID3D11RenderTargetView *render_target = nullptr;
  ID3D11DepthStencilView *depth_stencil = nullptr;
  Texture *texture = nullptr;
};

internal Frame_Buffer *make_frame_buffer(int width, int height, DXGI_FORMAT format, DXGI_FORMAT depth_format = DXGI_FORMAT_D24_UNORM_S8_UINT);
internal void clear_frame_buffer(Frame_Buffer *frame_buffer, f32 r, f32 g, f32 b, f32 a);

#endif // FRAME_BUFFER_H
