#ifndef RENDER_H
#define RENDER_H

#include <d3dcompiler.h>
#include <D3d11.h>

enum R_Texture_Format {
  R_TEXTURE_FORMAT_R8G8B8A8,
};

struct Texture {
  int width;
  int height;
  R_Texture_Format format;
  union {
    void *view;
  };
};

enum D3D11_Shader_Kind {
  D3D11_SHADER_BASIC_3D,
  D3D11_SHADER_RECT,
  D3D11_SHADER_PICKER,
  D3D11_SHADER_COUNT
};

struct Shader {
  String8 name;
  String8 file_name;
  u64 last_write_time;

  ID3D11InputLayout *input_layout;
  ID3D11VertexShader *vertex_shader;
  ID3D11PixelShader *pixel_shader;
};

enum D3D11_Uniform_Kind {
  D3D11_UNIFORM_BASIC_3D,
  D3D11_UNIFORM_RECT,
  D3D11_UNIFORM_PICKER,
  D3D11_UNIFORM_COUNT
};

enum R_Depth_State_Kind {
  R_DEPTH_STENCIL_STATE_DEFAULT,
  R_DEPTH_STENCIL_STATE_DISABLE,
  R_DEPTH_STENCIL_STATE_COUNT
};

enum R_Rasterizer_Kind {
  R_RASTERIZER_STATE_DEFAULT,
  R_RASTERIZER_STATE_TEXT,
  R_RASTERIZER_STATE_COUNT
};

enum R_Blend_State_Kind {
  R_BLEND_STATE_DEFAULT,
  R_BLEND_STATE_ALPHA,
  R_BLEND_STATE_COUNT
};

enum R_Sampler_State_Kind {
  R_SAMPLER_STATE_LINEAR,
  R_SAMPLER_STATE_POINT,
  R_SAMPLER_STATE_COUNT
};

struct R_D3D11_Uniform_Basic_3D {
  Matrix4 transform;
  Matrix4 world_matrix;
  // Matrix4 view_matrix;
  // Matrix4 projection_matrix;
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

struct R_D3D11_State {
  Arena *arena;
  Rect draw_region;
  Vector4 clear_color;
  Vector2Int window_dimension = Vector2Int(0, 0);

  ID3D11Device *device = nullptr;
  ID3D11DeviceContext *device_context = nullptr;
  IDXGISwapChain *swap_chain = nullptr;
  ID3D11RenderTargetView *render_target_view = nullptr;
  ID3D11DepthStencilView *depth_stencil_view = nullptr;
  ID3D11DepthStencilState *depth_stencil_states[R_DEPTH_STENCIL_STATE_COUNT];
  ID3D11RasterizerState *rasterizer_states[R_RASTERIZER_STATE_COUNT];
  ID3D11BlendState *blend_states[R_BLEND_STATE_COUNT];
  ID3D11SamplerState *sampler_states[R_SAMPLER_STATE_COUNT];

  Shader *shaders[D3D11_SHADER_COUNT];
  ID3D11Buffer *uniform_buffers[D3D11_UNIFORM_COUNT];

  ID3D11ShaderResourceView *fallback_tex;
};


internal inline R_D3D11_State *r_d3d11_state();

internal Texture *r_create_texture_from_file(String8 file_name);
internal Texture *r_create_texture(u8 *data, R_Texture_Format format, int w, int h);

internal Shader *r_d3d11_make_shader(String8 file_name, String8 program_name, D3D11_Shader_Kind shader_kind, D3D11_INPUT_ELEMENT_DESC input_elements[], int elements_count);
internal void r_d3d11_compile_shader(String8 file_name, String8 program_name, ID3D11VertexShader **vertex_shader, ID3D11PixelShader **pixel_shader, ID3D11InputLayout **input_layout, D3D11_INPUT_ELEMENT_DESC input_elements[], int elements_count);

#endif // RENDER_H
