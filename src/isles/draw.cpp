global Shader *shader_basic;
global Shader *shader_mesh;
global Shader *shader_entity;
global Shader *shader_ui;
global Shader *shader_picker;
global Shader *shader_shadow_map;
global Shader *shader_argb_texture;
global Shader *shader_skinned;
global Shader *shader_skinned_shadow_map;
global Shader *shader_particle;
global Shader *shader_color_wheel;
global Shader *shader_water;
global Shader *shader_skybox;

global Shader *current_shader;
global Vertex_List immediate_vertices;

global Depth_Map *shadow_map;

global Texture *sun_icon_texture;
global Texture *eye_of_horus_texture;
global Texture *flare_texture;

global Depth_State *depth_state_default;
global Depth_State *depth_state_disable;
global Depth_State *depth_state_no_write;
global Depth_State *depth_state_skybox;

global Rasterizer *rasterizer_default;
global Rasterizer *rasterizer_shadow_map;
global Rasterizer *rasterizer_text;
global Rasterizer *rasterizer_no_cull;
global Rasterizer *rasterizer_cull_front;
global Rasterizer *rasterizer_wireframe;

global Blend_State *blend_state_default;
global Blend_State *blend_state_alpha;
global Blend_State *blend_state_additive;

global Sampler *sampler_linear;
global Sampler *sampler_point;
global Sampler *sampler_anisotropic;
global Sampler *sampler_skybox;

global bool display_shadow_map;

global Triangle_Mesh *water_plane_mesh;

global Render_Target *reflection_render_target;
global Render_Target *refraction_render_target;

global Depth_Map *refraction_depth_map;

global Triangle_Mesh *skybox_mesh;
global Texture *skybox_texture;

global f32 water_height = -0.1f;
global Texture *water_dudv_texture;
global Texture *water_normal_map;
global f32 move_factor;

internal Depth_Map *make_depth_map(int width, int height) {
  R_D3D11_State *d3d = r_d3d11_state();

  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R32_TYPELESS;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;

  D3D11_DEPTH_STENCIL_VIEW_DESC depth_desc = {};
  depth_desc.Format = DXGI_FORMAT_D32_FLOAT;
  depth_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  depth_desc.Texture2D.MipSlice = 0;

  D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
  srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
  srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Texture2D.MipLevels = desc.MipLevels;
  srv_desc.Texture2D.MostDetailedMip = 0;

  HRESULT hr = S_OK;
  ID3D11Texture2D *tex2d = nullptr;
  ID3D11DepthStencilView *depth_stencil_view = nullptr;
  ID3D11ShaderResourceView *srv = nullptr;
  hr = d3d->device->CreateTexture2D(&desc, NULL, &tex2d);
  hr = d3d->device->CreateDepthStencilView(tex2d, &depth_desc, &depth_stencil_view);
  hr = d3d->device->CreateShaderResourceView(tex2d, &srv_desc, &srv);

  Texture *texture = new Texture();
  texture->width = width;
  texture->height = height;
  texture->format = desc.Format;
  texture->view = srv;

  Depth_Map *map = new Depth_Map();
  map->depth_stencil_view = depth_stencil_view;
  map->texture = texture;
  return map;
}

internal void set_shader(Shader *shader) {
  R_D3D11_State *d3d = r_d3d11_state();

  if (shader != current_shader) {
    current_shader = shader;

    d3d->device_context->VSSetShader(shader->vertex_shader, nullptr, 0);
    d3d->device_context->PSSetShader(shader->pixel_shader, nullptr, 0);
    d3d->device_context->GSSetShader(shader->geometry_shader, nullptr, 0);
    d3d->device_context->IASetInputLayout(shader->input_layout);

    for (int i = 0; i < shader->bindings->uniforms.count; i++) {
      Shader_Uniform *uniform = shader->bindings->uniforms[i];
      Shader_Loc *loc = &shader->bindings->uniform_locations[i];
      if (loc->vertex != -1) {
        d3d->device_context->VSSetConstantBuffers(loc->vertex, 1, &uniform->buffer);
      }
      if (loc->pixel != -1) {
        d3d->device_context->PSSetConstantBuffers(loc->pixel, 1, &uniform->buffer);
      }
      if (loc->geo != -1) {
        d3d->device_context->GSSetConstantBuffers(loc->geo, 1, &uniform->buffer);
      }
    }
  }
}

