global Shader *shader_basic;
global Shader *shader_mesh;
global Shader *shader_entity;
global Shader *shader_rect;
global Shader *shader_picker;
global Shader *shader_shadow_map;
global Shader *shader_argb_texture;

global Shader *current_shader;
global Vertex_List immediate_vertices;

global Shadow_Map *shadow_map;

global Texture *sun_icon_texture;
global Texture *eye_of_horus_texture;

global Triangle_Mesh *water_plane_mesh;

global ID3D11RasterizerState *rasterizer_cull_back;
global ID3D11RasterizerState *rasterizer_cull_front;
global ID3D11RasterizerState *rasterizer_wireframe;

internal Shadow_Map *make_shadow_map(int width, int height) {
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

  Shadow_Map *map = new Shadow_Map();
  map->tex2d = tex2d;
  map->depth_stencil_view = depth_stencil_view;
  map->texture = texture;
  return map;
}

internal void set_shader(Shader *shader) {
  R_D3D11_State *d3d = r_d3d11_state();
  if (shader != current_shader) {
    current_shader = shader;

    ID3D11VertexShader *vs = nullptr;
    ID3D11PixelShader *ps = nullptr;
    ID3D11InputLayout *ilay = nullptr;
    if (shader) {
      vs = shader->vertex_shader;
      ps = shader->pixel_shader;
      ilay = shader->input_layout;
    }
    d3d->device_context->VSSetShader(vs, nullptr, 0);
    d3d->device_context->PSSetShader(ps, nullptr, 0);
    d3d->device_context->IASetInputLayout(ilay);
  }
}

internal void reset_texture(String8 name) {
  R_D3D11_State *d3d = r_d3d11_state();

  Shader_Bindings *bindings = current_shader->bindings;
  Shader_Loc *loc = nullptr;
  for (int i = 0; i < bindings->texture_locations.count; i++) {
    loc = &bindings->texture_locations[i];
    if (str8_equal(name, loc->name)) {
      break;
    }
    loc = nullptr;
  }

  if (!loc) return;

  ID3D11ShaderResourceView *null_srv[1] = {nullptr};
  if (loc->vertex != -1) {
    d3d->device_context->VSSetShaderResources(loc->vertex, 1, null_srv);
  }
  if (loc->pixel != -1) {
    d3d->device_context->PSSetShaderResources(loc->pixel, 1, null_srv);
  }
}

internal void set_texture(String8 name, Texture *texture) {
  R_D3D11_State *d3d = r_d3d11_state();

  Shader_Bindings *bindings = current_shader->bindings;
  Shader_Loc *loc = nullptr;
  for (int i = 0; i < bindings->texture_locations.count; i++) {
    loc = &bindings->texture_locations[i];
    if (str8_equal(name, loc->name)) {
      break;
    }
    loc = nullptr;
  }

  if (!loc) {
    logprint("Could not find texture '%S'\n", name);
    return;
  }

  ID3D11ShaderResourceView *view = texture ? (ID3D11ShaderResourceView *)texture->view : (ID3D11ShaderResourceView *)d3d->fallback_tex->view;
  if (loc->vertex != -1) {
    d3d->device_context->VSSetShaderResources(loc->vertex, 1, &view);
  }
  if (loc->pixel != -1) {
    d3d->device_context->PSSetShaderResources(loc->pixel, 1, &view);
  }
}

internal void bind_uniform(Shader *shader, String8 name) {
  R_D3D11_State *d3d = r_d3d11_state();

  Shader_Bindings *bindings = shader->bindings;
  Shader_Loc *loc = nullptr;
  for (int i = 0; i < bindings->uniform_locations.count; i++) {
    loc = &bindings->uniform_locations[i];
    if (str8_equal(name, loc->name)) {
      break;
    }
    loc = nullptr;
  }

  Shader_Uniform *uniform = nullptr;
  for (int i = 0; i < bindings->uniforms.count; i++) {
    uniform = bindings->uniforms[i];
    if (str8_equal(name, uniform->name)) {
      break;
    }
    uniform = nullptr;
  }

  if (!uniform || !loc) {
    logprint("Could not find uniform '%S'\n", name);
    return;
  }

  if (loc->vertex != -1) {
    d3d->device_context->VSSetConstantBuffers(loc->vertex, 1, &uniform->buffer);
  }
  if (loc->pixel != -1) {
    d3d->device_context->PSSetConstantBuffers(loc->pixel, 1, &uniform->buffer);
  }
}

internal void apply_constants() {
  Shader *shader = current_shader;
  Shader_Bindings *bindings = shader->bindings;
  for (int i = 0; i < bindings->uniforms.count; i++) {
    Shader_Uniform *uniform = bindings->uniforms[i];
    if (uniform->dirty) {
      write_uniform_buffer(uniform->buffer, uniform->memory, uniform->size);
      uniform->dirty = 0;
    }
  }
}

