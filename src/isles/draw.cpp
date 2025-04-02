
global Shader *current_shader;
global Auto_Array<Vertex_3D> immediate_vertices;

internal void set_shader(Shader_Kind shader_kind) {
  R_D3D11_State *d3d = r_d3d11_state();
  Shader *shader = d3d->shaders[shader_kind];
  if (shader != current_shader) {
    current_shader = shader;
    d3d->device_context->VSSetShader(shader->vertex_shader, nullptr, 0);
    d3d->device_context->PSSetShader(shader->pixel_shader, nullptr, 0);
    d3d->device_context->IASetInputLayout(shader->input_layout);
  }
}

internal void set_rasterizer_state(Rasterizer_State_Kind rasterizer_kind) {
  R_D3D11_State *d3d = r_d3d11_state();
  ID3D11RasterizerState *rasterizer_state = d3d->rasterizer_states[rasterizer_kind];
  d3d->device_context->RSSetState(rasterizer_state);
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

internal inline void immediate_vertex(Vector3 v, Vector3 n, Vector4 c, Vector2 uv) {
  Vertex_3D vertex;
  vertex.position = v;
  vertex.normal = n;
  vertex.color = c;
  vertex.uv = uv;
  immediate_vertices.push(vertex);
}

internal void immediate_flush() {
  set_shader(SHADER_IMMEDIATE);

  if (immediate_vertices.count == 0) {
    return;
  }

  R_D3D11_State *d3d = r_d3d11_state();

  ID3D11Buffer *vertex_buffer = make_vertex_buffer(immediate_vertices.data, immediate_vertices.count, sizeof(Vertex_3D));
  d3d->device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  UINT stride = sizeof(Vertex_3D), offset = 0;
  d3d->device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

  d3d->device_context->Draw((UINT)immediate_vertices.count, 0);

  vertex_buffer->Release();
  immediate_vertices.reset_count();
}

internal void immediate_begin() {
  immediate_flush();
}

internal void draw_mesh(Triangle_Mesh *mesh, bool use_override_color, Vector4 override_color) {
  set_shader(SHADER_IMMEDIATE);

  R_D3D11_State *d3d = r_d3d11_state();

  Auto_Array<Vertex_3D> vertices;
  vertices.reserve(mesh->vertices.count);
  for (int i = 0; i < mesh->vertices.count; i++) {
    Vertex_3D vertex;
    vertex.position = mesh->vertices[i];
    vertex.uv = mesh->uvs[i];
    vertex.normal = mesh->normals[i];
    vertex.color = use_override_color ? override_color : Vector4(1, 1, 1, 1);
    vertices.push(vertex);
  }

  UINT stride = sizeof(Vertex_3D), offset = 0;
  ID3D11Buffer *vertex_buffer = make_vertex_buffer(vertices.data, vertices.count, sizeof(Vertex_3D));
  d3d->device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

  ID3D11SamplerState *sampler = d3d->sampler_states[SAMPLER_STATE_LINEAR];
  d3d->device_context->PSSetSamplers(0, 1, &sampler);

  for (int i = 0; i < mesh->triangle_list_info.count; i++) {
    Triangle_List_Info triangle_list_info = mesh->triangle_list_info[i];

    Material *material = mesh->materials[triangle_list_info.material_index];
    if (material->texture) {
      d3d->device_context->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView **)&material->texture->view);
    } else {
      d3d->device_context->PSSetShaderResources(0, 1, &d3d->fallback_tex);
    }

    d3d->device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d->device_context->Draw(triangle_list_info.vertices_count, triangle_list_info.first_index);
  }

  vertices.clear();
  vertex_buffer->Release();
}