internal void reset_primitives() {
  R_D3D11_State *d3d = r_d3d11_state();
  d3d->device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

internal void reset_texture(String8 name) {
  R_D3D11_State *d3d = r_d3d11_state();
  Shader *shader = current_shader;
  Shader_Bindings *bindings = shader->bindings;
  Shader_Loc *loc = nullptr;
  for (int i = 0; i < bindings->texture_locations.count; i++) {
    loc = &bindings->texture_locations[i];
    if (str8_equal(name, loc->name)) {
      break;
    }
    loc = nullptr;
  }

  if (loc) {
    ID3D11ShaderResourceView *null_srv[1] = {nullptr};
    if (loc->vertex != -1) {
      d3d->device_context->VSSetShaderResources(loc->vertex, 1, null_srv);
    }
    if (loc->pixel != -1) {
      d3d->device_context->PSSetShaderResources(loc->pixel, 1, null_srv);
    }
    if (loc->geo != -1) {
      d3d->device_context->PSSetShaderResources(loc->geo, 1, null_srv);
    }
  } else {
    logprint("Texture '%S' not found in shader '%S'\n", name, shader->file_name);
  }
}

internal void set_sampler(String8 name, Sampler *sampler) {
  R_D3D11_State *d3d = r_d3d11_state();
  Shader *shader = current_shader;
  Shader_Bindings *bindings = shader->bindings;
  Shader_Loc *loc = nullptr;
  for (int i = 0; i < bindings->sampler_locations.count; i++) {
    loc = &bindings->sampler_locations[i];
    if (str8_equal(loc->name, name)) {
      break;
    }
    loc = nullptr;
  }

  if (loc) {
    if (loc->vertex != -1) {
      d3d->device_context->VSSetSamplers(loc->vertex, 1, &sampler->resource);
    }
    if (loc->pixel != -1) {
      d3d->device_context->PSSetSamplers(loc->pixel, 1, &sampler->resource);
    }
    if (loc->geo != -1) {
      d3d->device_context->PSSetSamplers(loc->geo, 1, &sampler->resource);
    }
  } else {
    logprint("Sampler '%S' not found in shader '%S'\n", name, shader->file_name);
  }
}

internal Shader_Loc *find_shader_texture(Shader *shader, String8 name) {
  Shader_Loc *result = nullptr;
  Shader_Bindings *bindings = shader->bindings;
  Shader_Loc *loc = nullptr;
  for (int i = 0; i < bindings->texture_locations.count; i++) {
    Shader_Loc *loc = &bindings->texture_locations[i];
    if (str8_equal(name, loc->name)) {
      result = loc;
      break;
    }
  }
  return result;
}

internal inline void set_texture(Shader_Loc *loc, Texture *texture) {
  if (loc) {
    R_D3D11_State *d3d = r_d3d11_state();
    ID3D11ShaderResourceView *view = texture ? texture->view : d3d->fallback_tex->view;
    if (loc->vertex != -1) {
      d3d->device_context->VSSetShaderResources(loc->vertex, 1, &view);
    }
    if (loc->pixel != -1) {
      d3d->device_context->PSSetShaderResources(loc->pixel, 1, &view);
    }
    if (loc->geo != -1) {
      d3d->device_context->PSSetShaderResources(loc->geo, 1, &view);
    }
  }
}

internal inline void set_texture(String8 name, Texture *texture) {
  R_D3D11_State *d3d = r_d3d11_state();
  Shader *shader = current_shader;
  Shader_Loc *loc = find_shader_texture(shader, name);
  if (loc) {
    set_texture(loc, texture);
  } else {
    logprint("Texture '%S' not found in shader '%S'\n", name, shader->file_name);
  }
}

internal inline void apply_constants(Shader *shader = current_shader) {
  Shader_Bindings *bindings = shader->bindings;
  for (int i = 0; i < bindings->uniforms.count; i++) {
    Shader_Uniform *uniform = bindings->uniforms[i];
    if (uniform->dirty) {
      write_uniform_buffer(uniform->buffer, uniform->memory, uniform->size);
      uniform->dirty = 0;
    }
  }
}

internal inline Shader_Constant *find_shader_constant(Shader *shader, String8 name) {
  Shader_Constant *constant = nullptr;
  Shader_Bindings *bindings = shader->bindings;
  for (int i = 0; i < bindings->constants.count; i++) {
    constant = &bindings->constants[i];
    if (str8_equal(constant->name, name)) {
      break;
    }
    constant = nullptr;
  }

  if (!constant) {
    logprint("Constant '%S' not found in shader: '%S'\n", name, shader->file_name); 
  }
  return constant;
}

internal inline void write_shader_constant(Shader *shader, String8 name, void *ptr, u32 size) {
  Shader_Constant *constant = find_shader_constant(shader, name);
  if (constant) {
    Shader_Uniform *uniform = constant->uniform;
    MemoryCopy((void *)((uintptr_t)uniform->memory + constant->offset), ptr, size);
    uniform->dirty = 1;
  }
}

internal inline void write_shader_constant(Shader_Constant *constant, void *ptr, u32 size) {
  if (constant) {
    Shader_Uniform *uniform = constant->uniform;
    MemoryCopy((void *)((uintptr_t)uniform->memory + constant->offset), ptr, size);
    uniform->dirty = 1;
  }
}

internal inline void set_constant(Shader_Constant *constant, Matrix4 v) {
  if (!constant) return;
  Shader_Uniform *uniform = constant->uniform;
  write_shader_constant(constant, &v, sizeof(v));
}

internal inline void set_constant(Shader_Constant *constant, f32 v) {
  if (!constant) return;
  Shader_Uniform *uniform = constant->uniform;
  write_shader_constant(constant, &v, sizeof(v));
}

internal inline void set_constant(Shader_Constant *constant, Vector2 v) {
  if (!constant) return;
  Shader_Uniform *uniform = constant->uniform;
  write_shader_constant(constant, &v, sizeof(v));
}

internal inline void set_constant(Shader_Constant *constant, Vector3 v) {
  if (!constant) return;
  Shader_Uniform *uniform = constant->uniform;
  write_shader_constant(constant, &v, sizeof(v));
}

internal inline void set_constant(Shader_Constant *constant, Vector4 v) {
  if (!constant) return;
  Shader_Uniform *uniform = constant->uniform;
  write_shader_constant(constant, &v, sizeof(v));
}

internal inline void set_constant_array(String8 name, void *elems, uint size, uint count, Shader *shader = current_shader) {
  write_shader_constant(shader, name, elems, size * count);
}

internal inline void set_constant(String8 name, Matrix4 v, Shader *shader = current_shader) {
  write_shader_constant(shader, name, &v, sizeof(v));
}

internal inline void set_constant(String8 name, Vector4 v, Shader *shader = current_shader) {
  write_shader_constant(shader, name, &v, sizeof(v));
}

internal inline void set_constant(String8 name, Vector3 v, Shader *shader = current_shader) {
  write_shader_constant(shader, name, &v, sizeof(v));
}

internal inline void set_constant(String8 name, Vector2 v, Shader *shader = current_shader) {
  write_shader_constant(shader, name, &v, sizeof(v));
}

internal inline void set_constant(String8 name, f32 v, Shader *shader = current_shader) {
  write_shader_constant(shader, name, &v, sizeof(v));
}

internal inline void set_constant(String8 name, int v, Shader *shader = current_shader) {
  write_shader_constant(shader, name, &v, sizeof(v));
}

internal void set_rasterizer(Rasterizer *rasterizer) {
  R_D3D11_State *d3d = r_d3d11_state();
  d3d->device_context->RSSetState(rasterizer->resource);
}

internal void set_blend_state(Blend_State *blend_state) {
  R_D3D11_State *d3d = r_d3d11_state();
  d3d->device_context->OMSetBlendState(blend_state->resource, NULL, 0xFFFFFFF);
}

internal void reset_blend_state() {
  set_blend_state(blend_state_default);
}

internal void set_depth_state(Depth_State *depth_state) {
  R_D3D11_State *d3d = r_d3d11_state();
  d3d->device_context->OMSetDepthStencilState(depth_state->resource, 0);
}

internal void set_render_target(Render_Target *render_target) {
  R_D3D11_State *d3d = r_d3d11_state();
  d3d->device_context->OMSetRenderTargets(1, (ID3D11RenderTargetView **)&render_target->render_target_view, (ID3D11DepthStencilView *)render_target->depth_stencil_view);
}

internal void reset_render_target() {
  R_D3D11_State *d3d = r_d3d11_state();
  set_render_target(d3d->default_render_target);
}

internal void set_viewport(f32 left, f32 top, f32 right, f32 bottom) {
  R_D3D11_State *d3d = r_d3d11_state();
  D3D11_VIEWPORT viewport;
  viewport.TopLeftX = left;
  viewport.TopLeftY = top;
  viewport.Width = right - left;
  viewport.Height = bottom - top;
  viewport.MinDepth = 0;
  viewport.MaxDepth = 1;
  d3d->device_context->RSSetViewports(1, &viewport); 
}

internal void reset_viewport() {
  R_D3D11_State *d3d = r_d3d11_state();
  set_viewport(0, 0, (f32)d3d->window_dimension.x, (f32)d3d->window_dimension.y);
}

internal inline void grow_immediate_vertices(u64 min_capacity) {
  u64 new_capacity = immediate_vertices.byte_capacity;
  while (new_capacity < min_capacity) {
    new_capacity = new_capacity * 8 / 5 + 1; // golden ratio 1.6
  }
  immediate_vertices.data = realloc(immediate_vertices.data, new_capacity);
  immediate_vertices.byte_capacity = new_capacity;
}

internal inline void ensure_immediate_capacity(size_t size) {
  if (immediate_vertices.byte_count + size >= immediate_vertices.byte_capacity) {
    grow_immediate_vertices(immediate_vertices.byte_count + size);
  }
}

internal inline void *get_immediate_ptr() {
  return (void *)((uintptr_t)immediate_vertices.data + immediate_vertices.byte_count);
}

internal inline void put_immediate_vertex(void *v, size_t size) {
  ensure_immediate_capacity(size);
  void *ptr = get_immediate_ptr();
  MemoryCopy(ptr, v, size);
  immediate_vertices.byte_count += size;
}

internal inline void immediate_vertex(Vector3 v, Vector4 c) {
  Vertex_3D vertex;
  vertex.position = v;
  vertex.color = c;
  put_immediate_vertex(&vertex, sizeof(Vertex_3D));
}

internal void immediate_flush() {
  if (immediate_vertices.byte_count == 0) {
    return;
  }

  apply_constants();

  R_D3D11_State *d3d = r_d3d11_state();

  UINT format_size = 0;
  if (current_shader == shader_basic) {
    format_size = sizeof(Vertex_3D);
  } else if (current_shader == shader_argb_texture) {
    format_size = sizeof(Vertex_ARGB);
  } else {
    Assert(0);
  }
  UINT offset = 0;
  UINT vertices_count = (UINT)(immediate_vertices.byte_count / (u64)format_size);

  ID3D11Buffer *vertex_buffer = make_vertex_buffer(immediate_vertices.data, immediate_vertices.byte_count, 1);

  d3d->device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &format_size, &offset);

  d3d->device_context->Draw(vertices_count, 0);

  vertex_buffer->Release();
  immediate_vertices.byte_count = 0;
}

