#ifndef DRAW_H
#define DRAW_H

struct Draw_Context {
  Camera camera;
  Vector3 clip_plane;
};

struct Vertex_List {
  void *data = nullptr;
  u64 byte_count = 0;
  u64 byte_capacity = 0;
};

internal void draw_world(World *world, Camera camera);

internal void set_shader(Shader *shader);
internal void set_texture(String8 name, Texture *texture);

internal void set_blend_state(Blend_State *blend_state);

internal void immediate_quad(Texture *texture, Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3, Vector2 uv0, Vector2 uv1, Vector2 uv2, Vector2 uv3, Vector4 color);

internal void immediate_plane(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, Vector4 color);
internal void immediate_cube(Vector3 center, f32 size, Vector4 color);
internal void immediate_rectangle(Vector3 position, Vector3 size, Vector4 color);

#endif // DRAW_H
