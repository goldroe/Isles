#ifndef RENDER_H
#define RENDER_H

#include <d3dcompiler.h>
#include <D3d11.h>

struct Texture {
  int width;
  int height;
  DXGI_FORMAT format;
  union {
    void *view;
  };
};

enum Depth_State_Kind {
  DEPTH_STATE_DEFAULT,
  DEPTH_STATE_DISABLE,
  DEPTH_STATE_COUNT
};

enum Rasterizer_State_Kind {
  RASTERIZER_STATE_DEFAULT,
  RASTERIZER_STATE_NO_CULL,
  RASTERIZER_STATE_TEXT,
  RASTERIZER_STATE_COUNT
};

enum Blend_State_Kind {
  BLEND_STATE_DEFAULT,
  BLEND_STATE_ALPHA,
  BLEND_STATE_COUNT
};

enum Sampler_State_Kind {
  SAMPLER_STATE_LINEAR,
  SAMPLER_STATE_POINT,
  SAMPLER_STATE_COUNT
};

enum Shader_Kind {
  SHADER_BASIC,
  SHADER_MESH,
  SHADER_RECT,
  SHADER_PICKER,
  SHADER_SHADOW_MAP,
  SHADER_COUNT
};

struct Shader {
  String8 name;
  String8 file_name;
  u64 last_write_time;

  ID3D11InputLayout *input_layout;
  ID3D11VertexShader *vertex_shader;
  ID3D11PixelShader *pixel_shader;
};

enum Uniform_Kind {
  UNIFORM_BASIC,
  UNIFORM_MESH,
  UNIFORM_RECT,
  UNIFORM_PICKER,
  UNIFORM_SHADOW_MAP,
  UNIFORM_COUNT 
};

struct R_Uniform_Basic3D {
  Matrix4 transform;
};

struct R_Uniform_Mesh {
  Matrix4 transform;
  Matrix4 world_matrix;
  Vector3 light_dir;
  f32 _p0;
};

struct R_D3D11_Uniform_Rect {
  Matrix4 transform;
};

struct R_D3D11_Uniform_Picker {
  Matrix4 transform;
  Vector4 pick_color;
};

struct R_Uniform_Shadow_Map {
  Matrix4 world;
  Matrix4 light_view_projection;
};

struct Render_Target;

struct R_D3D11_State {
  Arena *arena;
  Rect draw_region;
  Vector4 clear_color;
  Vector2Int window_dimension = Vector2Int(0, 0);

  ID3D11Device *device = nullptr;
  ID3D11DeviceContext *device_context = nullptr;
  IDXGISwapChain *swap_chain = nullptr;
  Render_Target *default_render_target = nullptr;
  ID3D11DepthStencilState *depth_stencil_states[DEPTH_STATE_COUNT];
  ID3D11RasterizerState *rasterizer_states[RASTERIZER_STATE_COUNT];
  ID3D11BlendState *blend_states[BLEND_STATE_COUNT];
  ID3D11SamplerState *sampler_states[SAMPLER_STATE_COUNT];

  Shader *shaders[SHADER_COUNT];
  ID3D11Buffer *uniform_buffers[UNIFORM_COUNT];

  ID3D11ShaderResourceView *fallback_tex;
};


internal inline R_D3D11_State *r_d3d11_state();

internal ID3D11Buffer *make_vertex_buffer(void *data, size_t elem_count, size_t elem_size);
internal void write_uniform_buffer(ID3D11Buffer *buffer, void *data, UINT offset, UINT bytes);

internal Texture *r_create_texture_from_file(String8 file_name);
internal Texture *r_create_texture(u8 *data, DXGI_FORMAT format, int w, int h);


internal Shader *r_d3d11_make_shader(String8 file_name, String8 program_name, Shader_Kind shader_kind, D3D11_INPUT_ELEMENT_DESC input_elements[], int elements_count);
internal void r_d3d11_compile_shader(String8 file_name, String8 program_name, ID3D11VertexShader **vertex_shader, ID3D11PixelShader **pixel_shader, ID3D11InputLayout **input_layout, D3D11_INPUT_ELEMENT_DESC input_elements[], int elements_count);

#endif // RENDER_H
