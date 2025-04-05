#ifndef RENDER_H
#define RENDER_H

#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <d3d11.h>

enum {
  TEXTURE_FLAG_GENERATE_MIPS = (1<<0),
};

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

struct Shader_Uniform {
  String8 name;
  ID3D11Buffer *buffer = nullptr;
  void *memory = nullptr;
  u32 size = 0;
  b32 dirty = 0;
};

struct Shader_Constant {
  String8 name;
  Shader_Uniform *uniform;
  u32 offset = 0;
  u32 size = 0;
};

struct Shader_Loc {
  String8 name;
  int vertex = -1;
  int pixel  = -1;
};

struct Shader_Bindings {
  Auto_Array<Shader_Constant> constants;
  Auto_Array<Shader_Loc> texture_locations;
  Auto_Array<Shader_Loc> sampler_locations;
  Auto_Array<Shader_Loc> uniform_locations;
  Auto_Array<Shader_Uniform*> uniforms;

  Shader_Uniform *lookup_uniform(String8 name);
};

// enum {
//   VERTEX_FORMAT_XCUU,
//   VERTEX_FORMAT_XNCUU,
//   VERTEX_FORMAT_XYZ,
//   VERTEX_FORMAT_RECT,
//   VERTEX_FORMAT_XUUARGB,
// };

struct Shader {
  String8 name;
  String8 file_name;
  u64 last_write_time;

  ID3D11InputLayout *input_layout;
  ID3D11VertexShader *vertex_shader;
  ID3D11PixelShader *pixel_shader;
  Shader_Bindings *bindings;
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

  Auto_Array<Shader *> shaders;

  Texture *fallback_tex;
};


internal inline R_D3D11_State *r_d3d11_state();

internal ID3D11Buffer *make_vertex_buffer(void *data, size_t elem_count, size_t elem_size);
internal void write_uniform_buffer(ID3D11Buffer *buffer, void *data, UINT bytes);

internal Texture *r_create_texture_from_file(String8 file_name, int flags);
internal Texture *r_create_texture(u8 *data, DXGI_FORMAT format, int w, int h, int flags);


internal Shader *r_d3d11_make_shader(String8 file_name, String8 program_name, D3D11_INPUT_ELEMENT_DESC input_elements[], int elements_count);
internal void r_d3d11_compile_shader(String8 file_name, String8 program_name, ID3D11VertexShader **vertex_shader, ID3D11PixelShader **pixel_shader, ID3D11InputLayout **input_layout, D3D11_INPUT_ELEMENT_DESC input_elements[], int elements_count);

#endif // RENDER_H
