//@Todo UI item expansion

global Editor *g_editor;
global Picker *g_picker;

internal char *string_from_field_kind(Field_Kind kind) {
  switch (kind) {
  case FIELD_INT:
    return "int";
  case FIELD_FLOAT:
    return "float";
  case FIELD_VEC2:
    return "Vector2";
  case FIELD_VEC3:
    return "Vector3";
  case FIELD_VEC4:
    return "Vector4";
  }
  return nullptr;
}

internal inline Editor *get_editor() {
  return g_editor;
}

internal void init_editor() {
  g_editor = new Editor();

  Editor *editor = g_editor;

  editor->gizmo_meshes[GIZMO_TRANSLATE][AXIS_X] = load_mesh("data/meshes/gizmo/translate_x.obj");
  editor->gizmo_meshes[GIZMO_TRANSLATE][AXIS_Y] = load_mesh("data/meshes/gizmo/translate_y.obj");
  editor->gizmo_meshes[GIZMO_TRANSLATE][AXIS_Z] = load_mesh("data/meshes/gizmo/translate_z.obj");
  // editor->gizmo_meshes[GIZMO_ROTATE][AXIS_X]    = load_mesh("data/meshes/gizmo/rotation_x.obj");
  // editor->gizmo_meshes[GIZMO_ROTATE][AXIS_Y]    = load_mesh("data/meshes/gizmo/rotation_y.obj");
  // editor->gizmo_meshes[GIZMO_ROTATE][AXIS_Z]    = load_mesh("data/meshes/gizmo/rotation_z.obj");

  editor->panel = new Editor_Panel();
  editor->panel->icon_font = default_fonts[FONT_ICON];
  editor->panel->expand_load_world = false;
  editor->panel->expand_saveas = false;
  editor->panel->edit_saveas = ui_line_edit_create(str8_lit("Save As"));
  editor->panel->edit_load_world = ui_line_edit_create(str8_lit("Load"));

  editor->entity_panel = new Entity_Panel();
  editor->entity_panel->icon_font = default_fonts[FONT_ICON];

  Entity_Field *position_field = new Entity_Field();
  position_field->name = str8_lit("position");
  position_field->kind = FIELD_VEC3;
  {
    position_field->fields.push(ui_line_edit_create(str8_lit("x")));
    position_field->fields.push(ui_line_edit_create(str8_lit("y")));
    position_field->fields.push(ui_line_edit_create(str8_lit("z")));
  }
  editor->entity_panel->position_field = position_field;
  editor->entity_panel->common_fields.push(position_field);

  Entity_Field *color_field = new Entity_Field();
  color_field->name = str8_lit("override_color");
  color_field->kind = FIELD_VEC4;
  {
    color_field->fields.push(ui_line_edit_create(str8_lit("r")));
    color_field->fields.push(ui_line_edit_create(str8_lit("g")));
    color_field->fields.push(ui_line_edit_create(str8_lit("b")));
    color_field->fields.push(ui_line_edit_create(str8_lit("a")));
  }
  editor->entity_panel->color_field = color_field;
  editor->entity_panel->common_fields.push(color_field);

  Entity_Field *theta_field = new Entity_Field();
  theta_field->name = str8_lit("theta");
  theta_field->kind = FIELD_FLOAT;
  theta_field->fields.push(ui_line_edit_create(str8_lit("Theta")));
  editor->entity_panel->theta_field = theta_field;
  editor->entity_panel->common_fields.push(theta_field);

  Entity_Field *sun_dir = new Entity_Field();
  sun_dir->name = str8_lit("direction");
  sun_dir->expand = 0;
  sun_dir->kind = FIELD_VEC4;
  {
    sun_dir->fields.push(ui_line_edit_create(str8_lit("x")));
    sun_dir->fields.push(ui_line_edit_create(str8_lit("y")));
    sun_dir->fields.push(ui_line_edit_create(str8_lit("z")));
  }
  editor->entity_panel->sun_dir_field = sun_dir;
  editor->entity_panel->entity_fields[ENTITY_SUN].push(sun_dir);
}

internal void update_entity_panel(Editor *editor) {
  Entity_Panel *panel = editor->entity_panel;
  panel->dirty = false;

  Entity *entity = editor->active_selection;
  if (entity) {
    int n = 0;

    n = snprintf((char *)panel->position_field->fields[0]->buffer, panel->position_field->fields[0]->buffer_capacity, "%g", entity->position.x);
    panel->position_field->fields[0]->buffer_pos = panel->position_field->fields[0]->buffer_len = n;

    n = snprintf((char *)panel->position_field->fields[1]->buffer, panel->position_field->fields[1]->buffer_capacity, "%g", entity->position.y);
    panel->position_field->fields[1]->buffer_pos = panel->position_field->fields[1]->buffer_len = n;

    n = snprintf((char *)panel->position_field->fields[2]->buffer, panel->position_field->fields[2]->buffer_capacity, "%g", entity->position.z);
    panel->position_field->fields[2]->buffer_pos = panel->position_field->fields[2]->buffer_len = n;

    n = snprintf((char *)panel->color_field->fields[0]->buffer, panel->color_field->fields[0]->buffer_capacity, "%g", entity->override_color.x);
    panel->color_field->fields[0]->buffer_pos = panel->color_field->fields[0]->buffer_len = n;

    n = snprintf((char *)panel->color_field->fields[1]->buffer, panel->color_field->fields[1]->buffer_capacity, "%g", entity->override_color.y);
    panel->color_field->fields[1]->buffer_pos = panel->color_field->fields[1]->buffer_len = n;

    n = snprintf((char *)panel->color_field->fields[2]->buffer, panel->color_field->fields[2]->buffer_capacity, "%g", entity->override_color.z);
    panel->color_field->fields[2]->buffer_pos = panel->color_field->fields[2]->buffer_len = n;

    n = snprintf((char *)panel->color_field->fields[3]->buffer, panel->color_field->fields[3]->buffer_capacity, "%g", entity->override_color.w);
    panel->color_field->fields[3]->buffer_pos = panel->color_field->fields[3]->buffer_len = n;

    n = snprintf((char *)panel->theta_field->fields[0]->buffer, panel->theta_field->fields[0]->buffer_capacity, "%g", RadToDeg(entity->theta));
    panel->theta_field->fields[0]->buffer_pos = panel->theta_field->fields[0]->buffer_len = n;

    if (entity->kind == ENTITY_SUN) {
      Sun *sun = static_cast<Sun*>(entity);
      n = snprintf((char *)panel->sun_dir_field->fields[0]->buffer, panel->sun_dir_field->fields[0]->buffer_capacity, "%g", sun->light_direction.x);
      panel->sun_dir_field->fields[0]->buffer_pos = panel->sun_dir_field->fields[0]->buffer_len = n;

      n = snprintf((char *)panel->sun_dir_field->fields[1]->buffer, panel->sun_dir_field->fields[1]->buffer_capacity, "%g", sun->light_direction.y);
      panel->sun_dir_field->fields[1]->buffer_pos = panel->sun_dir_field->fields[1]->buffer_len = n;

      n = snprintf((char *)panel->sun_dir_field->fields[2]->buffer, panel->sun_dir_field->fields[2]->buffer_capacity, "%g", sun->light_direction.z);
      panel->sun_dir_field->fields[2]->buffer_pos = panel->sun_dir_field->fields[2]->buffer_len = n;
    }
  }
}