internal void draw_world(World *world, Camera camera) {
  R_D3D11_State *d3d = r_d3d11_state();

  set_rasterizer_state(RASTERIZER_STATE_DEFAULT);

  d3d->device_context->OMSetDepthStencilState(d3d->depth_stencil_states[DEPTH_STATE_DEFAULT], 0);
  d3d->device_context->OMSetBlendState(nullptr, NULL, 0xffffffff);
  d3d->device_context->OMSetRenderTargets(1, (ID3D11RenderTargetView **)&d3d->render_target_view, d3d->depth_stencil_view);

  for (int i = 0; i < world->entities.count; i++) {
    Entity *entity = world->entities[i];
    Triangle_Mesh *mesh = entity->mesh;

    //@Todo Set Transform
    Matrix4 rotation_matrix = rotate_rh(entity->theta, camera.up);
    Matrix4 world_matrix = translate(entity->visual_position) * rotation_matrix;
    Matrix4 transform = camera.projection_matrix * camera.view_matrix * world_matrix;

    R_D3D11_Uniform_Basic_3D uniform = {};
    uniform.transform = transform;
    uniform.world_matrix = world_matrix;
    ID3D11Buffer *uniform_buffer = d3d->uniform_buffers[UNIFORM_IMMEDIATE];
    write_uniform_buffer(uniform_buffer, &uniform, 0, sizeof(uniform));

    d3d->device_context->VSSetConstantBuffers(0, 1, &uniform_buffer);

    draw_mesh(mesh, entity->use_override_color, entity->override_color);
  }

  Entity *mirror = nullptr;
  if (world->guy && (mirror = lookup_entity(world->guy->mirror_id))) {
    {
      //@Note Draw guy clone
      Guy *guy = world->guy;

      Matrix4 rotation_matrix = rotate_rh(guy->theta, camera.up);
      Matrix4 world_matrix = translate(to_vector3(guy->reflect_position)) * rotation_matrix;
      Matrix4 transform = camera.projection_matrix * camera.view_matrix * world_matrix;

      R_D3D11_Uniform_Basic_3D uniform = {};
      uniform.transform = transform;
      uniform.world_matrix = world_matrix;
      ID3D11Buffer *uniform_buffer = d3d->uniform_buffers[UNIFORM_IMMEDIATE];
      write_uniform_buffer(uniform_buffer, &uniform, 0, sizeof(uniform));

      draw_mesh(guy->mesh, true, Vector4(0.4f, 0.4f, 0.4f, 1.0f));
    }

    // @Note Draw reflection beams
    // {
    //   immediate_begin();

    //   d3d->device_context->OMSetBlendState(d3d->blend_states[R_BLEND_STATE_ALPHA], NULL, 0xffffffff);
    //   d3d->device_context->OMSetDepthStencilState(d3d->depth_stencil_states[R_DEPTH_STENCIL_STATE_DEFAULT], 0);

    //   Vector3 mirror_forward = to_vector3(rotate_rh(-mirror->theta,  Vector3(0, 1, 0)) * Vector4(1, 0, 0, 1));
    //   Vector3 direction = to_vector3(world->guy->position - mirror->position);
    //   int distance = (int)Abs(magnitude(direction));

    //   Vector3 beam_size = Vector3(1, 1, 1);
    //   Vector4 beam_color = Vector4(1, 1, 1, 0.5f);
    //   Vector3 beam_start = to_vector3(mirror->position);
    //   f32 dx = mirror_forward.x > 0 ? 1.0f : -1.0f;
    //   f32 dz = mirror_forward.z > 0 ? 1.0f : -1.0f;
    //   f32 beam_start_x = (f32)mirror->position.x + dx;
    //   f32 beam_start_z = (f32)mirror->position.z + dz;

    //   Vector3 beam_position;
    //   // x
    //   beam_start = to_vector3(mirror->position);
    //   beam_start.x = beam_start_x;

    //   Matrix4 world_matrix = translate(beam_start);
    //   Matrix4 transform = camera.projection_matrix * camera.view_matrix * world_matrix;

    //   beam_size = Vector3(dx * (f32)distance, 1.f, 1.f);
    //   beam_size -= Vector3(0.f, 0.1f, 0.1f);
    //   beam_position = Vector3(dx * -0.5f, 0.001f, -0.5f);
    //   beam_position += Vector3(0.f, 0.05f, 0.05f);
    //   draw_imm_rectangle(beam_position, beam_size, beam_color);

    //   // z
    //   batch = push_immediate_batch(bucket);
    //   beam_start = to_vector3(mirror->position);
    //   beam_start.z = beam_start_z;

    //   batch = push_immediate_batch(bucket);
    //   batch->world = translate(beam_start);
    //   batch->view = camera.view_matrix;
    //   batch->projection = camera.projection_matrix;
    //   batch->texture = nullptr;

    //   beam_size = Vector3(1.f, 1.f, dz * (f32)distance);
    //   beam_size -= Vector3(0.1f, 0.1f, 0.f);
    //   beam_position = Vector3(dz * -0.5f, 0.001f, -0.5f);
    //   beam_position += Vector3(0.05f, 0.05f, 0.0f);
    //   draw_rectangle(batch, beam_position, beam_size, beam_color);
    // }
  }
}