internal void immediate_begin() {
  immediate_flush();
}

internal void draw_wireframe_mesh(Triangle_Mesh *mesh) {
  R_D3D11_State *d3d = r_d3d11_state();

  UINT stride = sizeof(Vertex_XNCUU), offset = 0;
  d3d->device_context->IASetVertexBuffers(0, 1, &mesh->vertex_buffer, &stride, &offset);
  d3d->device_context->Draw((UINT)mesh->vertices.count, 0);
}

internal void draw_mesh(Triangle_Mesh *mesh) {
  R_D3D11_State *d3d = r_d3d11_state();

  UINT stride = sizeof(Vertex_XNCUU), offset = 0;
  d3d->device_context->IASetVertexBuffers(0, 1, &mesh->vertex_buffer, &stride, &offset);

  for (int i = 0; i < mesh->triangle_list_info.count; i++) {
    Triangle_List_Info triangle_list_info = mesh->triangle_list_info[i];

    Material *material = mesh->materials[triangle_list_info.material_index];

    set_texture(current_shader->bindings->diffuse_texture, material->texture);

    d3d->device_context->Draw(triangle_list_info.vertices_count, triangle_list_info.first_index);
  }
}

internal void draw_mirror_reflection() {
  Entity_Manager *manager = get_entity_manager();
  Guy *guy = manager->by_type._Guy[0];
  Mirror *mirror = static_cast<Mirror*>(lookup_entity(guy->mirror_id));
  if (!mirror) return;

  Camera camera = get_game_state()->camera;

  // immediate_begin();

  // set_shader(shader_mesh);

  // set_blend_state(blend_state_alpha);
  // set_depth_state(DEPTH_STATE_DEFAULT);

  f32 dist = Abs(distance(mirror->position, guy->position));
  Vector3 position = mirror->position;

  Vector3 last_dir = get_nearest_axis(guy->forward);
  while (dist > 0) {
    Vector3 outgoing = (Abs(last_dir.x) > Abs(last_dir.z)) ? mirror->reflection_vectors[1] : mirror->reflection_vectors[0];
    last_dir = outgoing;

    f32 mirror_dist = 0;
    Reflection_Node *next_node = (Abs(outgoing.x) > Abs(outgoing.z)) ? mirror->node->reflect_x : mirror->node->reflect_z;
    if (next_node) mirror_dist = Abs(distance(next_node->mirror->position, mirror->position));

    if (!next_node || dist < mirror_dist) {
      position = mirror->position + outgoing * dist;
      dist = 0;
    } else {
      position = mirror->position + outgoing * mirror_dist;
      dist -= mirror_dist;
    }

    if (next_node) mirror = next_node->mirror;
  }

  //Draw clone
  // {
  //   Vector3 reflect_position = position;
  //   Matrix4 rotation_matrix = rotate_rh(guy->theta, camera.up);
  //   Matrix4 world_matrix = translate(reflect_position) * translate(guy->offset) * rotation_matrix;
  //   Matrix4 xform = camera.transform * world_matrix;

  //   set_shader(shader_mesh);


  //   set_constant(str_lit("xform"), xform);
  //   set_constant(str_lit("color"), Vector4(.4f, .4f, .4f, 1.f));
  //   apply_constants(); 

  //   set_sampler(str_lit("diffuse_sampler"), SAMPLER_STATE_LINEAR);

  //   draw_mesh(guy->mesh);
  // }
}