internal inline bool is_selected(Editor *editor, Entity *e) {
  for (int i = 0; i < editor->selections.count; i++) {
    if (editor->selections[i] == e) {
      return true;
    }
  }
  return false;
}

internal inline void clear_selection(Editor *editor) {
  editor->selections.reset_count();
  editor->active_selection = nullptr;
}

internal inline void select_entity(Editor *editor, Entity *e) {
  editor->selections.push(e);
  editor->active_selection = e;
}

internal inline void single_select_entity(Editor *editor, Entity *e) {
  editor->selections.reset_count();
  select_entity(editor, e);
}

internal void deselect_entity(Editor *editor, Entity *e) {
  int idx = 0;
  for (int i = 0; i < editor->selections.count; i++) {
    if (editor->selections[i] == e) {
      idx = i;
      editor->selections.remove(i);
      break;
    }
  }

  idx = ClampTop(idx, (int)editor->selections.count - 1);
  if (editor->active_selection == e && editor->selections.count) {
    editor->active_selection = editor->selections[idx];
  }
}

internal Entity *copy_entity(Entity *e) {
  Entity_Prototype *prototype = entity_prototype_lookup(e->prototype_id);
  Entity *clone = entity_from_prototype(prototype);
  clone->flags = e->flags;
  clone->set_position(e->position);
  clone->theta = e->theta;
  clone->override_color = e->override_color;
  clone->use_override_color = e->use_override_color;

  switch (e->kind) {
  case ENTITY_SUN:
  {
    Sun *sun = static_cast<Sun*>(e);
    Sun *sun_clone = static_cast<Sun*>(clone);
    sun_clone->light_direction = sun->light_direction;
    break;
  }
  }

  get_world()->entities.push(clone);
  return clone;
}

internal void clone_selection(Editor *editor) {
  if (editor->selections.count == 0) return;

  Auto_Array<Entity *> selections;
  array_copy(&selections, editor->selections);

  clear_selection(editor);

  for (int i = 0; i < selections.count; i++) {
    Entity *e = selections[i];
    Entity *clone = copy_entity(e);
    select_entity(editor, clone);
  }

  selections.clear();
}

internal void editor_clone_entity(Editor *editor, Entity *e) {
  if (!e) return;

  Entity *clone = copy_entity(e);
  single_select_entity(editor, clone);
}

