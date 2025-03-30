//@Todo UI item expansion

global Editor *editor;
global Picker *g_picker;

internal void draw_world(World *world, Camera camera);

internal inline Editor *get_editor() {
  return editor;
}

internal void init_editor() {
  editor = new Editor();

  editor->entity_panel = new Entity_Panel();
  editor->entity_panel->position_x = ui_line_edit_create(str8_lit("X"));
  editor->entity_panel->position_y = ui_line_edit_create(str8_lit("Y"));
  editor->entity_panel->position_z = ui_line_edit_create(str8_lit("Z"));
  editor->entity_panel->theta      = ui_line_edit_create(str8_lit("Theta"));
  editor->entity_panel->color_r = ui_line_edit_create(str8_lit("R"));
  editor->entity_panel->color_g = ui_line_edit_create(str8_lit("G"));
  editor->entity_panel->color_b = ui_line_edit_create(str8_lit("B"));

  editor->gizmo_models[GIZMO_TRANSLATE][AXIS_X] = load_model("data/models/gizmo/translate_x.obj");
  editor->gizmo_models[GIZMO_TRANSLATE][AXIS_Y] = load_model("data/models/gizmo/translate_y.obj");
  editor->gizmo_models[GIZMO_TRANSLATE][AXIS_Z] = load_model("data/models/gizmo/translate_z.obj");
  editor->gizmo_models[GIZMO_ROTATE][AXIS_X] = load_model("data/models/gizmo/rotation_x.obj");
  editor->gizmo_models[GIZMO_ROTATE][AXIS_Y] = load_model("data/models/gizmo/rotation_y.obj");
  editor->gizmo_models[GIZMO_ROTATE][AXIS_Z] = load_model("data/models/gizmo/rotation_z.obj");

  editor->panel = new Editor_Panel();
  editor->panel->expand_load_world = false;
  editor->panel->expand_saveas = false;
  editor->panel->edit_saveas = ui_line_edit_create(str8_lit("Save As"));
  editor->panel->edit_load_world = ui_line_edit_create(str8_lit("Load"));
}

void Editor::update_entity_panel() {
  Entity *entity = editor->selected_entity;
  if (entity) {
    int n;
    n = snprintf((char *)entity_panel->position_x->buffer, entity_panel->position_x->buffer_capacity, "%d", entity->position.x);
    entity_panel->position_x->buffer_len = n;
    entity_panel->position_x->buffer_pos = n;

    n = snprintf((char *)entity_panel->position_y->buffer, entity_panel->position_y->buffer_capacity, "%d", entity->position.y);
    entity_panel->position_y->buffer_len = n;
    entity_panel->position_y->buffer_pos = n;

    n = snprintf((char *)entity_panel->position_z->buffer, entity_panel->position_z->buffer_capacity, "%d", entity->position.z);
    entity_panel->position_z->buffer_len = n;
    entity_panel->position_z->buffer_pos = n;

    n = snprintf((char *)entity_panel->theta->buffer, entity_panel->theta->buffer_capacity, "%.2f", RadToDeg(entity->theta_target));
    entity_panel->theta->buffer_len = n;
    entity_panel->theta->buffer_pos = n;

    n = snprintf((char *)entity_panel->color_r->buffer, entity_panel->color_r->buffer_capacity, "%.2f", entity->override_color.x);
    entity_panel->color_r->buffer_len = n;
    entity_panel->color_r->buffer_pos = n;

    n = snprintf((char *)entity_panel->color_g->buffer, entity_panel->color_g->buffer_capacity, "%.2f", entity->override_color.y);
    entity_panel->color_g->buffer_len = n;
    entity_panel->color_g->buffer_pos = n;

    n = snprintf((char *)entity_panel->color_b->buffer, entity_panel->color_b->buffer_capacity, "%.2f", entity->override_color.z);
    entity_panel->color_b->buffer_len = n;
    entity_panel->color_b->buffer_pos = n;
 }
}

void Editor::select_entity(Entity *e) {
  selected_entity = e;
  update_entity_panel();
}