internal void write_shader_constant(Shader *shader, String8 name, void *ptr, u32 size) {
  Shader_Bindings *bindings = shader->bindings;
  Shader_Constant *constant = nullptr;
  for (int i = 0; i < bindings->constants.count; i++) {
    constant = &bindings->constants[i];
    if (str8_equal(constant->name, name)) {
      break;
    }
    constant = nullptr;
  }

  Shader_Uniform *uniform = constant->uniform;
  MemoryCopy((void *)((uintptr_t)uniform->memory + constant->offset), ptr, size);
  uniform->dirty = 1;
}

internal void set_constant(String8 name, Matrix4 v) {
  write_shader_constant(current_shader, name, &v, sizeof(v));
}

internal void set_constant(String8 name, Vector4 v) {
  write_shader_constant(current_shader, name, &v, sizeof(v));
}

internal void set_constant(String8 name, Vector3 v) {
  write_shader_constant(current_shader, name, &v, sizeof(v));
}

internal void set_constant(String8 name, Vector2 v) {
  write_shader_constant(current_shader, name, &v, sizeof(v));
}

internal void set_constant(String8 name, f32 v) {
  write_shader_constant(current_shader, name, &v, sizeof(v));
}

internal void set_sampler(String8 name, Sampler_State_Kind sampler_kind) {
  R_D3D11_State *d3d = r_d3d11_state();

  ID3D11SamplerState *sampler = d3d->sampler_states[sampler_kind];

  Shader_Bindings *bindings = current_shader->bindings;
  Shader_Loc *loc = nullptr;
  for (int i = 0; i < bindings->sampler_locations.count; i++) {
    loc = &bindings->sampler_locations[i];
    if (str8_equal(loc->name, name)) {
      break;
    }
    loc = nullptr;
  }

  if (!loc) {
    logprint("Could not find sampler '%S'\n", name);
    return;
  }

  if (loc->vertex != -1) {
    d3d->device_context->VSSetSamplers(loc->vertex, 1, &sampler);
  }
  if (loc->pixel != -1) {
    d3d->device_context->PSSetSamplers(loc->pixel, 1, &sampler);
  }
}

internal void set_rasterizer_state(Rasterizer_State_Kind rasterizer_kind) {
  R_D3D11_State *d3d = r_d3d11_state();
  ID3D11RasterizerState *rasterizer_state = d3d->rasterizer_states[rasterizer_kind];
  d3d->device_context->RSSetState(rasterizer_state);
}

internal void set_rasterizer(ID3D11RasterizerState *rasterizer) {
  R_D3D11_State *d3d = r_d3d11_state();
  d3d->device_context->RSSetState(rasterizer);
}

internal void set_blend_state(Blend_State_Kind blend_state_kind) {
  R_D3D11_State *d3d = r_d3d11_state();
  ID3D11BlendState *blend_state = d3d->blend_states[blend_state_kind];
  d3d->device_context->OMSetBlendState(blend_state, NULL, 0xFFFFFFF);
}

internal void set_depth_state(Depth_State_Kind depth_state_kind) {
  R_D3D11_State *d3d = r_d3d11_state();
  ID3D11DepthStencilState *depth_state = d3d->depth_stencil_states[depth_state_kind];
  d3d->device_context->OMSetDepthStencilState(depth_state, 0);
}

internal void set_render_target(Render_Target *render_target) {
  R_D3D11_State *d3d = r_d3d11_state();
  d3d->device_context->OMSetRenderTargets(1, (ID3D11RenderTargetView **)&render_target->render_target_view, (ID3D11DepthStencilView *)render_target->depth_stencil_view);
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
    set_texture(str8_lit("diffuse_texture"), material->texture);

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

  // set_blend_state(BLEND_STATE_ALPHA);
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
  {
    Vector3 reflect_position = position;
    Matrix4 rotation_matrix = rotate_rh(guy->theta, camera.up);
    Matrix4 world_matrix = translate(reflect_position) * translate(guy->offset) * rotation_matrix;
    Matrix4 xform = camera.transform * world_matrix;

    set_shader(shader_mesh);

    bind_uniform(shader_mesh, str8_lit("Constants"));

    set_constant(str8_lit("xform"), xform);
    set_constant(str8_lit("color"), Vector4(.4f, .4f, .4f, 1.f));
    apply_constants(); 

    set_sampler(str8_lit("diffuse_sampler"), SAMPLER_STATE_LINEAR);

    draw_mesh(guy->mesh);
  }

}