internal void draw_imm_quad(Texture *texture, Vector2 position, Vector2 size, Vector2 uv, Vector2 uv_size, Vector4 color) {
  Vector3 p0 = Vector3(position.x,          position.y, 0.0f);
  Vector3 p1 = Vector3(position.x + size.x, position.y, 0.0f);
  Vector3 p2 = Vector3(position.x + size.x, position.y + size.y, 0.0f);
  Vector3 p3 = Vector3(position.x,          position.y + size.y, 0.0f);
  Vector2 uv0 = Vector2(uv.x, uv.y);
  Vector2 uv1 = Vector2(uv.x + uv_size.x, uv.y);
  Vector2 uv2 = Vector2(uv.x + uv_size.x, uv.y + uv_size.y);
  Vector2 uv3 = Vector2(uv.x, uv.y + uv_size.y);

  immediate_vertex(p0, Vector3(), color, uv0);
  immediate_vertex(p1, Vector3(), color, uv1);
  immediate_vertex(p2, Vector3(), color, uv2);
  immediate_vertex(p0, Vector3(), color, uv0);
  immediate_vertex(p2, Vector3(), color, uv2);
  immediate_vertex(p3, Vector3(), color, uv3);
}

internal void draw_imm_rectangle(Vector3 position, Vector3 size, Vector4 color) {
    Vector3 half = 0.5f * size;
    Vector2 uv = Vector2();
    Vector3 normal = Vector3();
    Vector3 p0 = Vector3(position.x,          position.y,          position.z + size.z);
    Vector3 p1 = Vector3(position.x + size.x, position.y,          position.z + size.z);
    Vector3 p2 = Vector3(position.x + size.x, position.y + size.y, position.z + size.z);
    Vector3 p3 = Vector3(position.x,          position.y + size.y, position.z + size.z);
    Vector3 p4 = Vector3(position.x + size.x, position.y,          position.z);
    Vector3 p5 = Vector3(position.x,          position.y,          position.z);
    Vector3 p6 = Vector3(position.x,          position.y + size.y, position.z);
    Vector3 p7 = Vector3(position.x + size.x, position.y + size.y, position.z);

    // front
    immediate_vertex(p0, normal, color, uv);
    immediate_vertex(p1, normal, color, uv);
    immediate_vertex(p2, normal, color, uv);
    immediate_vertex(p0, normal, color, uv);
    immediate_vertex(p2, normal, color, uv);
    immediate_vertex(p3, normal, color, uv);
    // back
    immediate_vertex(p4, normal, color, uv);
    immediate_vertex(p5, normal, color, uv);
    immediate_vertex(p6, normal, color, uv);
    immediate_vertex(p4, normal, color, uv);
    immediate_vertex(p6, normal, color, uv);
    immediate_vertex(p7, normal, color, uv);
    // left
    immediate_vertex(p5, normal, color, uv);
    immediate_vertex(p0, normal, color, uv);
    immediate_vertex(p3, normal, color, uv);
    immediate_vertex(p5, normal, color, uv);
    immediate_vertex(p3, normal, color, uv);
    immediate_vertex(p6, normal, color, uv);
    // right
    immediate_vertex(p1, normal, color, uv);
    immediate_vertex(p4, normal, color, uv);
    immediate_vertex(p7, normal, color, uv);
    immediate_vertex(p1, normal, color, uv);
    immediate_vertex(p7, normal, color, uv);
    immediate_vertex(p2, normal, color, uv);
    // top
    immediate_vertex(p3, normal, color, uv);
    immediate_vertex(p2, normal, color, uv);
    immediate_vertex(p7, normal, color, uv);
    immediate_vertex(p3, normal, color, uv);
    immediate_vertex(p7, normal, color, uv);
    immediate_vertex(p6, normal, color, uv);
    // bottom
    immediate_vertex(p1, normal, color, uv);
    immediate_vertex(p0, normal, color, uv);
    immediate_vertex(p5, normal, color, uv);
    immediate_vertex(p1, normal, color, uv);
    immediate_vertex(p5, normal, color, uv);
    immediate_vertex(p4, normal, color, uv);
}

internal void draw_imm_cube(Vector3 center, f32 size, Vector4 color) {
  draw_imm_rectangle(center, Vector3(size, size, size), color);
}