internal void init_draw() {
  R_D3D11_State *d3d = r_d3d11_state();
  shadow_map = make_depth_map(2048, 2048);

  water_plane_mesh = gen_plane_mesh(Vector2(1000.0f, 1000.0f));

  reflection_render_target = make_render_target(d3d->window_dimension.x, d3d->window_dimension.y, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT);
  refraction_render_target = make_render_target(d3d->window_dimension.x, d3d->window_dimension.y, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT);
  refraction_depth_map = make_depth_map(d3d->window_dimension.x, d3d->window_dimension.y);

  String8 skybox_file_names[6] = {
    str_lit("data/textures/interstellar_skybox/xpos.png"),
    str_lit("data/textures/interstellar_skybox/xneg.png"),
    str_lit("data/textures/interstellar_skybox/ypos.png"),
    str_lit("data/textures/interstellar_skybox/yneg.png"),
    str_lit("data/textures/interstellar_skybox/zpos.png"),
    str_lit("data/textures/interstellar_skybox/zneg.png"),
    // str_lit("data/textures/skybox2/right.png"),
    // str_lit("data/textures/skybox2/left.png"),
    // str_lit("data/textures/skybox2/top.png"),
    // str_lit("data/textures/skybox2/bottom.png"),
    // str_lit("data/textures/skybox2/front.png"),
    // str_lit("data/textures/skybox2/back.png"),
  };
  skybox_texture = create_texture_cube(skybox_file_names);
  skybox_mesh = gen_cube_mesh();

  water_dudv_texture = create_texture_from_file(str_lit("data/textures/waterDUDV.png"), 0);
  water_normal_map = create_texture_from_file(str_lit("data/textures/normalMap.png"), 0);
}