internal void r_picker_render_gizmo(Picker *picker) {
  Render_Target *render_target = picker->render_target;
  
  R_D3D11_State *d3d = r_d3d11_state();

  Editor *editor = g_editor;

  set_blend_state(BLEND_STATE_DEFAULT);
  set_depth_state(DEPTH_STATE_DISABLE);
  set_rasterizer_state(RASTERIZER_STATE_NO_CULL);

  d3d->device_context->OMSetRenderTargets(1, (ID3D11RenderTargetView **)&render_target->render_target_view, (ID3D11DepthStencilView*)render_target->depth_stencil_view);

  float clear_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  d3d->device_context->ClearRenderTargetView((ID3D11RenderTargetView*)picker->render_target->render_target_view, clear_color);
  d3d->device_context->ClearDepthStencilView((ID3D11DepthStencilView *)d3d->default_render_target->depth_stencil_view, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

  set_shader(shader_picker);

  bind_uniform(shader_picker, str8_lit("Constants"));

  Entity *e = editor->active_selection;

  f32 gizmo_scale_factor = Abs(length(editor->camera.origin - editor->active_selection->position) * 0.5f);
  gizmo_scale_factor = ClampBot(gizmo_scale_factor, 1.0f);

  for (u32 axis = AXIS_X; axis <= AXIS_Z; axis++) {
    Triangle_Mesh *mesh = editor->gizmo_meshes[editor->active_gizmo][axis];

    Matrix4 world_matrix = translate(e->position) * scale(make_vec3(gizmo_scale_factor));
    Matrix4 view_matrix = editor->camera.view_matrix;
    Matrix4 xform = editor->camera.projection_matrix * view_matrix * world_matrix;

    u32 id = axis;
    Vector4 pick_color;
    pick_color.x = ((id & 0x000000FF) >> 0 ) / 255.0f;
    pick_color.y = ((id & 0x0000FF00) >> 8 ) / 255.0f;
    pick_color.z = ((id & 0x00FF00FF) >> 16) / 255.0f;
    pick_color.w = ((id & 0xFF0000FF) >> 24) / 255.0f;

    set_constant(str8_lit("xform"), xform);
    set_constant(str8_lit("pick_color"), pick_color);
    apply_constants();

    UINT stride = sizeof(Vector3), offset = 0;
    ID3D11Buffer *vertex_buffer = make_vertex_buffer(mesh->vertices.data, mesh->vertices.count, sizeof(Vector3));
    d3d->device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

    d3d->device_context->Draw((UINT)mesh->vertices.count, 0);

    vertex_buffer->Release();
  }

  set_render_target(d3d->default_render_target);
}

internal void picker_render(Picker *picker) {
  R_D3D11_State *d3d = r_d3d11_state();

  Editor *editor = g_editor;

  set_blend_state(BLEND_STATE_DEFAULT);
  set_depth_state(DEPTH_STATE_DEFAULT);

  Render_Target *render_target = picker->render_target;
  d3d->device_context->OMSetRenderTargets(1, (ID3D11RenderTargetView **)&render_target->render_target_view, (ID3D11DepthStencilView *)render_target->depth_stencil_view);

  float clear_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  d3d->device_context->ClearRenderTargetView((ID3D11RenderTargetView *)render_target->render_target_view, clear_color);
  d3d->device_context->ClearDepthStencilView((ID3D11DepthStencilView *)render_target->depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

  set_shader(shader_picker);

  bind_uniform(shader_picker, str8_lit("Constants"));


  Game_State *game_state = get_game_state();
  World *world = get_world();

  for (int i = 0; i < world->entities.count; i++) {
    Entity *e = world->entities[i];
    Triangle_Mesh *mesh = e->mesh;
    if (!mesh) continue;

    Matrix4 rotation_matrix = rotate_rh(e->visual_rotation.y, editor->camera.up);
    Matrix4 world_matrix = translate(e->position) * rotation_matrix;
    Matrix4 view = editor->camera.view_matrix;
    Matrix4 xform = editor->camera.projection_matrix * editor->camera.view_matrix * world_matrix;

    Vector4 pick_color;
    pick_color.x = ((e->id & 0x000000FF) >> 0 ) / 255.0f;
    pick_color.y = ((e->id & 0x0000FF00) >> 8 ) / 255.0f;
    pick_color.z = ((e->id & 0x00FF00FF) >> 16) / 255.0f;
    pick_color.w = ((e->id & 0xFF0000FF) >> 24) / 255.0f;

    set_constant(str8_lit("xform"), xform);
    set_constant(str8_lit("pick_color"), pick_color);
    apply_constants();

    ID3D11Buffer *vertex_buffer = make_vertex_buffer(mesh->vertices.data, mesh->vertices.count, sizeof(Vector3));
    UINT stride = sizeof(Vector3), offset = 0;
    d3d->device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

    for (int triangle_list_idx = 0; triangle_list_idx < mesh->triangle_list_info.count; triangle_list_idx++) {
      Triangle_List_Info triangle_list_info = mesh->triangle_list_info[triangle_list_idx];
      d3d->device_context->Draw((UINT)triangle_list_info.vertices_count, triangle_list_info.first_index);
    }

    vertex_buffer->Release();
  }

  // Draw billboards
  for (int i = 0; i < world->entities.count; i++) {
    Entity *e = world->entities[i];
    if (e->kind == ENTITY_SUN) {
      Matrix4 rotation_matrix = rotate_rh(e->visual_rotation.y, editor->camera.up);
      Matrix4 world_matrix = translate(e->position) * rotation_matrix;
      Matrix4 view = editor->camera.view_matrix;
      Matrix4 xform = editor->camera.projection_matrix * editor->camera.view_matrix * world_matrix;

      Vector4 pick_color;
      pick_color.x = ((e->id & 0x000000FF) >> 0 ) / 255.0f;
      pick_color.y = ((e->id & 0x0000FF00) >> 8 ) / 255.0f;
      pick_color.z = ((e->id & 0x00FF00FF) >> 16) / 255.0f;
      pick_color.w = ((e->id & 0xFF0000FF) >> 24) / 255.0f;

      set_constant(str8_lit("xform"), xform);
      set_constant(str8_lit("pick_color"), pick_color);
      apply_constants();

      Vector3 plane_normal = e->position - editor->camera.origin;
      plane_normal.y = 0.0f;
      plane_normal = normalize(plane_normal);
      
      Vector3 up = Vector3(0, 1, 0);
      Vector3 right = normalize(cross(plane_normal, up));
      f32 size = 0.5f;
      Vector3 p0 = -size * right - size * up;
      Vector3 p1 =  size * right - size * up;
      Vector3 p2 =  size * right + size * up;
      Vector3 p3 = -size * right + size * up;

      Vector3 vertices[6];
      vertices[0] = p0;
      vertices[1] = p1;
      vertices[2] = p2;
      vertices[3] = p0;
      vertices[4] = p2;
      vertices[5] = p3;

      ID3D11Buffer *vertex_buffer = make_vertex_buffer(vertices, ArrayCount(vertices), sizeof(Vector3));
      UINT stride = sizeof(Vector3), offset = 0;
      d3d->device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
      d3d->device_context->Draw((UINT)ArrayCount(vertices), 0);
      vertex_buffer->Release();
    }
  }

  set_render_target(d3d->default_render_target);
}

internal Picker *make_picker(int width, int height) {
  Picker *picker = new Picker();
  picker->dimension = Vector2((f32)width, (f32)height);

  R_D3D11_State *d3d = r_d3d11_state();
  picker->render_target = make_render_target(width, height, DXGI_FORMAT_R8G8B8A8_UNORM);

  HRESULT hr = S_OK;
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
    hr = d3d->device->CreateTexture2D(&staging_desc, NULL, (ID3D11Texture2D **)&picker->staging_texture);
  }

  return picker;
}

internal Pid picker_get_id(Picker *picker, Vector2Int mouse) {
  Pid result = 0xFFFFFFFF;

  R_D3D11_State *d3d = r_d3d11_state();
  D3D11_BOX src_box = {};
  src_box.left   = (UINT)mouse.x;
  src_box.right  = src_box.left + 1;
  src_box.top    = (UINT)mouse.y;
  src_box.bottom = src_box.top + 1;
  src_box.front = 0;
  src_box.back = 1;
  d3d->device_context->CopySubresourceRegion((ID3D11Texture2D *)picker->staging_texture, 0, 0, 0, 0, (ID3D11Texture2D *)picker->render_target->texture, 0, &src_box);

  D3D11_MAPPED_SUBRESOURCE mapped = {};
  if (d3d->device_context->Map((ID3D11Texture2D *)picker->staging_texture, 0, D3D11_MAP_READ, 0, &mapped) == S_OK) {
    u8 *ptr = (u8 *)mapped.pData;
    u8 r = *ptr++;
    u8 g = *ptr++;
    u8 b = *ptr++;
    u8 a = *ptr++;
    result = r | (g << 8) | (b << 16) | (a << 24);

    d3d->device_context->Unmap((ID3D11Texture2D *)picker->staging_texture, 0);
  }

  return result;
}

internal void update_editor() {
  Editor *editor = g_editor;

  R_D3D11_State *d3d = r_d3d11_state();

  World *world = get_world();

  update_camera_position(&editor->camera);

  Vector2Int mouse_position = g_input.mouse_position;

  editor->gizmo_axis_hover = (Axis)-1;
  if (editor->active_selection) {
    r_picker_render_gizmo(g_picker);
    Pid gizmo_id = picker_get_id(g_picker, mouse_position);
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
      Vector3 ray = get_mouse_ray(editor->camera, Vector2Int((int)mouse_position.x, (int)mouse_position.y), g_viewport->dimension);
      Vector3 ray_start = get_mouse_ray(editor->camera, editor->gizmo_mouse_start, g_viewport->dimension);
      Vector3 distance = 15.0f * (ray - ray_start);

      Axis gizmo_axis = editor->gizmo_axis_active;
      Vector3 axis_vector = unit_vector(gizmo_axis);
      Vector3 travel = projection(distance, axis_vector);
      Vector3 meters = floor(travel);

      editor->active_selection->set_position(editor->active_selection->position + meters);
      update_entity_panel(editor);

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
      picker_render(g_picker);
      Pid id = picker_get_id(g_picker, g_input.mouse_position);
      if (id != 0xFFFFFFFF) {
        Entity *e = lookup_entity(id);
        if (key_down(OS_KEY_CONTROL)) {
          if (is_selected(editor, e)) {
            deselect_entity(editor, e);
          } else {
            select_entity(editor, e);
          }
        } else {
          single_select_entity(editor, e);
        }
      } else {
        clear_selection(editor);
      }
    }
  }

  Vector2 camera_delta = get_mouse_right_drag_delta();
  camera_delta = 0.2f * camera_delta;
  update_camera_orientation(&editor->camera, camera_delta);

  Entity *selected_entity = editor->active_selection;

  draw_scene();

  set_shader(shader_mesh);
  bind_uniform(shader_mesh, str8_lit("Constants"));

  set_sampler(str8_lit("diffuse_sampler"), SAMPLER_STATE_LINEAR);
  set_texture(str8_lit("diffuse_texture"), sun_icon_texture);
  set_blend_state(BLEND_STATE_ALPHA);

  for (int i = 0; i < world->entities.count; i++) {
    Entity *e = world->entities[i];
    if (e->kind == ENTITY_SUN) {
      Matrix4 rotation_matrix = rotate_rh(e->theta, editor->camera.up);
      Matrix4 world_matrix = translate(e->position) * rotation_matrix;
      Matrix4 xform = editor->camera.projection_matrix * editor->camera.view_matrix * world_matrix;

      set_constant(str8_lit("xform"), xform);
      apply_constants();
     
      Vector3 right = editor->camera.right;
      Vector3 up = cross(right, editor->camera.forward);
      f32 size = 0.5f;
      Vertex_XCUU p0 = Vertex_XCUU(-size * right - size * up, make_vec4(1.0f), Vector2(0, 1));
      Vertex_XCUU p1 = Vertex_XCUU( size * right - size * up, make_vec4(1.0f), Vector2(1, 1));
      Vertex_XCUU p2 = Vertex_XCUU( size * right + size * up, make_vec4(1.0f), Vector2(1, 0));
      Vertex_XCUU p3 = Vertex_XCUU(-size * right + size * up, make_vec4(1.0f), Vector2(0, 0));

      Vertex_XCUU vertices[6];
      vertices[0] = p0;
      vertices[1] = p1;
      vertices[2] = p2;
      vertices[3] = p0;
      vertices[4] = p2;
      vertices[5] = p3;
 
      ID3D11Buffer *vertex_buffer = make_vertex_buffer(vertices, ArrayCount(vertices), sizeof(Vertex_XCUU));
      UINT stride = sizeof(Vertex_XCUU), offset = 0;
      d3d->device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
      d3d->device_context->Draw((UINT)ArrayCount(vertices), 0);
      vertex_buffer->Release();
    }
  }

  set_blend_state(BLEND_STATE_DEFAULT);

  // Draw selection
  set_shader(shader_mesh);
  bind_uniform(shader_mesh, str8_lit("Constants"));
  set_rasterizer(rasterizer_wireframe);
  set_depth_state(DEPTH_STATE_DEFAULT);

  for (int i = 0; i < editor->selections.count; i++) {
    Entity *e = editor->selections[i];

    Matrix4 rotation_matrix = rotate_rh(e->theta, Vector3(0, 1, 0));
    Matrix4 xform = editor->camera.projection_matrix * editor->camera.view_matrix * translate(e->position) * rotation_matrix;
    set_constant(str8_lit("xform"), xform);
    apply_constants();

    Vector4 strobe_color = (0.5f * editor->select_strobe_t + 0.5f) * Vector4(1, 0, 1, 1);
    if (e->mesh) draw_mesh(e->mesh, strobe_color);
  }
  set_rasterizer(rasterizer_cull_back);

  //@Note Draw Gizmos
  if (editor->active_selection) {
    R_D3D11_State *d3d = r_d3d11_state();

    set_depth_state(DEPTH_STATE_DEFAULT);

    set_rasterizer_state(RASTERIZER_STATE_NO_CULL);

    set_shader(shader_mesh);
    bind_uniform(shader_mesh, str8_lit("Constants"));

    set_sampler(str8_lit("diffuse_sampler"), SAMPLER_STATE_LINEAR);

    f32 gizmo_scale_factor = Abs(length(editor->camera.origin - editor->active_selection->position) * 0.5f);
    gizmo_scale_factor = ClampBot(gizmo_scale_factor, 1.0f);

    Matrix4 world_matrix = translate(selected_entity->position) * scale(make_vec3(gizmo_scale_factor));
    Matrix4 view_matrix = editor->camera.view_matrix;
    Matrix4 xform = editor->camera.projection_matrix * view_matrix * world_matrix;
    set_constant(str8_lit("xform"), xform);
    set_texture(str8_lit("diffuse_texture"), d3d->fallback_tex);
    apply_constants();

    for (int axis = AXIS_X; axis <= AXIS_Z; axis++) {
      Triangle_Mesh *mesh = editor->gizmo_meshes[editor->active_gizmo][axis];

      d3d->device_context->ClearDepthStencilView((ID3D11DepthStencilView *)d3d->default_render_target->depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

      Vector4 color = Vector4(unit_vector(axis), 1.0f);
      if (editor->gizmo_axis_hover == axis && editor->gizmo_axis_active == (Axis)-1) {
        color = mix(color, Vector4(1.0f, 1.0f, 1.0f, 1.0f), 0.4f);
      }
      if (editor->gizmo_axis_active == axis) {
        color = mix(color, Vector4(1.0f, 1.0f, 1.0f, 1.0f), 0.4f);
      }

      Vertex_XCUU *vertices = new Vertex_XCUU[mesh->vertices.count];
      for (int i = 0; i < mesh->vertices.count; i++) {
        Vertex_XCUU vertex = Vertex_XCUU(mesh->vertices[i], color, Vector2());
        vertices[i] = vertex;
      }

      ID3D11Buffer *vertex_buffer = make_vertex_buffer(vertices, mesh->vertices.count, sizeof(Vertex_XCUU));
      UINT stride = sizeof(Vertex_XCUU), offset = 0;
      d3d->device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
      d3d->device_context->Draw((UINT)mesh->vertices.count, 0);
      vertex_buffer->Release();

      delete [] vertices;
    }
  }

  editor->select_strobe_t += (editor->select_strobe_target - editor->select_strobe_t) * get_frame_delta();
  if (Abs(editor->select_strobe_target - editor->select_strobe_t) < 0.1f) {
    editor->select_strobe_t = editor->select_strobe_target;
    if (editor->select_strobe_target > 0.0f) editor->select_strobe_target = 0.0f;
    else editor->select_strobe_target = 1.0f;
  }
}

internal Entity_Prototype *entity_prototype_lookup(u64 prototype_id) {
  Editor *editor = g_editor;
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
  Editor *editor = g_editor;
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
  entity->mesh = prototype->entity.mesh;
  return entity;
}

internal void entity_from_prototype(Entity *entity, Entity_Prototype *prototype) {
  entity->prototype_id = prototype->id;
  entity->kind  = prototype->entity.kind;
  entity->flags = prototype->entity.flags;
  entity->mesh = prototype->entity.mesh;
}

internal void editor_present_ui() {
  Editor *editor = g_editor;
  Editor_Panel *panel = editor->panel;
  Entity_Panel *entity_panel = editor->entity_panel;

  if (entity_panel->dirty) {
    update_entity_panel(editor); 
  }
  
  ui_set_next_pref_width(ui_pixels(110.f));
  ui_set_next_pref_height(ui_pixels(300.f));
  UI_Box *column = ui_box_create(UI_BOX_FLAG_DRAW_BACKGROUND, str8_lit("##column"));
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
    UI_Signal sig = ui_line_edit(panel->edit_saveas, str8_lit("##saveas_edit"));
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
    UI_Signal sig = ui_line_edit(panel->edit_load_world, str8_lit("##load_edit"));
    if ((sig.flags & UI_SIGNAL_FLAG_PRESSED) && sig.key == OS_KEY_ENTER) {
      String8 name = str8(panel->edit_load_world->buffer, panel->edit_load_world->buffer_len);
      load_world(name);
      editor->panel->expand_load_world = false;
    }
  }

  ui_set_next_pref_width(ui_txt(2.0f));
  ui_set_next_pref_height(ui_txt(4.0f));
  if (ui_clicked(ui_button(str8_lit("New")))) {
    World *world = new World();
    set_world(world);
  }

  ui_pop_parent();

  //@Note Entity Panel
  {
    Entity *e = editor->active_selection;
    Entity_Panel *panel = editor->entity_panel;

    f32 field_height = 26.0f;
    f32 panel_width = 280.0f;
    f32 panel_x = g_viewport->dimension.x - panel_width;
    f32 panel_y = 0;
    Vector4 panel_color = Vector4(0.24f, 0.86f, 0.59f, 1.0f);
    // Vector4 panel_color = Vector4(0.16f, 0.89f, 0.76f, 1.0f);

    ui_set_next_fixed_x(panel_x);
    ui_set_next_fixed_y(panel_y);
    ui_set_next_fixed_width(panel_width);
    ui_set_next_pref_height(ui_children_sum());
    ui_set_next_child_layout_axis(AXIS_Y);
    ui_set_next_background_color(panel_color);
    UI_Box *panel_box = ui_box_create(UI_BOX_FLAG_DRAW_BACKGROUND, str8_lit("##entity_panel"));
    UI_Parent(panel_box)
      UI_BackgroundColor(panel_color)
      UI_BorderColor(Vector4(0.01f, 0.49f, 0.36f, 1.0f))
      UI_TextColor(Vector4(0.19f, 0.15f, 0.33f, 1.0f))
    {
      // Header
      ui_set_next_pref_width(ui_pct(1.0f));
      if (e) {
        ui_textf("Entity: %s #%llu", string_from_entity_kind(e->kind), e->id);
      } else {
        ui_textf("Entity: (none)");
      }

      if (e) {
        ui_set_next_child_layout_axis(AXIS_X);
        ui_set_next_pref_width(ui_pct(1.0f));
        ui_set_next_pref_height(ui_pixels(field_height * 1.5f));
        UI_Box *tab_header = ui_box_create(0, str8_lit("##tab_header"));

        UI_Parent(tab_header)
          UI_PrefWidth(ui_pixels(panel_width*0.33f))
          UI_PrefHeight(ui_pct(1.0f))
        {
          UI_Box *tab_one = ui_box_create(UI_BOX_FLAG_CLICKABLE | UI_BOX_FLAG_TEXT_ELEMENT | UI_BOX_FLAG_DRAW_BORDER, str8_lit("1##One"));
          UI_Box *tab_common = ui_box_create(UI_BOX_FLAG_CLICKABLE | UI_BOX_FLAG_TEXT_ELEMENT | UI_BOX_FLAG_DRAW_BORDER, str8_lit("Common"));
          UI_Box *tab_flags = ui_box_create(UI_BOX_FLAG_CLICKABLE | UI_BOX_FLAG_TEXT_ELEMENT | UI_BOX_FLAG_DRAW_BORDER, str8_lit("Flags"));

          if (ui_clicked(ui_signal_from_box(tab_one))) {
            panel->active_tab = ENTITY_TAB_FIRST;
          }

          if (ui_clicked(ui_signal_from_box(tab_common))) {
            panel->active_tab = ENTITY_TAB_COMMON;
          }

          if (ui_clicked(ui_signal_from_box(tab_flags))) {
            panel->active_tab = ENTITY_TAB_FLAGS;
          }
        }
      }

      // New Entity
      if (!e) {
        ui_set_next_pref_width(ui_pct(1.0f));
        ui_set_next_pref_height(ui_pixels(field_height));
        // ui_set_next_background_color(Vector4(1.f, 0.f, 0.f, 1.f));
        ui_set_next_child_layout_axis(AXIS_X);
        UI_Box *etype_panel = ui_box_create(UI_BOX_FLAG_DRAW_BACKGROUND, str8_lit("##etype"));
        UI_Parent(etype_panel)
        {
          ui_set_next_font(panel->icon_font);
          UI_Box *left_arrow = ui_box_create(UI_BOX_FLAG_CLICKABLE | UI_BOX_FLAG_TEXT_ELEMENT | UI_BOX_FLAG_DRAW_BACKGROUND, ui_string_from_icon_kind(UI_ICON_TRIANGLE_LEFT, "##arrow_left"));
          UI_Signal left_sig = ui_signal_from_box(left_arrow);
          if (ui_clicked(left_sig)) {
            editor->prototype_idx--;
            editor->prototype_idx = range_clamp(editor->prototype_idx, 0, (int)editor->prototype_array.count - 1);
          }

          ui_set_next_pref_width(ui_pixels(panel_width*0.8f));
          Entity_Prototype *prototype = editor->prototype_array[editor->prototype_idx];
          ui_textf("%s", prototype->name);

          ui_set_next_font(panel->icon_font);
          UI_Box *right_arrow = ui_box_create(UI_BOX_FLAG_CLICKABLE | UI_BOX_FLAG_TEXT_ELEMENT | UI_BOX_FLAG_DRAW_BACKGROUND, ui_string_from_icon_kind(UI_ICON_TRIANGLE_RIGHT, "##arrow_right"));
          UI_Signal right_sig = ui_signal_from_box(right_arrow);
          if (ui_clicked(right_sig)) {
            editor->prototype_idx++;
            editor->prototype_idx = range_clamp(editor->prototype_idx, 0, (int)editor->prototype_array.count - 1);
          }
        }

        ui_set_next_pref_width(ui_pct(1.0f));
        if (ui_clicked(ui_button(str8_lit("Instantiate")))) {
          Entity_Prototype *prototype = editor->prototype_array[editor->prototype_idx];
          Entity *instance = entity_from_prototype(prototype);
          instance->set_position(floor(editor->camera.origin + editor->camera.forward));
          if (editor->active_selection) {
            Vector3 unit = get_nearest_axis(editor->camera.origin - editor->active_selection->position);
            instance->set_position(editor->active_selection->position + unit);
          }
          single_select_entity(editor, instance);
          World *world = get_world();
          world->entities.push(instance);
          if (instance->kind == ENTITY_GUY) {
            world->guy = static_cast<Guy*>(instance);
          }
        }
      }

      if (e && panel->active_tab == ENTITY_TAB_FIRST) {
        if (e->kind == ENTITY_SUN) {
          Sun *sun = static_cast<Sun*>(e);
          Auto_Array<Entity_Field*> fields = panel->entity_fields[ENTITY_SUN];
          for (int i = 0; i < fields.count; i++) {
            Entity_Field *field = fields[i];
            bool is_compound = field->fields.count > 1;

            ui_set_next_pref_width(ui_pct(1.0f));
            ui_set_next_pref_height(ui_pixels(field_height));
            ui_set_next_child_layout_axis(AXIS_X);
            int flags = UI_BOX_FLAG_CLICKABLE | UI_BOX_FLAG_DRAW_BORDER;
            if (is_compound) flags |= UI_BOX_FLAG_CLICKABLE;
            UI_Box *field_container = ui_box_create_format(flags, "##field_%d", i);
            UI_Signal field_cont_sig = ui_signal_from_box(field_container);

            UI_Parent(field_container)
              UI_PrefWidth(ui_pct(0.5f))
              UI_PrefHeight(ui_pct(1.0f))
            {
              ui_text(field->name);

              if (is_compound) {
                ui_textf("(%s)", string_from_field_kind(field->kind));
                if (ui_clicked(field_cont_sig)) {
                  field->expand = !field->expand;
                }
              } else {
                UI_Line_Edit *line_edit = field->fields[0];
                // ui_set_next_text_padding(4.0f);
                UI_Signal sig = ui_line_edit(line_edit, str8_lit("##edit"));
                if (ui_pressed(sig) && sig.key == OS_KEY_ENTER) {
                  printf("ENTER\n");
                  field->dirty = true;
                  editor->entity_panel->dirty = true;
                }
              }
            }
            if (is_compound && field->expand) {
              for (int j = 0; j < field->fields.count; j++) {
                ui_set_next_pref_width(ui_pct(1.0f));
                ui_set_next_pref_height(ui_pixels(field_height));
                ui_set_next_child_layout_axis(AXIS_X);
                UI_Box *cont = ui_box_create(UI_BOX_FLAG_DRAW_BORDER, 0);

                UI_Parent(cont)
                  UI_PrefWidth(ui_pct(0.5f))
                  UI_PrefHeight(ui_pixels(field_height))
                {
                  UI_Line_Edit *line_edit = field->fields[j];
                  ui_text(line_edit->name);
              
                  // ui_set_next_text_padding(4.0f);
                  UI_Signal sig = ui_line_editf(line_edit, "##edit_%d%d", i, j);
                  if (ui_pressed(sig) && sig.key == OS_KEY_ENTER) {
                    field->dirty = true;
                    editor->entity_panel->dirty = true;
                  }
                }
              }
            }
          }
        }
      }

      // Common Fields
      if (e && panel->active_tab == ENTITY_TAB_COMMON) {
        for (int i = 0; i < panel->common_fields.count; i++) {
          Entity_Field *field = panel->common_fields[i];
          bool is_compound = field->fields.count > 1;

          ui_set_next_pref_width(ui_pct(1.0f));
          ui_set_next_pref_height(ui_pixels(field_height));
          ui_set_next_child_layout_axis(AXIS_X);
          int flags = UI_BOX_FLAG_CLICKABLE | UI_BOX_FLAG_DRAW_BORDER;
          if (is_compound) flags |= UI_BOX_FLAG_CLICKABLE;
          UI_Box *field_container = ui_box_create_format(flags, "##field_%d", i);
          UI_Signal field_cont_sig = ui_signal_from_box(field_container);

          UI_Parent(field_container)
            UI_PrefWidth(ui_pct(0.5f))
            UI_PrefHeight(ui_pct(1.0f))
          {
            ui_text(field->name);

            if (is_compound) {
              ui_textf("(%s)", string_from_field_kind(field->kind));
              if (ui_clicked(field_cont_sig)) {
                field->expand = !field->expand;
              }
            } else {
              UI_Line_Edit *line_edit = field->fields[0];
              // ui_set_next_text_padding(4.0f);
              UI_Signal sig = ui_line_edit(line_edit, str8_lit("##edit"));
              if (ui_pressed(sig) && sig.key == OS_KEY_ENTER) {
                printf("ENTER\n");
                field->dirty = true;
                editor->entity_panel->dirty = true;
              }
            }
          }
          if (is_compound && field->expand) {
            for (int j = 0; j < field->fields.count; j++) {
              ui_set_next_pref_width(ui_pct(1.0f));
              ui_set_next_pref_height(ui_pixels(field_height));
              ui_set_next_child_layout_axis(AXIS_X);
              UI_Box *cont = ui_box_create(UI_BOX_FLAG_DRAW_BORDER, 0);

              UI_Parent(cont)
                UI_PrefWidth(ui_pct(0.5f))
                UI_PrefHeight(ui_pixels(field_height))
              {
                UI_Line_Edit *line_edit = field->fields[j];
                ui_text(line_edit->name);
              
                // ui_set_next_text_padding(4.0f);
                UI_Signal sig = ui_line_editf(line_edit, "##edit_%d%d", i, j);
                if (ui_pressed(sig) && sig.key == OS_KEY_ENTER) {
                  field->dirty = true;
                  editor->entity_panel->dirty = true;
                }
              }
            }
          }
        }

        // Override Color
        {
          UI_Icon_Kind button_icon = e->use_override_color ? UI_ICON_CHECK : UI_ICON_CHECK_EMPTY;
          ui_set_next_font(panel->icon_font);
          ui_set_next_pref_width(ui_pct(1.0f));
          if (ui_clicked(ui_button(ui_string_from_icon_kind(button_icon, "##overbtn")))) {
            e->use_override_color = !e->use_override_color;
          }
        }

        if (ui_clicked(ui_button(str8_lit("Clone")))) {
          editor_clone_entity(editor, editor->active_selection);
        }

        if (ui_clicked(ui_button(str8_lit("Destroy!")))) {
          remove_grid_entity(get_world(), editor->active_selection);
          e->to_be_destroyed = true;
          deselect_entity(editor, e);
        }
      }

      // Flags
      if (e && panel->active_tab == ENTITY_TAB_FLAGS) {
        for (int i = 0; i < ArrayCount(entity_flag_array); i++) {
          Entity_Flags flag = entity_flag_array[i];
          ui_set_next_pref_width(ui_pct(1.0f));
          // inactive flag
          if (~(e->flags) & flag) {
            ui_set_next_background_color(Vector4(0.36f, 0.7f, 0.55f, 0.5f));
            ui_set_next_box_flags(UI_BOX_FLAG_DRAW_BACKGROUND);
          }

          if (ui_clicked(ui_button(str8_cstring(string_from_entity_flag(flag))))) {
            if (e->flags & flag) {
              e->flags = e->flags & (~flag);
            } else {
              e->flags |= flag;
            }
          }
        }
      }
    }
  }

  //@Note Update enttiy on field edit
  if (editor->active_selection) {
    Entity_Panel *panel = editor->entity_panel;
    Entity *e = editor->active_selection;

    if (panel->position_field->dirty) {
      panel->position_field->dirty = false;
      Vector3 p;
      p.x = strtof((char *)panel->position_field->fields[0]->buffer, nullptr); 
      p.y = strtof((char *)panel->position_field->fields[1]->buffer, nullptr); 
      p.z = strtof((char *)panel->position_field->fields[2]->buffer, nullptr); 
      e->set_position(p);
    }

    if (panel->color_field->dirty) {
      panel->color_field->dirty = false;
      e->override_color.x = strtof((char *)panel->color_field->fields[0]->buffer, nullptr); 
      e->override_color.y = strtof((char *)panel->color_field->fields[1]->buffer, nullptr); 
      e->override_color.z = strtof((char *)panel->color_field->fields[2]->buffer, nullptr); 
      e->override_color.w = strtof((char *)panel->color_field->fields[3]->buffer, nullptr); 
    }

    if (panel->theta_field->dirty) {
      panel->theta_field->dirty = false;
      f32 theta = strtof((char *)panel->theta_field->fields[0]->buffer, nullptr);
      theta = DegToRad(theta);
      e->set_theta(theta);
    }

    if (e->kind == ENTITY_SUN) {
      Sun *sun = static_cast<Sun*>(e);

      if (panel->sun_dir_field->dirty) {
        panel->sun_dir_field->dirty = false;
        sun->light_direction.x = strtof((char *)panel->sun_dir_field->fields[0]->buffer, nullptr); 
        sun->light_direction.y = strtof((char *)panel->sun_dir_field->fields[1]->buffer, nullptr); 
        sun->light_direction.z = strtof((char *)panel->sun_dir_field->fields[2]->buffer, nullptr); 
      }
    }
  }

  //@Note Editor shortcuts
  if (editor->active_selection) {
    Entity *e = editor->active_selection;
    if (key_down(OS_KEY_CONTROL) && key_pressed(OS_KEY_C)) {
      clone_selection(editor);
    }
    if (key_pressed(OS_KEY_DELETE)) {
      for (int i = 0; i < editor->selections.count; i++) {
        Entity *e = editor->selections[i];
        e->to_be_destroyed = true;
        remove_grid_entity(get_world(), e);
      }
      clear_selection(editor);
    }

    Vector3 up = Vector3(0, 1, 0);
    Vector3 forward = editor->camera.forward;
    forward.y = 0;
    forward = get_nearest_axis(forward);
    Vector3 right = normalize(cross(forward, up));

    Vector3 move_distance = Vector3();

    if (key_down(OS_KEY_CONTROL) && !key_down(OS_KEY_SHIFT) && key_pressed(OS_KEY_UP)) {
      editor->entity_panel->dirty = true;
      move_distance += forward;
    }
    if (key_down(OS_KEY_CONTROL) && !key_down(OS_KEY_SHIFT) && key_pressed(OS_KEY_DOWN)) {
      editor->entity_panel->dirty = true;
      move_distance -= forward;
    }
    if (key_down(OS_KEY_CONTROL) && key_pressed(OS_KEY_RIGHT)) {
      editor->entity_panel->dirty = true;
      move_distance += right;
    }
    if (key_down(OS_KEY_CONTROL) && key_pressed(OS_KEY_LEFT)) {
      editor->entity_panel->dirty = true;
      move_distance -= right;
    }
    if (key_down(OS_KEY_CONTROL) && key_down(OS_KEY_SHIFT) && key_pressed(OS_KEY_UP)) {
      editor->entity_panel->dirty = true;
      move_distance += up;
    }
    if (key_down(OS_KEY_CONTROL) && key_down(OS_KEY_SHIFT) && key_pressed(OS_KEY_DOWN)) {
      editor->entity_panel->dirty = true;
      move_distance -= up;
    }

    if (length(move_distance) > 0.0f) {
      for (int i = 0; i < editor->selections.count; i++) {
        Entity *e = editor->selections[i];
        e->set_position(e->position + move_distance);
      }
    }

    if (e->kind == ENTITY_SUN) {
      if (key_down(OS_KEY_CONTROL) && key_pressed(OS_KEY_L)) {
        Sun *sun = static_cast<Sun*>(e);
        sun->light_direction = editor->camera.forward;
      }
    }
  }
}
