#ifndef DRAW_H
#define DRAW_H

internal void draw_world(World *world, Camera camera);

internal void set_shader(Shader_Kind shader_kind);

internal void set_rasterizer_state(Rasterizer_State_Kind rasterizer_kind);
internal void set_blend_state(Blend_State_Kind blend_state_kind);
internal void set_depth_state(Depth_State_Kind depth_state_kind);

internal void draw_imm_cube(Vector3 center, f32 size, Vector4 color);
internal void draw_imm_rectangle(Vector3 position, Vector3 size, Vector4 color);
internal void draw_imm_quad(Texture *texture, Vector2 position, Vector2 size, Vector2 uv, Vector2 uv_size, Vector4 color);

#endif // DRAW_H