internal void r_picker_render_gizmo(Picker *picker) {
  R_D3D11_State *d3d11_state = r_d3d11_state();

  d3d11_state->device_context->OMSetBlendState(nullptr, NULL, 0xffffffff);
  d3d11_state->device_context->OMSetDepthStencilState(d3d11_state->depth_stencil_states[R_DEPTH_STENCIL_STATE_DEFAULT], 0);

  d3d11_state->device_context->OMSetRenderTargets(1, (ID3D11RenderTargetView **)&picker->render_target_view, d3d11_state->depth_stencil_view);

  float clear_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  d3d11_state->device_context->ClearRenderTargetView((ID3D11RenderTargetView*)picker->render_target_view, clear_color);
  d3d11_state->device_context->ClearDepthStencilView(d3d11_state->depth_stencil_view, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

  Shader *shader = d3d11_state->shaders[D3D11_SHADER_PICKER];
  d3d11_state->device_context->VSSetShader(shader->vertex_shader, nullptr, 0);
  d3d11_state->device_context->PSSetShader(shader->pixel_shader, nullptr, 0);

  d3d11_state->device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  d3d11_state->device_context->IASetInputLayout(shader->input_layout);

  Auto_Array<Vector3> vertices;

  Game_State *game_state = get_game_state();

  Entity *e = editor->selected_entity;

  for (u32 axis = AXIS_X; axis <= AXIS_Z; axis++) {
    Model *model = editor->gizmo_models[editor->active_gizmo][axis];

    Matrix4 world_matrix = translate(e->visual_position.x, e->visual_position.y, e->visual_position.z) * scale(Vector3(2.f, 2.f, 2.f));
    Matrix4 view_matrix = editor->camera.view_matrix;
    Matrix4 transform = editor->camera.projection_matrix * view_matrix * world_matrix;

    u32 id = axis;
    Vector4 pick_color;
    pick_color.x = ((id & 0x000000FF) >> 0 ) / 255.0f;
    pick_color.y = ((id & 0x0000FF00) >> 8 ) / 255.0f;
    pick_color.z = ((id & 0x00FF00FF) >> 16) / 255.0f;
    pick_color.w = ((id & 0xFF0000FF) >> 24) / 255.0f;

    R_D3D11_Uniform_Picker uniform = {};
    uniform.transform = transform;
    uniform.pick_color = pick_color;

    ID3D11Buffer *uniform_buffer = d3d11_state->uniform_buffers[D3D11_UNIFORM_PICKER];
    d3d11_state->device_context->VSSetConstantBuffers(0, 1, &uniform_buffer);
    d3d11_state->device_context->PSSetConstantBuffers(0, 1, &uniform_buffer);
    {
      D3D11_MAPPED_SUBRESOURCE resource = {};
      if (d3d11_state->device_context->Map(uniform_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource) == S_OK) {
        memcpy(resource.pData, &uniform, sizeof(uniform));
        d3d11_state->device_context->Unmap(uniform_buffer, 0);
      }
    }

    for (int mesh_idx = 0; mesh_idx < model->meshes.count; mesh_idx++) {
      Mesh *mesh = model->meshes[mesh_idx];

      vertices.reserve(mesh->positions.count);

      for (int vert_idx = 0; vert_idx < mesh->positions.count; vert_idx++) {
        Vector3 p = mesh->positions[vert_idx];
        vertices.push(p);
      }

      ID3D11Buffer *vertex_buffer = nullptr;
      {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = sizeof(Vector3) * (UINT)vertices.count;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem = vertices.data;
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;
        d3d11_state->device->CreateBuffer(&desc, &data, &vertex_buffer);
      }
      UINT stride = sizeof(Vector3), offset = 0;
      d3d11_state->device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

      d3d11_state->device_context->Draw((UINT)vertices.count, 0);

      vertex_buffer->Release();
      vertices.reset_count();
    }
  }

  d3d11_state->swap_chain->Present(1, 0);

  d3d11_state->device_context->OMSetRenderTargets(1, &d3d11_state->render_target_view, d3d11_state->depth_stencil_view);
}

internal void r_picker_render(Picker *picker) {
  R_D3D11_State *d3d11_state = r_d3d11_state();

  d3d11_state->device_context->OMSetBlendState(nullptr, NULL, 0xffffffff);

  d3d11_state->device_context->OMSetDepthStencilState(d3d11_state->depth_stencil_states[R_DEPTH_STENCIL_STATE_DEFAULT], 0);

  d3d11_state->device_context->OMSetRenderTargets(1, (ID3D11RenderTargetView **)&picker->render_target_view, d3d11_state->depth_stencil_view);

  float clear_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  d3d11_state->device_context->ClearRenderTargetView((ID3D11RenderTargetView*)picker->render_target_view, clear_color);
  d3d11_state->device_context->ClearDepthStencilView(d3d11_state->depth_stencil_view, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

  Shader *shader = d3d11_state->shaders[D3D11_SHADER_PICKER];
  ID3D11VertexShader *vertex_shader = shader->vertex_shader;
  ID3D11PixelShader *pixel_shader = shader->pixel_shader;
  d3d11_state->device_context->VSSetShader(vertex_shader, nullptr, 0);
  d3d11_state->device_context->PSSetShader(pixel_shader, nullptr, 0);

  ID3D11InputLayout *input_layout = shader->input_layout;
  d3d11_state->device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  d3d11_state->device_context->IASetInputLayout(input_layout);

  Auto_Array<Vector3> vertices;

  Game_State *game_state = get_game_state();
  World *world = get_world();

  for (int i = 0; i < world->entities.count; i++) {
    Entity *e = world->entities[i];
    Model *model = e->model;

    Matrix4 rotation_matrix = rotate_rh(e->visual_rotation.y, editor->camera.up);
    Matrix4 world_matrix = translate(e->visual_position) * rotation_matrix * translate(-e->visual_position);
    Matrix4 view = editor->camera.view_matrix;
    Matrix4 transform = editor->camera.projection_matrix * editor->camera.view_matrix;

    Vector4 pick_color;
    pick_color.x = ((e->id & 0x000000FF) >> 0 ) / 255.0f;
    pick_color.y = ((e->id & 0x0000FF00) >> 8 ) / 255.0f;
    pick_color.z = ((e->id & 0x00FF00FF) >> 16) / 255.0f;
    pick_color.w = ((e->id & 0xFF0000FF) >> 24) / 255.0f;

    R_D3D11_Uniform_Picker uniform = {};
    uniform.transform = transform;
    uniform.pick_color = pick_color;

    ID3D11Buffer *uniform_buffer = d3d11_state->uniform_buffers[D3D11_UNIFORM_PICKER];
    d3d11_state->device_context->VSSetConstantBuffers(0, 1, &uniform_buffer);
    d3d11_state->device_context->PSSetConstantBuffers(0, 1, &uniform_buffer);
    {
      D3D11_MAPPED_SUBRESOURCE resource = {};
      if (d3d11_state->device_context->Map(uniform_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource) == S_OK) {
        memcpy(resource.pData, &uniform, sizeof(uniform));
        d3d11_state->device_context->Unmap(uniform_buffer, 0);
      }
    }

    for (int mesh_idx = 0; mesh_idx < model->meshes.count; mesh_idx++) {
      Mesh *mesh = model->meshes[mesh_idx];

      vertices.reserve(mesh->positions.count);

      for (int vert_idx = 0; vert_idx < mesh->positions.count; vert_idx++) {
        Vector3 p = e->visual_position + mesh->positions[vert_idx];
        vertices.push(p);
      }

      ID3D11Buffer *vertex_buffer = nullptr;
      {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = sizeof(Vector3) * (UINT)vertices.count;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem = vertices.data;
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;
        d3d11_state->device->CreateBuffer(&desc, &data, &vertex_buffer);
      }
      UINT stride = sizeof(Vector3), offset = 0;
      d3d11_state->device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

      d3d11_state->device_context->Draw((UINT)vertices.count, 0);

      vertex_buffer->Release();
      vertices.reset_count();
    }
  }

  d3d11_state->swap_chain->Present(1, 0);

  d3d11_state->device_context->OMSetRenderTargets(1, &d3d11_state->render_target_view, d3d11_state->depth_stencil_view);
}

internal void r_picker_init(Picker *picker) {
  R_D3D11_State *d3d11_state = r_d3d11_state();

  if (picker->texture) ((ID3D11Texture2D *)picker->texture)->Release();
  if (picker->select_texture) ((ID3D11Texture2D *)picker->select_texture)->Release();
  if (picker->render_target_view) ((ID3D11RenderTargetView *)picker->render_target_view)->Release();

  HRESULT hr = S_OK;

  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = picker->dimension.x;
  desc.Height = picker->dimension.y;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_RENDER_TARGET;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  desc.MiscFlags = 0;
  ID3D11Texture2D *texture = nullptr;
  hr = d3d11_state->device->CreateTexture2D(&desc, NULL, &texture);

  D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
  rtv_desc.Format = desc.Format;
  rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  rtv_desc.Texture2D.MipSlice = 0;
  ID3D11RenderTargetView *view = nullptr;
  hr = d3d11_state->device->CreateRenderTargetView(texture, &rtv_desc, &view);

  picker->texture = texture;
  picker->render_target_view = view;

  {
    D3D11_TEXTURE2D_DESC staging_desc = {};
    staging_desc.Width = 1;
    staging_desc.Height = 1;
    staging_desc.MipLevels = 1;
    staging_desc.ArraySize = 1;
    staging_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    staging_desc.SampleDesc.Count = 1;
    staging_desc.SampleDesc.Quality = 0;
    staging_desc.Usage = D3D11_USAGE_STAGING;
    staging_desc.BindFlags = 0;
    staging_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    staging_desc.MiscFlags = 0;
    hr = d3d11_state->device->CreateTexture2D(&staging_desc, NULL, (ID3D11Texture2D **)&picker->select_texture);
  }
}

internal Pid r_picker_get_id(Picker *picker, Vector2Int mouse) {
  Pid result = 0xFFFFFFFF;

  R_D3D11_State *d3d11_state = r_d3d11_state();
  D3D11_BOX src_box = {};
  src_box.left = mouse.x;
  src_box.right = mouse.x + 1;
  src_box.top = mouse.y;
  src_box.bottom = mouse.y + 1;
  src_box.front = 0;
  src_box.back = 1;

  HRESULT hr = S_OK;

  d3d11_state->device_context->CopySubresourceRegion((ID3D11Texture2D *)picker->select_texture, 0, 0, 0, 0, (ID3D11Texture2D *)picker->texture, 0, &src_box);

  D3D11_MAPPED_SUBRESOURCE mapped = {};
  hr = d3d11_state->device_context->Map((ID3D11Texture2D *)picker->select_texture, 0, D3D11_MAP_READ, 0, &mapped);
  u8 *ptr = (u8 *)mapped.pData;

  u8 r = *ptr++;
  u8 g = *ptr++;
  u8 b = *ptr++;
  u8 a = *ptr++;
  result = r | (g << 8) | (b << 16) | (a << 24);
        
  d3d11_state->device_context->Unmap((ID3D11Texture2D *)picker->select_texture, 0);

  return result;
}

internal void update_editor() {
  update_camera_position(&editor->camera);

  Vector2Int mouse_position = g_input.mouse_position;

  editor->gizmo_axis_hover = (Axis)-1;
  if (editor->selected_entity) {
    r_picker_render_gizmo(g_picker);
    Pid gizmo_id = r_picker_get_id(g_picker, mouse_position);
    if (gizmo_id != 0xFFFFFFFF) {
      editor->gizmo_axis_hover = (Axis)gizmo_id;
    }
  }

  if (!mouse_down(0)) {
    editor->gizmo_axis_active = (Axis)-1;
  }

  if (editor->gizmo_axis_active != -1) {
    switch (editor->active_gizmo) {
    case GIZMO_TRANSLATE:
    {
      Vector3 ray = get_mouse_ray(editor->camera, mouse_position, g_viewport->dimension);
      Vector3 ray_start = get_mouse_ray(editor->camera, editor->gizmo_mouse_start, g_viewport->dimension);
      Vector3 distance = 15.0f * (ray - ray_start);

      Axis gizmo_axis = editor->gizmo_axis_active;
      Vector3 gizmo_vector = vector_from_axis(gizmo_axis);
      Vector3 travel = projection(distance, gizmo_vector);

      Vector3Int meters = truncate(travel);
      editor->selected_entity->set_position(editor->selected_entity->position + meters);
      editor->update_entity_panel();

      if (meters.x || meters.y || meters.z) {
        editor->gizmo_mouse_start = mouse_position;
      }
      break;
    }
    case GIZMO_ROTATE:
    {
        
      break;
    }
    case GIZMO_SCALE:
    {
      break;
    }
    }
  }

  if (editor->gizmo_axis_hover != -1) {
    if (mouse_clicked(0)) {
      editor->gizmo_axis_active = editor->gizmo_axis_hover;
      editor->gizmo_mouse_start = mouse_position;
    }
  } else if (editor->gizmo_axis_active == -1){
    if (mouse_clicked(0)) {
      r_picker_render(g_picker);
      Pid id = r_picker_get_id(g_picker, mouse_position);
      if (id != 0xFFFFFFFF) {
        Entity *e = lookup_entity(id);
        editor->select_entity(e);
      } else {
        editor->select_entity(nullptr);
      }
    }
  }

  Vector2 camera_delta = get_mouse_right_drag_delta();
  camera_delta = 0.2f * camera_delta;
  update_camera_orientation(&editor->camera, camera_delta);

  Entity *selected_entity = editor->selected_entity;

  draw_world(get_world(), editor->camera);

  if (editor->selected_entity) {
    Draw_Bucket *bucket = draw_push();
    bucket->immediate.clear_depth_buffer = true;
    bucket->immediate.depth_state = R_DEPTH_STENCIL_STATE_DEFAULT;
    bucket->immediate.light_dir = Vector3(1.0f, 1.0f, 1.0f);

    Matrix4 world_matrix = make_matrix4(1.0f);
    Matrix4 view = editor->camera.view_matrix;
    Matrix4 transform = editor->camera.projection_matrix * editor->camera.view_matrix;

    for (int axis = AXIS_X; axis <= AXIS_Z; axis++) {
      Model *model = editor->gizmo_models[editor->active_gizmo][axis];

      Matrix4 world_matrix = translate(selected_entity->visual_position.x, selected_entity->visual_position.y, selected_entity->visual_position.z) * scale(Vector3(2.0f, 2.0f, 2.0f));
      Matrix4 view_matrix = editor->camera.view_matrix;
      Matrix4 transform = editor->camera.projection_matrix * view_matrix * world_matrix;

      for (int mesh_idx = 0; mesh_idx < model->meshes.count; mesh_idx++) {
        Mesh *mesh = model->meshes[mesh_idx];

        Draw_Immediate_Batch *batch = push_immediate_batch(bucket);
        batch->world = world_matrix;
        batch->view = editor->camera.view_matrix;
        batch->projection = editor->camera.projection_matrix;
        batch->texture = nullptr;

        Vector4 color = mesh->material.diffuse_color;
        if (editor->gizmo_axis_hover == axis && editor->gizmo_axis_active == (Axis)-1) {
          color = mix(color, Vector4(1.0f, 1.0f, 1.0f, 1.0f), 0.3f);
        }
        if (editor->gizmo_axis_active == axis) {
          color = mix(color, Vector4(1.0f, 1.0f, 1.0f, 1.0f), 0.3f);
        }

        for (int vert_idx = 0; vert_idx < mesh->positions.count; vert_idx++) {
          Vector3 p = mesh->positions[vert_idx];
          Vector2 uv = Vector2();
          Vector3 normal = Vector3();
          draw_immediate_vertex(batch, p, normal, color, uv);
        }
      }
    }
  }
}

internal u64 djb2_hash(const char *str) {
  u64 hash = 5381;
  int c;
  while (c = *str++) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

internal Entity_Prototype *entity_prototype_lookup(u64 prototype_id) {
  Entity_Prototype *result = nullptr;
  auto it = editor->prototype_table.find(prototype_id);
  if (it != editor->prototype_table.end()) {
    result = it->second;
  }
  return result;
}

internal Entity_Prototype *entity_prototype_lookup(const char *name) {
  u64 id = djb2_hash(name);
  Entity_Prototype *result = entity_prototype_lookup(id);
  return result;
}

internal Entity_Prototype *entity_prototype_create(char *name) {
  Entity_Prototype *result = (Entity_Prototype*)calloc(sizeof(Entity_Prototype), 1);
  result->name = name;
  result->id = djb2_hash(name);
  editor->prototype_table.insert({result->id, result});
  editor->prototype_array.push(result);
  return result;
}

internal Entity *entity_from_prototype(Entity_Prototype *prototype) {
  Entity *entity = entity_make(prototype->entity.kind);
  entity->prototype_id = prototype->id;
  entity->kind  = prototype->entity.kind;
  entity->flags = prototype->entity.flags;
  entity->model = prototype->entity.model;
  return entity;
}

internal void entity_from_prototype(Entity *entity, Entity_Prototype *prototype) {
  entity->prototype_id = prototype->id;
  entity->kind  = prototype->entity.kind;
  entity->flags = prototype->entity.flags;
  entity->model = prototype->entity.model;
}

internal void editor_present_ui() {
  Editor_Panel *panel = editor->panel;
  
  ui_set_next_pref_width(ui_pixels(110.f));
  ui_set_next_pref_height(ui_pixels(300.f));
  UI_Box *column = ui_box_create(0, str8_lit("##column"));
  ui_push_parent(column);

  //@Note Level name
  ui_push_pref_width(ui_txt(2.0f));
  ui_push_pref_height(ui_txt(4.0f));
  ui_box_create_format(UI_BOX_FLAG_TEXT_ELEMENT, "%S##level", get_world()->name);

  //@Note Save
  if (ui_clicked(ui_button(str8_lit("Save")))) {
    World *world = get_world();
    save_world(world);
  }

  //@Note Save As prompt
  if (ui_clicked(ui_button(str8_lit("Save As")))) {
    editor->panel->expand_saveas = !editor->panel->expand_saveas;
  }
  if (editor->panel->expand_saveas) {
    ui_set_next_pref_width(ui_pixels(80.0f));
    ui_set_next_pref_height(ui_pixels(20.0f));
    ui_set_next_text_padding(2.0f);
    UI_Signal sig = ui_line_edit(str8_lit("##saveas_edit"), panel->edit_saveas);
    if ((sig.flags & UI_SIGNAL_FLAG_PRESSED) && sig.key == OS_KEY_ENTER) {
      String8 name = str8(panel->edit_saveas->buffer, panel->edit_saveas->buffer_len);
      save_world(get_world(), name);
      editor->panel->expand_saveas = false;
    }
  }

  //@Note Load prompt
  if (ui_clicked(ui_button(str8_lit("Load")))) {
    editor->panel->expand_load_world = !editor->panel->expand_load_world;
  }
  if (editor->panel->expand_load_world) {
    ui_set_next_pref_width(ui_pixels(80.0f));
    ui_set_next_pref_height(ui_pixels(20.0f));
    ui_set_next_text_padding(2.0f);
    UI_Signal sig = ui_line_edit(str8_lit("##load_edit"), panel->edit_load_world);
    if ((sig.flags & UI_SIGNAL_FLAG_PRESSED) && sig.key == OS_KEY_ENTER) {
      String8 name = str8(panel->edit_load_world->buffer, panel->edit_load_world->buffer_len);
      load_world(name);
      editor->panel->expand_load_world = false;
    }
  }

  ui_push_pref_width(ui_txt(2.0f));
  ui_push_pref_height(ui_txt(4.0f));
  if (ui_clicked(ui_button(str8_lit("New")))) {
    World *world = new World();
    set_world(world);
  }

  ui_pop_parent();

  ui_push_background_color(Vector4(0.18f, 0.42f, 0.33f, 1.f));

  ui_set_next_pref_width(ui_pixels(120.f));
  ui_set_next_pref_height(ui_pixels(180.f));
  ui_set_next_child_layout_axis(AXIS_Y);
  ui_push_text_padding(4.0f);
  UI_Box *prototype_panel = ui_box_create(UI_BOX_FLAG_DRAW_BACKGROUND, str8_lit("##prototype"));
  ui_push_parent(prototype_panel);
  {
    ui_push_pref_width(ui_txt(1.0f));
    ui_push_pref_height(ui_txt(1.0f));

    UI_Box *header = ui_box_create(UI_BOX_FLAG_TEXT_ELEMENT, str8_lit("Prototype"));

    Entity_Prototype *prototype = editor->prototype_array[editor->prototype_idx];

    UI_Box *text = ui_box_create_format(UI_BOX_FLAG_TEXT_ELEMENT, "%s##name", prototype->name);

    ui_set_next_pref_width(ui_children_sum());
    ui_set_next_pref_height(ui_children_sum());
    ui_set_next_child_layout_axis(AXIS_X);
    UI_Box *cont = ui_box_create(0, str8_lit("##cont"));

    ui_push_parent(cont);
    if (ui_clicked(ui_button(str8_lit("Prev")))) {
      editor->prototype_idx--;
    }
    if (ui_clicked(ui_button(str8_lit("Next")))) {
      editor->prototype_idx++;
    }
    ui_pop_parent();
    editor->prototype_idx = range_clamp(editor->prototype_idx, 0, (int)editor->prototype_array.count - 1);

    if (ui_clicked(ui_button(str8_lit("Instantiate")))) {
      Entity *instance = entity_from_prototype(prototype);
      instance->set_position(truncate(editor->camera.origin + editor->camera.forward));
      if (editor->selected_entity) {
        Vector3Int unit = truncate(get_nearest_axis(editor->camera.origin - to_vector3(editor->selected_entity->position)));
        instance->set_position(editor->selected_entity->position + unit);
      }
      editor->select_entity(instance);

      World *world = get_world();
      world->entities.push(instance);
      if (instance->kind == ENTITY_GUY) {
        world->guy = static_cast<Guy*>(instance);
      }
    }

    ui_pop_pref_width();
    ui_pop_pref_height();
  }
  ui_pop_parent();
  ui_pop_background_color();

  Entity *entity = editor->selected_entity;
  Entity_Panel *entity_panel = editor->entity_panel;
  UI_Line_Edit *edit_x = entity_panel->position_x;
  UI_Line_Edit *edit_y = entity_panel->position_y;
  UI_Line_Edit *edit_z = entity_panel->position_z;
  if (entity) {
    f32 start = g_viewport->dimension.x - 160.0f;
    ui_set_next_fixed_x(start);
    ui_set_next_fixed_y(0.0f);
    ui_set_next_fixed_width(160.0f);
    ui_set_next_fixed_height(300.0f);
    // ui_set_next_background_color(Vector4(1.f, 1.f, 1.f, 1.f));
    UI_Box *entity_panel_box = ui_box_create(UI_BOX_FLAG_DRAW_BACKGROUND, str8_lit("##ent_panel"));
    ui_push_parent(entity_panel_box);
    {
      ui_box_create_format(UI_BOX_FLAG_TEXT_ELEMENT, "Entity: %s #%llu", string_from_entity_kind(entity->kind), entity->id);

      ui_set_next_pref_width(ui_children_sum());
      ui_set_next_pref_height(ui_children_sum());
      ui_set_next_child_layout_axis(AXIS_X);
      ui_set_next_background_color(Vector4(0.f, 0.f, 0.f, 0.f));
      UI_Box *panel_x = ui_box_create(0, str8_lit("##panel_x"));
      ui_push_parent(panel_x);
      {
        ui_set_next_text_padding(2.0f);
        ui_box_create(UI_BOX_FLAG_TEXT_ELEMENT, edit_x->name);

        ui_set_next_pref_width(ui_pixels(80.f));
        ui_set_next_pref_height(ui_pixels(20.f));
        ui_set_next_text_padding(4.0f);
        UI_Signal sig = ui_text_edit(str8_lit("##field_x"), edit_x->buffer, edit_x->buffer_capacity, &edit_x->buffer_pos, &edit_x->buffer_len);

        if (sig.flags & UI_SIGNAL_FLAG_PRESSED) {
          int x = (int)strtol((char *)edit_x->buffer, NULL, 10);
          entity->set_position(AXIS_X, x);
        }
      }
      ui_pop_parent();

      ui_set_next_pref_width(ui_children_sum());
      ui_set_next_pref_height(ui_children_sum());
      ui_set_next_child_layout_axis(AXIS_X);
      ui_set_next_background_color(Vector4(0.f, 0.f, 0.f, 0.f));
      UI_Box *panel_y = ui_box_create(0, str8_lit("##panel_y"));
      ui_push_parent(panel_y);
      {
        ui_set_next_text_padding(2.0f);
        ui_box_create(UI_BOX_FLAG_TEXT_ELEMENT, edit_y->name);

        ui_set_next_text_padding(4.0f);
        ui_set_next_pref_width(ui_pixels(80.f));
        ui_set_next_pref_height(ui_pixels(20.f));
        UI_Signal sig = ui_text_edit(str8_lit("##field_y"), edit_y->buffer, edit_y->buffer_capacity, &edit_y->buffer_pos, &edit_y->buffer_len);

        if (sig.flags & UI_SIGNAL_FLAG_PRESSED) {
          int y = (int)strtol((char *)edit_y->buffer, NULL, 10);
          entity->set_position(AXIS_Y, y);
        }
      }
      ui_pop_parent();

      ui_set_next_pref_width(ui_children_sum());
      ui_set_next_pref_height(ui_children_sum());
      ui_set_next_child_layout_axis(AXIS_X);
      ui_set_next_background_color(Vector4(0.f, 0.f, 0.f, 0.f));
      UI_Box *panel_z = ui_box_create(0, str8_lit("##panel_z"));
      ui_push_parent(panel_z);
      {
        ui_set_next_text_padding(2.0f);
        ui_box_create(UI_BOX_FLAG_TEXT_ELEMENT, edit_z->name);

        ui_set_next_pref_width(ui_pixels(80.f));
        ui_set_next_pref_height(ui_pixels(20.f));
        ui_set_next_text_padding(4.0f);
        UI_Signal sig = ui_text_edit(str8_lit("##field_z"), edit_z->buffer, edit_z->buffer_capacity, &edit_z->buffer_pos, &edit_z->buffer_len);

        if (sig.flags & UI_SIGNAL_FLAG_PRESSED) {
          int z = (int)strtol((char *)edit_z->buffer, NULL, 10);
          entity->set_position(AXIS_Z, z);
        }
      }
      ui_pop_parent();

      //@Note Theta
      {
        ui_set_next_pref_width(ui_children_sum());
        ui_set_next_pref_height(ui_children_sum());
        ui_set_next_child_layout_axis(AXIS_X);
        UI_Box *cont = ui_box_create(0, str8_lit("##theta"));
        ui_push_parent(cont);
        {
          ui_set_next_text_padding(2.0f);
          ui_box_create(UI_BOX_FLAG_TEXT_ELEMENT, entity_panel->theta->name);

          ui_set_next_pref_width(ui_pixels(80.f));
          ui_set_next_pref_height(ui_pixels(20.f));
          ui_set_next_text_padding(4.0f);
          UI_Signal sig = ui_text_edit(str8_lit("##field"), entity_panel->theta->buffer, entity_panel->theta->buffer_capacity, &entity_panel->theta->buffer_pos, &entity_panel->theta->buffer_len);

          if (sig.flags & UI_SIGNAL_FLAG_PRESSED) {
            f32 f = strtof((char *)entity_panel->theta->buffer, NULL);
            f = DegToRad(f);
            entity->set_theta(f);
          }
        }
        ui_pop_parent();
      }

      UI_Line_Edit *color_r = entity_panel->color_r;
      UI_Line_Edit *color_g = entity_panel->color_g;
      UI_Line_Edit *color_b = entity_panel->color_b;

      //@Note Override color
      ui_set_next_pref_width(ui_children_sum());
      ui_set_next_pref_height(ui_children_sum());
      ui_set_next_child_layout_axis(AXIS_X);
      ui_set_next_background_color(Vector4(0.f, 0.f, 0.f, 0.f));
      UI_Box *color_box_r = ui_box_create(0, str8_lit("##color_r"));
      ui_push_parent(color_box_r);
      {
        ui_set_next_text_padding(2.0f);
        ui_box_create(UI_BOX_FLAG_TEXT_ELEMENT, color_r->name);

        ui_set_next_pref_width(ui_pixels(80.f));
        ui_set_next_pref_height(ui_pixels(20.f));
        ui_set_next_text_padding(4.0f);
        UI_Signal sig = ui_text_edit(str8_lit("##field_r"), color_r->buffer, color_r->buffer_capacity, &color_r->buffer_pos, &color_r->buffer_len);

        if (sig.flags & UI_SIGNAL_FLAG_PRESSED) {
          f32 r = (f32)strtof((char *)color_r->buffer, NULL);
          entity->override_color.x = r;
        }
      }
      ui_pop_parent();

      ui_set_next_pref_width(ui_children_sum());
      ui_set_next_pref_height(ui_children_sum());
      ui_set_next_child_layout_axis(AXIS_X);
      ui_set_next_background_color(Vector4(0.f, 0.f, 0.f, 0.f));
      UI_Box *color_g_box = ui_box_create(0, str8_lit("##color_g"));
      ui_push_parent(color_g_box);
      {
        ui_set_next_text_padding(2.0f);
        ui_box_create(UI_BOX_FLAG_TEXT_ELEMENT, color_g->name);

        ui_set_next_text_padding(4.0f);
        ui_set_next_pref_width(ui_pixels(80.f));
        ui_set_next_pref_height(ui_pixels(20.f));
        UI_Signal sig = ui_text_edit(str8_lit("##field_g"), color_g->buffer, color_g->buffer_capacity, &color_g->buffer_pos, &color_g->buffer_len);

        if (sig.flags & UI_SIGNAL_FLAG_PRESSED) {
          f32 g = strtof((char *)color_g->buffer, NULL);
          entity->override_color.y = g;
        }
      }
      ui_pop_parent();

      ui_set_next_pref_width(ui_children_sum());
      ui_set_next_pref_height(ui_children_sum());
      ui_set_next_child_layout_axis(AXIS_X);
      ui_set_next_background_color(Vector4(0.f, 0.f, 0.f, 0.f));
      UI_Box *color_b_box = ui_box_create(0, str8_lit("##color_b"));
      ui_push_parent(color_b_box);
      {
        ui_set_next_text_padding(2.0f);
        ui_box_create(UI_BOX_FLAG_TEXT_ELEMENT, color_b->name);

        ui_set_next_pref_width(ui_pixels(80.f));
        ui_set_next_pref_height(ui_pixels(20.f));
        ui_set_next_text_padding(4.0f);
        UI_Signal sig = ui_text_edit(str8_lit("##field_b"), color_b->buffer, color_b->buffer_capacity, &color_b->buffer_pos, &color_b->buffer_len);

        if (sig.flags & UI_SIGNAL_FLAG_PRESSED) {
          f32 b = strtof((char *)color_b->buffer, NULL);
          entity->override_color.z = b;
        }
      }
      ui_pop_parent();


      if (ui_clicked(ui_button(str8_lit("Clone")))) {
        Entity_Prototype *prototype = entity_prototype_lookup(editor->selected_entity->prototype_id);
        if (prototype) {
          Entity *new_entity = entity_from_prototype(prototype);
          new_entity->set_position(entity->position);
          new_entity->override_color = editor->selected_entity->override_color;

          editor->select_entity(new_entity);
          get_world()->entities.push(new_entity);
        }
      }

      if (ui_clicked(ui_button(str8_lit("Delete")))) {
        remove_grid_entity(get_world(), editor->selected_entity);
        entity->to_be_destroyed = true;
        editor->select_entity(nullptr);
      }
    }
    ui_pop_parent();

    //@Note Entity Flag Panel
    {
      f32 start = g_viewport->dimension.x - 100.0f;
      ui_set_next_fixed_x(start);
      ui_set_next_fixed_y(400.0f);
      ui_set_next_fixed_width(100.0f);
      ui_set_next_fixed_height(200.0f);
      ui_set_next_child_layout_axis(AXIS_Y);
      ui_push_background_color(Vector4(0.37f, 0.12f, 0.43f, 1.0f));
      UI_Box *flag_panel = ui_box_create(0, str8_lit("##flag_panel"));
      ui_push_parent(flag_panel);
      for (int i = 0; i < ArrayCount(entity_flag_array); i++) {
        Entity_Flags flag = entity_flag_array[i];

        ui_set_next_text_padding(2.0f);
        if (entity->flags & flag) {
          ui_set_next_background_color(Vector4(0.54f, 0.21f, 0.61f, 1.0f)); 
        }

        if (ui_clicked(ui_button(str8_cstring(string_from_entity_flag(flag))))) {
          if (entity->flags & flag) {
            entity->flags = entity->flags & ~flag;
          } else {
            entity->flags |= flag;
          }
        }
      }
      ui_pop_background_color();
      ui_pop_parent();
    }
  }

  //@Note Editor shortcuts
  if (editor->selected_entity) {
    if (key_down(OS_KEY_CONTROL) && key_pressed(OS_KEY_C)) {
      Entity_Prototype *prototype = entity_prototype_lookup(editor->selected_entity->prototype_id);
      if (prototype) {
        Entity *new_entity = entity_from_prototype(prototype);
        new_entity->set_position(editor->selected_entity->position);
        new_entity->override_color = editor->selected_entity->override_color;

        editor->select_entity(new_entity);
        get_world()->entities.push(new_entity);
      }
    }
    if (key_pressed(OS_KEY_DELETE)) {
      remove_grid_entity(get_world(), editor->selected_entity);
      editor->selected_entity->to_be_destroyed = true;
      editor->select_entity(nullptr);
    }
  }
}
