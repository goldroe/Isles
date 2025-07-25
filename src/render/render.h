#ifndef RENDER_H
#define RENDER_H

#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <d3d11.h>

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
  int geo    = -1;
};

struct Shader_Bindings {
  Auto_Array<Shader_Uniform*> uniforms;
  Auto_Array<Shader_Constant> constants;
  Auto_Array<Shader_Loc> texture_locations;
  Auto_Array<Shader_Loc> sampler_locations;
  Auto_Array<Shader_Loc> uniform_locations;

  Shader_Constant *xform = nullptr;
  Shader_Constant *world = nullptr;

  Shader_Loc *diffuse_texture = nullptr;
};

struct Shader {
  String8 name;
  String8 file_name;
  u64 last_write_time = 0;

  b32 initialized = 0;
  b32 has_geometry_shader = false;
  Shader_Bindings *bindings = nullptr;

  ID3D11InputLayout *input_layout = nullptr;
  ID3D11VertexShader *vertex_shader = nullptr;
  ID3D11PixelShader *pixel_shader = nullptr;
  ID3D11GeometryShader *geometry_shader = nullptr;

  D3D11_INPUT_ELEMENT_DESC *input_elements = nullptr;
  int input_element_count = 0;
};

enum {
  TEXTURE_FLAG_GENERATE_MIPS = (1<<0),
};

struct Texture {
  int width;
  int height;
  DXGI_FORMAT format;
  union {
    ID3D11ShaderResourceView *view;
  };
};

struct Blend_State {
  ID3D11BlendState *resource;
};

struct Sampler {
  ID3D11SamplerState *resource;
};

struct Rasterizer {
  ID3D11RasterizerState *resource;
};

struct Depth_State {
  ID3D11DepthStencilState *resource; 
};

struct R_D3D11_State {
  Allocator allocator;
  Rect draw_region;
  Vector4 clear_color;
  Vector2Int window_dimension = Vector2Int(0, 0);

  ID3D11Device *device = nullptr;
  ID3D11DeviceContext *devcon = nullptr;
  IDXGISwapChain *swap_chain = nullptr;
  ID3D11RenderTargetView *render_target = nullptr;
  ID3D11DepthStencilView *depth_stencil = nullptr;

  Auto_Array<Shader*> shaders;

  Texture *fallback_tex;
};

struct Depth_Map {
  Vector2Int dimension;
  Texture *texture = nullptr;
  ID3D11DepthStencilView *depth_stencil = nullptr;
};

struct R_Point_Light {
  Vector3 position;
  float range;
  Vector4 color;
  Vector3 att;
  float pad;
};

// internal void r_resize_render_target(Vector2Int dimension);
internal inline R_D3D11_State *r_d3d11_state();

internal ID3D11Buffer *make_vertex_buffer(void *data, size_t elem_count, size_t elem_size);
internal void write_uniform_buffer(ID3D11Buffer *buffer, void *data, UINT bytes);

internal Texture *create_texture_from_file(String8 file_name, int flags);
internal Texture *create_texture(u8 *data, DXGI_FORMAT format, int w, int h, int flags);

internal Shader *shader_create(String8 file_name, String8 program_name, D3D11_INPUT_ELEMENT_DESC input_elements[], int elements_count, bool use_geometry_shader);
internal void shader_compile(Shader *shader);

internal Texture *create_texture_cube(String8 file_names[6]);
#endif // RENDER_H