internal void draw_scene() {
  local_persist bool first_call = true;
  if (first_call) {
    first_call = false;
    init_draw();
  }

  R_D3D11_State *d3d = r_d3d11_state();

  Entity_Manager *manager = get_entity_manager();


  Game_State *game_state = get_game_state();
  Editor *editor = get_editor();
  World *world = get_world();

  Camera camera;
  if (game_state->editing) {
    camera = editor->camera;
  } else {
    camera = game_state->camera;
  }

  Matrix4 projection = ortho_rh_zo(0.0f, (f32)d3d->window_dimension.x, 0.f, (f32)d3d->window_dimension.y);

  // @Note Shadow mapping
  Sun *sun = manager->by_type._Sun.count ? manager->by_type._Sun[0] : 0;
  if (sun) {
    set_viewport(0, 0, (f32)shadow_map->texture->width, (f32)shadow_map->texture->height);
    set_depth_state(depth_state_default);
    set_rasterizer(rasterizer_shadow_map);
    d3d->device_context->OMSetRenderTargets(0, nullptr, shadow_map->depth_stencil_view);
    d3d->device_context->ClearDepthStencilView(shadow_map->depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

    set_shader(shader_shadow_map);

    Shader_Constant *c_world = find_shader_constant(shader_shadow_map, str_lit("world"));
    Shader_Constant *c_light_vp = find_shader_constant(shader_shadow_map, str_lit("light_view_projection"));

    set_constant(c_light_vp, sun->light_space_matrix);

    for (Entity *entity : manager->entities) {
      Triangle_Mesh *mesh = entity->mesh;
      if (entity->animation_state) continue;
      if (!mesh) continue;

      //@Todo Set Transform
      Matrix4 rotation_matrix = rotate_rh(entity->theta, camera.up);
      Matrix4 world_matrix = translate(entity->visual_position) * translate(entity->offset) *rotation_matrix;

      set_constant(c_world, world_matrix);
      apply_constants();

      UINT stride = sizeof(Vertex_XNCUU), offset = 0;
      d3d->device_context->IASetVertexBuffers(0, 1, &mesh->vertex_buffer, &stride, &offset);
      d3d->device_context->Draw((UINT)mesh->vertices.count, 0);
    }

    set_shader(shader_skinned_shadow_map);

    for (Entity *entity : manager->entities) {
      Animation_State *animation_state = entity->animation_state;
      Triangle_Mesh *mesh = entity->mesh;
      if (!animation_state) continue;

      Matrix4 rotation_matrix = rotate_rh(entity->theta, camera.up);
      Matrix4 world_matrix = translate(entity->visual_position) * translate(entity->offset) *rotation_matrix;
      set_constant(str_lit("world"), world_matrix);
      set_constant_array(str_lit("bone_matrices"), animation_state->bone_transforms, sizeof(Matrix4), MAX_BONES);
      set_constant(str_lit("light_view_projection"), sun->light_space_matrix);
      apply_constants();

      uint stride = sizeof(Vertex_Skinned), offset = 0;
      d3d->device_context->IASetVertexBuffers(0, 1, &mesh->skinned_vertex_buffer, &stride, &offset);
      d3d->device_context->Draw((UINT)mesh->skinned_vertices.count, 0);
    }

    reset_viewport();
  }

  set_depth_state(depth_state_default);
  set_blend_state(blend_state_default);
  set_rasterizer(rasterizer_default);
  set_render_target(d3d->default_render_target);

  draw_world(world, camera);

  if (!game_state->editing) {
    draw_mirror_reflection();
  }

  set_depth_state(depth_state_no_write);

  set_blend_state(blend_state_additive);

  for (Particle_Source *source : manager->by_type._Particle_Source) {
    Particles *particles = &source->particles;
    if (particles->count == 0) continue;

    set_shader(shader_particle);
    set_texture(str_lit("diffuse_texture"), sun_icon_texture);
    set_sampler(str_lit("diffuse_sampler"), sampler_linear);
    d3d->device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    set_constant(str_lit("camera_position"), camera.origin);
    set_constant(str_lit("to_view_projection"), camera.transform);
    apply_constants();

    Particle_Pt *points = new Particle_Pt[particles->count];
    for (int i = 0; i < particles->count; i++) {
      Particle_Pt *pt = points + i;
      pt->position = particles->positions[i];
      pt->color    = particles->colors[i];
      pt->scale    = particles->scales[i];
    }

    ID3D11Buffer *vertex_buffer = make_vertex_buffer(points, particles->count, sizeof(Particle_Pt));
    uint stride = sizeof(Particle_Pt), offset = 0;
    d3d->device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
    d3d->device_context->Draw(particles->count, 0);
    vertex_buffer->Release();
    delete [] points;
  }

  set_depth_state(depth_state_default);

  reset_blend_state();
  reset_primitives();

  if (0) {
    set_shader(shader_color_wheel);
    set_constant(str_lit("projection"), projection);
    set_constant(str_lit("radius"), 250.0f);
    set_constant(str_lit("center"), Vector2(400.0f, 400.0f));
    apply_constants();

    Rect rect = make_rect(-0.5f, -0.5f, 1.0f, 1.0f);
    Vector2 vertices[6];
    vertices[0] = Vector2(rect.x0, rect.y0);
    vertices[1] = Vector2(rect.x1, rect.y0);
    vertices[2] = Vector2(rect.x1, rect.y1);
    vertices[3] = Vector2(rect.x0, rect.y0);
    vertices[4] = Vector2(rect.x1, rect.y1);
    vertices[5] = Vector2(rect.x0, rect.y1);
    ID3D11Buffer *vertex_buffer = make_vertex_buffer(vertices, 6, sizeof(Vector2));
    uint stride = sizeof(Vector2), offset = 0;
    d3d->device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
    d3d->device_context->Draw(6, 0);
    vertex_buffer->Release();
  }

  // Shadow map
  if (display_shadow_map) {
    Vector2 dim = get_viewport()->dimension;
    immediate_begin();
    set_shader(shader_argb_texture);
    set_sampler(str_lit("diffuse_sampler"), sampler_point);
    set_depth_state(depth_state_disable);
    // set_blend_state(blend_state_alpha);
    set_rasterizer(rasterizer_text);

    immediate_quad(shadow_map->texture,
      Vector2(0.0f, 0.0f), Vector2(dim.x, 0.0f), Vector2(dim.x, dim.y), Vector2(0.0f, dim.y),
      Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 0.0f),
      Vector4(1.0f, 1.0f, 1.0f, 1.0f));
    reset_texture(str_lit("diffuse_texture"));
  }
}

