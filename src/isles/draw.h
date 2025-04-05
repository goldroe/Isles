#ifndef DRAW_H
#define DRAW_H

struct Vertex_List {
  void *data = nullptr;
  u64 byte_count = 0;
  u64 byte_capacity = 0;
};

struct Shadow_Map {
  Texture *texture = nullptr;
  ID3D11Texture2D *tex2d = nullptr;
  ID3D11DepthStencilView *depth_stencil_view = nullptr;
};

internal void draw_world(World *world, Camera camera);

internal void set_shader(Shader *shader);
internal void set_texture(String8 name, Texture *texture);

internal void set_rasterizer_state(Rasterizer_State_Kind rasterizer_kind);
internal void set_blend_state(Blend_State_Kind blend_state_kind);
internal void set_depth_state(Depth_State_Kind depth_state_kind);

internal void draw_imm_cube(Vector3 center, f32 size, Vector4 color);
internal void draw_imm_rectangle(Vector3 position, Vector3 size, Vector4 color);

internal void draw_imm_quad(Texture *texture, Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3, Vector2 uv0, Vector2 uv1, Vector2 uv2, Vector2 uv3, Vector4 color);

#endif // DRAW_H