internal void draw_scene() {
  local_persist bool first_call = true;
  if (first_call) {
    first_call = false;
    shadow_map = make_shadow_map(2048, 2048);
  }

  Entity_Manager *manager = get_entity_manager();

  R_D3D11_State *d3d = r_d3d11_state();

  Game_State *game_state = get_game_state();
  Editor *editor = get_editor();
  World *world = get_world();

  Camera camera;
  if (game_state->editing) {
    camera = editor->camera;
  } else {
    camera = game_state->camera;
  }

  // @Note Shadow mapping
  Sun *sun = manager->by_type._Sun.count ? manager->by_type._Sun[0] : 0;
  if (sun) {
    set_viewport(0, 0, (f32)shadow_map->texture->width, (f32)shadow_map->texture->height);
    set_depth_state(DEPTH_STATE_DEFAULT);
    set_rasterizer(rasterizer_cull_front);
    d3d->device_context->OMSetRenderTargets(0, nullptr, shadow_map->depth_stencil_view);
    d3d->device_context->ClearDepthStencilView(shadow_map->depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

    set_shader(shader_shadow_map);

    bind_uniform(shader_shadow_map, str8_lit("Constants"));
    Shader_Uniform *shadow_map_uniform = shader_shadow_map->bindings->lookup_uniform(str8_lit("Constants"));

    for (Entity *entity : manager->entities) {
      Triangle_Mesh *mesh = entity->mesh;
      if (!mesh) continue;

      //@Todo Set Transform
      Matrix4 rotation_matrix = rotate_rh(entity->theta, camera.up);
      Matrix4 world_matrix = translate(entity->visual_position) * translate(entity->offset) *rotation_matrix;

      set_constant(str8_lit("world"), world_matrix);
      set_constant(str8_lit("light_view_projection"), sun->light_space_matrix);
      apply_constants();

      UINT stride = sizeof(Vertex_XNCUU), offset = 0;
      d3d->device_context->IASetVertexBuffers(0, 1, &mesh->vertex_buffer, &stride, &offset);
      d3d->device_context->Draw((UINT)mesh->vertices.count, 0);
    }

    reset_viewport();
  }

  set_depth_state(DEPTH_STATE_DEFAULT);
  set_blend_state(BLEND_STATE_DEFAULT);
  set_rasterizer_state(RASTERIZER_STATE_DEFAULT);
  set_render_target(d3d->default_render_target);

  draw_world(world, camera);

  if (!game_state->editing) {
    draw_mirror_reflection();
  }

  // Shadow map
  if (0) {
    Vector2 dim = get_viewport()->dimension;
    immediate_begin();
    set_shader(shader_argb_texture);
    set_sampler(str8_lit("diffuse_sampler"), SAMPLER_STATE_POINT);
    set_depth_state(DEPTH_STATE_DISABLE);
    set_blend_state(BLEND_STATE_ALPHA);
    set_rasterizer_state(RASTERIZER_STATE_TEXT);

    bind_uniform(shader_argb_texture, str8_lit("Constants"));
    immediate_quad(shadow_map->texture,
      Vector2(0.0f, 0.0f), Vector2(dim.x, 0.0f), Vector2(dim.x, dim.y), Vector2(0.0f, dim.y),
      Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 0.0f),
      Vector4(1.0f, 1.0f, 1.0f, 0.4f));
    reset_texture(str8_lit("diffuse_texture"));
  }
}

internal void draw_world(World *world, Camera camera) {
  Entity_Manager *manager = get_entity_manager();

  R_D3D11_State *d3d = r_d3d11_state();

  Sun *sun = manager->by_type._Sun.count ? manager->by_type._Sun[0] : 0;
  Matrix4 light_space = sun ? sun->light_space_matrix : make_matrix4(1.0f);
  Vector3 light_direction = sun ? sun->light_direction : Vector3(0, 0, 0);
  Vector4 light_color = sun ? sun->override_color : make_vec4(1.0f);

  set_shader(shader_entity);

  Shader_Uniform *uniform = shader_entity->bindings->lookup_uniform(str8_lit("Constants"));
  bind_uniform(shader_entity, str8_lit("Constants"));

  set_texture(str8_lit("shadow_map"), shadow_map->texture);
  set_sampler(str8_lit("diffuse_sampler"), SAMPLER_STATE_LINEAR);
  set_sampler(str8_lit("point_sampler"), SAMPLER_STATE_POINT);
  set_constant(str8_lit("light_direction"), light_direction);
  set_constant(str8_lit("light_view_projection"), light_space);
  set_constant(str8_lit("light_color"), light_color);
    
  for (Entity *entity : manager->entities) {
    Triangle_Mesh *mesh = entity->mesh;
    if (!mesh) continue;

    Matrix4 rotation_matrix = rotate_rh(entity->theta, camera.up);
    Matrix4 world_matrix = translate(entity->visual_position) * translate(entity->offset) *rotation_matrix;
    Matrix4 xform = camera.transform * world_matrix;

    set_constant(str8_lit("xform"), xform);
    set_constant(str8_lit("world"), world_matrix);
    set_constant(str8_lit("use_override_color"), (float)entity->use_override_color);
    set_constant(str8_lit("override_color"), entity->override_color);
    apply_constants();

    draw_mesh(mesh);
  }

  reset_texture(str8_lit("shadow_map"));
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
  set_texture(str8_lit("diffuse_texture"), texture);

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