internal void draw_skybox(Camera *camera) {
  R_D3D11_State *d3d = r_d3d11_state();
  set_shader(shader_skybox);

  set_rasterizer(rasterizer_default);
  set_depth_state(depth_state_skybox);

  Matrix4 skybox_view = camera->view_matrix;
  skybox_view.columns[3] = Vector4();
  Matrix4 xform = camera->projection_matrix * skybox_view;

  set_texture(str_lit("skybox_texture"), skybox_texture);
  set_sampler(str_lit("skybox_sampler"), sampler_skybox);

  set_constant(str_lit("xform"), xform);
  apply_constants();

  uint stride = sizeof(Vertex_XNCUU), offset = 0;
  d3d->device_context->IASetVertexBuffers(0, 1, &skybox_mesh->vertex_buffer, &stride, &offset);
  d3d->device_context->Draw((uint)skybox_mesh->vertices.count, 0);

  set_depth_state(depth_state_default);
  set_rasterizer(rasterizer_default);
}

internal void draw_world(World *world, Camera camera) {
  Entity_Manager *manager = get_entity_manager();

  R_D3D11_State *d3d = r_d3d11_state();

  Sun *sun = manager->by_type._Sun.count ? manager->by_type._Sun[0] : 0;
  Matrix4 light_space = sun ? sun->light_space_matrix : make_matrix4(1.0f);
  Vector3 light_direction = sun ? sun->light_direction : Vector3(0, 0, 0);
  Vector4 light_color = sun ? sun->override_color : make_vec4(1.0f);

  set_constant(str_lit("light_direction"), light_direction, shader_entity);
  set_constant(str_lit("light_view_projection"), light_space, shader_entity);
  set_constant(str_lit("light_color"), light_color, shader_entity);
  set_constant(str_lit("clip_plane"), Vector4(0, -1, 0, 10000), shader_entity);
  apply_constants(shader_entity);

  set_constant(str_lit("light_direction"), light_direction, shader_skinned);
  set_constant(str_lit("light_view_projection"), light_space, shader_skinned);
  set_constant(str_lit("light_color"), light_color, shader_skinned);
  apply_constants(shader_skinned);


  Auto_Array<R_Point_Light> point_lights;
  point_lights.reserve(manager->by_type._Point_Light.count);
  for (int i = 0; i < manager->by_type._Point_Light.count; i++) {
    Point_Light *point_light = manager->by_type._Point_Light[i];
    R_Point_Light pt;
    pt.position = point_light->position;
    pt.range = point_light->range;
    pt.color = point_light->override_color;
    pt.att = point_light->attenuation;
    point_lights.push(pt);
  }

  set_constant(str_lit("point_light_count"), (int)point_lights.count, shader_entity);
  if (point_lights.count) {
    set_constant_array(str_lit("point_lights"), point_lights.data, sizeof(R_Point_Light), (int)point_lights.count, shader_entity);
  }
  apply_constants(shader_entity);

  set_shader(shader_entity);
  set_texture(str_lit("shadow_map"), shadow_map->texture);
  set_sampler(str_lit("diffuse_sampler"), sampler_anisotropic);
  set_sampler(str_lit("shadow_sampler"), sampler_point);

  Shader_Constant *c_eye_pos = find_shader_constant(shader_entity, str_lit("eye_pos"));
  Shader_Constant *c_use_tint = find_shader_constant(shader_entity, str_lit("use_override_color"));
  Shader_Constant *c_tint = find_shader_constant(shader_entity, str_lit("override_color"));

  for (Entity *entity : manager->entities) {
    if (entity->kind == ENTITY_GUY) continue; //temp

    Triangle_Mesh *mesh = entity->mesh;
    if (!mesh) continue;

    Matrix4 rotation_matrix = rotate_rh(entity->theta, camera.up);
    Matrix4 world_matrix = translate(entity->visual_position) * translate(entity->offset) *rotation_matrix;
    Matrix4 xform = camera.transform * world_matrix;
    set_constant(shader_entity->bindings->xform, xform);
    set_constant(shader_entity->bindings->world, world_matrix);
    set_constant(c_eye_pos, camera.origin);
    set_constant(c_use_tint, (float)entity->use_override_color);
    set_constant(c_tint, entity->override_color);
    apply_constants();

    draw_mesh(mesh);
  }

  for (Guy *guy : manager->by_type._Guy) {
    Animation_State *animation_state = guy->animation_state;
    Triangle_Mesh *mesh = guy->mesh;

    if (!animation_state) continue;

    set_shader(shader_skinned);

    set_sampler(str_lit("diffuse_sampler"), sampler_linear);
    set_sampler(str_lit("shadow_sampler"), sampler_linear);
    set_texture(str_lit("shadow_map"), shadow_map->texture);

    Matrix4 rotation_matrix = rotate_rh(guy->theta, camera.up);
    Matrix4 world_matrix = translate(guy->visual_position) * translate(guy->offset) * rotation_matrix;
    Matrix4 xform = camera.transform * world_matrix;

    Vector4 color = guy->use_override_color ? make_vec4(1.0f) : guy->override_color;
    set_constant(str_lit("xform"), xform);
    set_constant(str_lit("world"), world_matrix);
    set_constant(str_lit("color"), color);
    set_constant_array(str_lit("bone_matrices"), animation_state->bone_transforms, sizeof(Matrix4), MAX_BONES);
    apply_constants();

    uint stride = sizeof(Vertex_Skinned), offset = 0;
    d3d->device_context->IASetVertexBuffers(0, 1, &mesh->skinned_vertex_buffer, &stride, &offset);

    for (int i = 0; i < mesh->triangle_list_info.count; i++) {
      Triangle_List_Info triangle_list_info = mesh->triangle_list_info[i];

      Material *material = mesh->materials[triangle_list_info.material_index];
      set_texture(str_lit("diffuse_texture"), material->texture);

      d3d->device_context->Draw(triangle_list_info.vertices_count, triangle_list_info.first_index);
    }
  }

  reset_texture(str_lit("shadow_map"));


  // Water Render
  {
    Vector3 water_position = Vector3(0, -0.1f, 0);
    f32 wave_speed = 0.03f;
    move_factor += wave_speed * get_frame_delta();
    move_factor = fmodf(move_factor, 1.0f);

    Camera mirrored = camera;
    f32 distance = 2.0f * (camera.origin.y - water_position.y);
    mirrored.origin.y -= distance;
    mirrored.update_euler_angles(camera.yaw, -camera.pitch);
    update_camera_matrix(&mirrored);

    set_render_target(reflection_render_target);
    clear_render_target(reflection_render_target, 0, 0, 0, 0);
    d3d->device_context->ClearDepthStencilView(reflection_render_target->depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

    set_rasterizer(rasterizer_default);

    set_shader(shader_entity);

    Shader_Constant *c_eye_pos = find_shader_constant(shader_entity, str_lit("eye_pos"));
    Shader_Constant *c_use_tint = find_shader_constant(shader_entity, str_lit("use_override_color"));
    Shader_Constant *c_tint = find_shader_constant(shader_entity, str_lit("override_color"));

    // reflection
    set_constant(str_lit("clip_plane"), Vector4(0, 1, 0, -water_position.y + 0.1f));

    for (Entity *e : manager->entities) {
      if (e->kind == ENTITY_GUY) continue; //temp
      Triangle_Mesh *mesh = e->mesh;
      if (!mesh) continue;

      Matrix4 world_matrix = translate(e->visual_position) * translate(e->offset) * rotate_rh(e->theta, vec3_up());
      Matrix4 xform = mirrored.transform * world_matrix;
      set_constant(shader_entity->bindings->xform, xform);
      set_constant(shader_entity->bindings->world, world_matrix);
      set_constant(c_eye_pos, mirrored.origin);
      set_constant(c_use_tint, (f32)e->use_override_color);
      set_constant(c_tint, e->override_color);
      apply_constants();
      draw_mesh(mesh);
    }

    set_shader(shader_skinned);
    for (Guy *guy : manager->by_type._Guy) {
      Animation_State *animation_state = guy->animation_state;
      if (!animation_state) continue;
      Triangle_Mesh *mesh = guy->mesh;

      set_sampler(str_lit("diffuse_sampler"), sampler_linear);
      set_sampler(str_lit("shadow_sampler"), sampler_linear);
      set_texture(str_lit("shadow_map"), shadow_map->texture);

      Matrix4 world_matrix = translate(guy->visual_position) * translate(guy->offset) * rotate_rh(guy->theta, vec3_up());
      Matrix4 xform = mirrored.transform * world_matrix;
      Vector4 color = guy->use_override_color ? make_vec4(1.0f) : guy->override_color;
      set_constant(str_lit("xform"), xform);
      set_constant(str_lit("world"), world_matrix);
      set_constant(str_lit("color"), color);
      set_constant_array(str_lit("bone_matrices"), animation_state->bone_transforms, sizeof(Matrix4), MAX_BONES);
      apply_constants();

      uint stride = sizeof(Vertex_Skinned), offset = 0;
      d3d->device_context->IASetVertexBuffers(0, 1, &mesh->skinned_vertex_buffer, &stride, &offset);

      for (int i = 0; i < mesh->triangle_list_info.count; i++) {
        Triangle_List_Info triangle_list_info = mesh->triangle_list_info[i];
        Material *material = mesh->materials[triangle_list_info.material_index];
        set_texture(str_lit("diffuse_texture"), material->texture);
        d3d->device_context->Draw(triangle_list_info.vertices_count, triangle_list_info.first_index);
      }
    }

    // draw_skybox(&mirrored);

    set_shader(shader_entity);

    // set_render_target(refraction_render_target);
    d3d->device_context->OMSetRenderTargets(1, &refraction_render_target->render_target_view, refraction_depth_map->depth_stencil_view);
    clear_render_target(refraction_render_target, 0, 0, 0, 0);

    set_constant(str_lit("clip_plane"), Vector4(0, -1, 0, water_height));
    d3d->device_context->ClearDepthStencilView(refraction_depth_map->depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

    for (Entity *e : manager->by_type._Inanimate) {
      Triangle_Mesh *mesh = e->mesh;

      Matrix4 world_matrix = translate(e->visual_position) * translate(e->offset) * rotate_rh(e->theta, vec3_up());
      Matrix4 xform = camera.transform * world_matrix;
      set_constant(str_lit("eye_pos"), mirrored.origin);
      set_constant(str_lit("xform"), xform);
      set_constant(str_lit("world"), world_matrix);
      set_constant(str_lit("use_override_color"), (f32)e->use_override_color);
      set_constant(str_lit("override_color"), e->override_color);
      apply_constants();

      draw_mesh(mesh);
    }

    reset_render_target();

    // draw water
    set_shader(shader_water);
    set_rasterizer(rasterizer_default);
    set_blend_state(blend_state_alpha);

    set_sampler(str_lit("main_sampler"), sampler_linear);
    set_texture(str_lit("reflection_texture"), reflection_render_target->texture);
    set_texture(str_lit("refraction_texture"), refraction_render_target->texture);
    set_texture(str_lit("dudv_map"), water_dudv_texture);
    set_texture(str_lit("normal_map"), water_normal_map);
    set_texture(str_lit("depth_map"), refraction_depth_map->texture);

    Matrix4 world_matrix = translate(water_position);
    Matrix4 xform = camera.transform * world_matrix;
    set_constant(str_lit("xform"), xform);
    set_constant(str_lit("world"), world_matrix);
    set_constant(str_lit("eye_position"), camera.origin);
    set_constant(str_lit("tiling"), 0.166f);
    set_constant(str_lit("wave_strength"), 0.02f);
    set_constant(str_lit("move_factor"), move_factor);
    apply_constants();

    uint stride = sizeof(Vertex_XNCUU), offset = 0;
    d3d->device_context->IASetVertexBuffers(0, 1, &water_plane_mesh->vertex_buffer, &stride, &offset);
    d3d->device_context->Draw((uint)water_plane_mesh->vertices.count, 0);

    reset_texture(str_lit("reflection_texture"));
    reset_texture(str_lit("refraction_texture"));
    reset_texture(str_lit("depth_map"));
    set_rasterizer(rasterizer_default);
    set_blend_state(blend_state_default);
  }

  // draw_skybox(&camera);
}

internal inline ARGB argb_from_vector(Vector4 color) {
  ARGB argb = 0;
  argb |= (u32)(color.w*255);
  argb |= (u32)(color.x*255) << 8;
  argb |= (u32)(color.y*255) << 16;
  argb |= (u32)(color.z*255) << 24;
  return argb;
}

internal void imm_quad_vertex(Vector2 p, Vector2 uv, ARGB argb) {
  Vertex_ARGB v;
  v.position = p;
  v.uv = uv;
  v.argb = argb;
  put_immediate_vertex(&v, sizeof(Vertex_ARGB));
}

internal void immediate_quad(Texture *texture, Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3, Vector2 uv0, Vector2 uv1, Vector2 uv2, Vector2 uv3, Vector4 color) {
  set_texture(str_lit("diffuse_texture"), texture);

  ARGB argb = argb_from_vector(color);
  imm_quad_vertex(p0, uv0, argb);
  imm_quad_vertex(p1, uv1, argb);
  imm_quad_vertex(p2, uv2, argb);
  imm_quad_vertex(p0, uv0, argb);
  imm_quad_vertex(p2, uv2, argb);
  imm_quad_vertex(p3, uv3, argb);

  immediate_flush();
}

internal void immediate_plane(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, Vector4 color) {
  immediate_vertex(p0, color);
  immediate_vertex(p1, color);
  immediate_vertex(p2, color);

  immediate_vertex(p0, color);
  immediate_vertex(p2, color);
  immediate_vertex(p3, color);
}

internal void immediate_rectangle(Vector3 position, Vector3 size, Vector4 color) {
    Vector3 half = 0.5f * size;
    Vector3 p0 = Vector3(position.x,          position.y,          position.z + size.z);
    Vector3 p1 = Vector3(position.x + size.x, position.y,          position.z + size.z);
    Vector3 p2 = Vector3(position.x + size.x, position.y + size.y, position.z + size.z);
    Vector3 p3 = Vector3(position.x,          position.y + size.y, position.z + size.z);
    Vector3 p4 = Vector3(position.x + size.x, position.y,          position.z);
    Vector3 p5 = Vector3(position.x,          position.y,          position.z);
    Vector3 p6 = Vector3(position.x,          position.y + size.y, position.z);
    Vector3 p7 = Vector3(position.x + size.x, position.y + size.y, position.z);

    // front
    immediate_vertex(p0, color);
    immediate_vertex(p1, color);
    immediate_vertex(p2, color);
    immediate_vertex(p0, color);
    immediate_vertex(p2, color);
    immediate_vertex(p3, color);
    // back
    immediate_vertex(p4, color);
    immediate_vertex(p5, color);
    immediate_vertex(p6, color);
    immediate_vertex(p4, color);
    immediate_vertex(p6, color);
    immediate_vertex(p7, color);
    // left
    immediate_vertex(p5, color);
    immediate_vertex(p0, color);
    immediate_vertex(p3, color);
    immediate_vertex(p5, color);
    immediate_vertex(p3, color);
    immediate_vertex(p6, color);
    // right
    immediate_vertex(p1, color);
    immediate_vertex(p4, color);
    immediate_vertex(p7, color);
    immediate_vertex(p1, color);
    immediate_vertex(p7, color);
    immediate_vertex(p2, color);
    // top
    immediate_vertex(p3, color);
    immediate_vertex(p2, color);
    immediate_vertex(p7, color);
    immediate_vertex(p3, color);
    immediate_vertex(p7, color);
    immediate_vertex(p6, color);
    // bottom
    immediate_vertex(p1, color);
    immediate_vertex(p0, color);
    immediate_vertex(p5, color);
    immediate_vertex(p1, color);
    immediate_vertex(p5, color);
    immediate_vertex(p4, color);
}

internal void immediate_cube(Vector3 center, f32 size, Vector4 color) {
  immediate_rectangle(center, Vector3(size, size, size), color);
}
