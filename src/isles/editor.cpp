
global Editor *g_editor;
global Picker *g_picker;

global Key_Map *editor_key_map;

std::unordered_map<u64,Key_Command> editor_command_table;

#define SIGN(x) (x > 0 ? 1 : (x < 0 ? -1 : 0))
#define FRAC0(x) (x - floor(x))
#define FRAC1(x) (1 - x + floor(x))

internal bool raycast(Vector3 origin, Vector3 direction, f32 max_d, Raycast *raycast) {
  // raycast->ray = make_ray(origin, direction);
  // raycast->block = block_id_zero();
  raycast->hit = NULL;
  raycast->hit_position = {0, 0, 0};
  raycast->face = FACE_TOP;

  f64 tMaxX, tMaxY, tMaxZ, tDeltaX, tDeltaY, tDeltaZ;
  Vector3Int block;

  f64 x1, y1, z1; // start point   
  f64 x2, y2, z2; // end point   

  x1 = origin.x;
  y1 = origin.y;
  z1 = origin.z;
  x2 = x1 + (f64)(direction.x * max_d);
  y2 = y1 + (f64)(direction.y * max_d);
  z2 = z1 + (f64)(direction.z * max_d);

  int dx = SIGN(x2 - x1);
  if (dx != 0) tDeltaX = fmin(dx / (x2 - x1), 10000000.0); else tDeltaX = 10000000.0;
  if (dx > 0) tMaxX = tDeltaX * FRAC1(x1); else tMaxX = tDeltaX * FRAC0(x1);
  block.x = (int)floor(x1);

  int dy = SIGN(y2 - y1);
  if (dy != 0) tDeltaY = fmin(dy / (y2 - y1), 10000000.0); else tDeltaY = 10000000.0;
  if (dy > 0) tMaxY = tDeltaY * FRAC1(y1); else tMaxY = tDeltaY * FRAC0(y1);
  block.y = (int)floor(y1);

  int dz = SIGN(z2 - z1);
  if (dz != 0) tDeltaZ = fmin(dz / (z2 - z1), 10000000.0); else tDeltaZ = 10000000.0;
  if (dz > 0) tMaxZ = tDeltaZ * FRAC1(z1); else tMaxZ = tDeltaZ * FRAC0(z1);
  block.z = (int)floor(z1);

  // logprint("dir: %.2f %.2f %.2f\n", direction.x, direction.y, direction.z);
  // logprint("dstep: %d %d %d\n", dx, dy, dz);
  // logprint("origin: %.2f %.2f %.2f\n", origin.x, origin.y, origin.z);
  // logprint("start: %d %d %d\n", block.x, block.y, block.z);

  while (true) {
    if (tMaxX < tMaxY) {
      if (tMaxX < tMaxZ) {
        block.x += dx;
        tMaxX += tDeltaX;
        raycast->face = (dx < 0) ? FACE_EAST : FACE_WEST;
      } else {
        block.z += dz;
        tMaxZ += tDeltaZ;
        raycast->face = (dz < 0) ? FACE_NORTH : FACE_SOUTH;
      }
    } else {
      if (tMaxY < tMaxZ) {
        block.y += dy;
        tMaxY += tDeltaY;
        raycast->face = (dy < 0) ? FACE_TOP : FACE_BOTTOM;
      } else {
        block.z += dz;
        tMaxZ += tDeltaZ;
        raycast->face = (dz < 0) ? FACE_NORTH : FACE_SOUTH;
      }
    }

    raycast->hit_position.x = (f32)block.x;
    raycast->hit_position.y = (f32)block.y;
    raycast->hit_position.z = (f32)block.z;

    if (tMaxX > 1 && tMaxY > 1 && tMaxZ > 1) break;

    Entity *entity = find_entity_at(to_vec3(block));
    if (entity) {
      raycast->hit = entity;
      return true;
    }

    if (block.y == 0.0f) return false;
  }

  return false;
}

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
  editor->entity_panel->dirty = true;
  editor->entity_panel->override_color = e->override_color;
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
  Entity_Manager *manager = get_entity_manager();

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

  entity_push(manager, clone);
  return clone;
}

internal void editor_clone_entity(Editor *editor, Entity *e) {
  if (!e) return;

  Entity *clone = copy_entity(e);
  single_select_entity(editor, clone);
}

internal void get_move_vectors(Camera camera, Vector3 *forward, Vector3 *right) {
  Vector3 U = Vector3(0, 1, 0);
  Vector3 F = camera.forward;
  F.y = 0;
  F = get_nearest_axis(F);
  Vector3 R = normalize(cross_product(F, U));
  *forward = F;
  *right = R;
}

COMMAND(EditorMoveLeft) {
  Editor *editor = get_editor();
  editor->entity_panel->dirty = true;

  Vector3 forward, right;
  get_move_vectors(editor->camera, &forward, &right);

  for (int i = 0; i < editor->selections.count; i++) {
    Entity *e = editor->selections[i];
    e->set_position(e->position - right);
  }
}

COMMAND(EditorMoveRight) {
  Editor *editor = get_editor();
  editor->entity_panel->dirty = true;

  Vector3 forward, right;
  get_move_vectors(editor->camera, &forward, &right);

  for (int i = 0; i < editor->selections.count; i++) {
    Entity *e = editor->selections[i];
    e->set_position(e->position + right);
  }
}

COMMAND(EditorMoveUp) {
  Editor *editor = get_editor();
  editor->entity_panel->dirty = true;

  for (int i = 0; i < editor->selections.count; i++) {
    Entity *e = editor->selections[i];
    e->set_position(e->position + Vector3(0, 1, 0));
  }
}

COMMAND(EditorMoveDown) {
  Editor *editor = get_editor();
  editor->entity_panel->dirty = true;

  for (int i = 0; i < editor->selections.count; i++) {
    Entity *e = editor->selections[i];
    e->set_position(e->position - Vector3(0, 1, 0));
  }
}

COMMAND(EditorMoveForward) {
  Editor *editor = get_editor();
  editor->entity_panel->dirty = true;

  Vector3 forward, right;
  get_move_vectors(editor->camera, &forward, &right);

  for (int i = 0; i < editor->selections.count; i++) {
    Entity *e = editor->selections[i];
    e->set_position(e->position + forward);
  }
}

COMMAND(EditorMoveBackward) {
  Editor *editor = get_editor();
  editor->entity_panel->dirty = true;

  Vector3 forward, right;
  get_move_vectors(editor->camera, &forward, &right);

  for (int i = 0; i < editor->selections.count; i++) {
    Entity *e = editor->selections[i];
    e->set_position(e->position - forward);
  }
}

COMMAND(EditorCopySelection) {
  Editor *editor = get_editor();
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

COMMAND(EditorDeleteSelection) {
  Editor *editor = get_editor();
  for (int i = 0; i < editor->selections.count; i++) {
    Entity *e = editor->selections[i];
    e->to_be_destroyed = true;
  }
  clear_selection(editor);
}

COMMAND(EditorClearSelection) {
  Editor *editor = get_editor();
  clear_selection(editor);
}

COMMAND(EditorMultiSelect) {
  Editor *editor = get_editor();
  if (editor->hover_entity) {
    if (is_selected(editor, editor->hover_entity)) {
      deselect_entity(editor, editor->hover_entity);
    } else {
      select_entity(editor, editor->hover_entity);
    }
  }
}

COMMAND(EditorSelect) {
  Editor *editor = get_editor();
  if (editor->hover_entity) {
    single_select_entity(editor, editor->hover_entity);
  }
}

COMMAND(EditorSelectRange) {
  Editor *editor = get_editor();
  if (editor->active_selection && editor->hover_entity) {
    Entity *start = editor->active_selection;
    Entity *end = editor->hover_entity;
    AABB range_box = make_aabb_from_corners(start->position, end->position);

    for (f32 z = range_box.min.z; z <= range_box.max.z; z++) {
      for (f32 y = range_box.min.y; y <= range_box.max.y; y++) {
        for (f32 x = range_box.min.x; x <= range_box.max.x; x++) {
          Entity *entity = find_entity_at(Vector3(x, y, z));
          if (entity && !is_selected(editor, entity)) {
            select_entity(editor, entity);
          }
        }
      }
    }
  }
}

internal void insert_key_map_command(String8 name, Key_Proc *proc) {
  Key_Command command;
  command.name = name;
  command.proc = proc;
  u64 hash = djb2_hash_string(name);
  editor_command_table.insert({hash, command});
}

internal void init_editor_key_map() {
  editor_key_map = create_key_map(str8_lit("Editor"));

  insert_key_map_command(str8_lit("EditorMoveLeft"), EditorMoveLeft);
  insert_key_map_command(str8_lit("EditorMoveRight"), EditorMoveRight);
  insert_key_map_command(str8_lit("EditorMoveForward"), EditorMoveForward);
  insert_key_map_command(str8_lit("EditorMoveBackward"), EditorMoveBackward);
  insert_key_map_command(str8_lit("EditorMoveUp"), EditorMoveUp);
  insert_key_map_command(str8_lit("EditorMoveDown"), EditorMoveDown);
  insert_key_map_command(str8_lit("EditorCopySelection"), EditorCopySelection);
  insert_key_map_command(str8_lit("EditorDeleteSelection"), EditorDeleteSelection);
  insert_key_map_command(str8_lit("EditorClearSelection"), EditorClearSelection);
  insert_key_map_command(str8_lit("EditorSelect"), EditorSelect);
  insert_key_map_command(str8_lit("EditorMultiSelect"), EditorMultiSelect);
  insert_key_map_command(str8_lit("EditorSelectRange"), EditorSelectRange);

  OS_Handle file_handle = os_open_file(str8_lit("data/Editor.keymap"), OS_AccessFlag_Read);
  if (!os_valid_handle(file_handle)) {
    return;
  }

  String8 file = os_read_file_string(file_handle);
  os_close_handle(file_handle);

  Lexer *lexer = init_lexer(file);

  for (;;) {
    if (lexer->token.kind == TOKEN_EOF) break;

    Key key = 0;
    String8 command_name = str8_zero();

    while (lexer->token.kind != TOKEN_COLON2 && lexer->token.kind != TOKEN_EOF) {
      if (lexer->token.kind == TOKEN_NAME) {
        Token name = lexer->token;
        next_token(lexer);

        bool is_prefix = false;
        if (lexer->token.kind == TOKEN_MINUS) is_prefix = true;

        if (is_prefix) {
          if (str8_equal(name.name, str8_lit("C"))) {
            key |= KEYMOD_CONTROL;
          } else if (str8_equal(name.name, str8_lit("A"))) {
            key |= KEYMOD_ALT;
          } else if (str8_equal(name.name, str8_lit("S"))) {
            key |= KEYMOD_SHIFT;
          } else {
            logprint("Invalid prefix '%S'.\n", name.name);
          }
        } else {
          auto it = key_names_table.find(djb2_hash_string(name.name));
          if (it != key_names_table.end()) {
            Key named_key = get_key_name(name.name);
            key |= named_key;
          } else {
            logprint("Invalid key '%S'.\n", name.name);
          }
        }

        if (lexer->token.kind == TOKEN_MINUS) next_token(lexer);

      } else {
        logprint("Unexpected token.\n");
        next_token(lexer);
        break;
      }
    }

    if (lexer->token.kind == TOKEN_COLON2) {
      next_token(lexer);

      if (lexer->token.kind == TOKEN_NAME) {
        command_name = lexer->token.name;
      } else {
        logprint("Bad token.\n");
      }
      next_token(lexer);
    }

    if (key && command_name.count > 0) {
      u64 hash = djb2_hash_string(command_name);
      auto it = editor_command_table.find(hash);
      if (it != editor_command_table.end()) {
        Key_Command command = it->second;
        key_map_bind(editor_key_map, key, command);
      } else {
        logprint("Command '%S' not found.\n", command_name);
      }
    }
  }
}

internal void init_editor() {
  g_editor = new Editor();
  Editor *editor = g_editor;

  init_editor_key_map();

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

  editor->select_strobe_t = 0;
  editor->select_strobe_max = 0.8f;
  editor->select_strobe_target = editor->select_strobe_max;
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

internal void r_picker_render_gizmo(Picker *picker) {
  Render_Target *render_target = picker->render_target;
  
  R_D3D11_State *d3d = r_d3d11_state();

  Editor *editor = g_editor;

  set_blend_state(blend_state_default);
  set_depth_state(depth_state_disable);
  set_rasterizer(rasterizer_no_cull);

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

    Matrix4 world_matrix = translate(e->position) * translate(e->offset) * scale(make_vec3(gizmo_scale_factor));
    Matrix4 xform = editor->camera.transform * world_matrix;

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
  Entity_Manager *manager = get_entity_manager();

  R_D3D11_State *d3d = r_d3d11_state();

  Editor *editor = g_editor;

  set_blend_state(blend_state_default);
  set_depth_state(depth_state_default);

  Render_Target *render_target = picker->render_target;
  d3d->device_context->OMSetRenderTargets(1, (ID3D11RenderTargetView **)&render_target->render_target_view, (ID3D11DepthStencilView *)render_target->depth_stencil_view);

  float clear_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  d3d->device_context->ClearRenderTargetView((ID3D11RenderTargetView *)render_target->render_target_view, clear_color);
  d3d->device_context->ClearDepthStencilView((ID3D11DepthStencilView *)render_target->depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

  set_shader(shader_picker);

  bind_uniform(shader_picker, str8_lit("Constants"));


  Game_State *game_state = get_game_state();
  World *world = get_world();

  for (Entity *e : manager->entities) {
    Triangle_Mesh *mesh = e->mesh;
    if (!mesh) continue;

    Matrix4 rotation_matrix = rotate_rh(e->theta, editor->camera.up);
    Matrix4 world_matrix = translate(e->position) * translate(e->offset) * rotation_matrix;
    Matrix4 xform = editor->camera.transform * world_matrix;

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

  for (Sun *sun : manager->by_type._Sun) {
    Matrix4 rotation_matrix = rotate_rh(sun->theta, editor->camera.up);
    Matrix4 world_matrix = translate(sun->position) * translate(sun->offset) * rotation_matrix;
    Matrix4 xform = editor->camera.transform * world_matrix;

    Vector4 pick_color;
    pick_color.x = ((sun->id & 0x000000FF) >> 0 ) / 255.0f;
    pick_color.y = ((sun->id & 0x0000FF00) >> 8 ) / 255.0f;
    pick_color.z = ((sun->id & 0x00FF00FF) >> 16) / 255.0f;
    pick_color.w = ((sun->id & 0xFF0000FF) >> 24) / 255.0f;

    set_constant(str8_lit("xform"), xform);
    set_constant(str8_lit("pick_color"), pick_color);
    apply_constants();

    Vector3 plane_normal = sun->position - editor->camera.origin;
    plane_normal.y = 0.0f;
    plane_normal = normalize(plane_normal);
      
    Vector3 up = Vector3(0, 1, 0);
    Vector3 right = normalize(cross_product(plane_normal, up));
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

internal void update_editor(OS_Event_List *events) {
  Entity_Manager *manager = get_entity_manager();
  Editor *editor = g_editor;

  R_D3D11_State *d3d = r_d3d11_state();

  World *world = get_world();

  // Camera
  update_camera_position(&editor->camera);
  Vector2 camera_delta = get_mouse_right_drag_delta();
  camera_delta = 0.2f * camera_delta;
  update_camera_orientation(&editor->camera, camera_delta);

  // Picking
  {
    Vector3 origin = editor->camera.origin;
    Vector3 mouse_ray = get_mouse_ray(editor->camera, g_input.mouse_position, get_viewport()->dimension);
    Raycast raycast_result = {};
    if (raycast(origin, mouse_ray, 100.0f, &raycast_result)) {
      editor->hover_entity = raycast_result.hit;
    } else {
      editor-> hover_entity = nullptr; 
    }
  }

  for (OS_Event *evt = events->first; evt; evt = evt->next) {
    bool pressed = false;

    Key key = 0;
    Key_Mod mods = 0;
    if (evt->flags & OS_EventFlag_Control) mods |= KEYMOD_CONTROL;
    if (evt->flags & OS_EventFlag_Alt)     mods |= KEYMOD_ALT;
    if (evt->flags & OS_EventFlag_Shift)   mods |= KEYMOD_SHIFT;

    switch (evt->kind) {
    default:
      break;
    case OS_EventKind_MouseDown:
    case OS_EventKind_Press:
      pressed = true;
    case OS_EventKind_Release:
    case OS_EventKind_MouseUp:
    {
      key = make_key((u16)evt->key, mods);
      break;
    }
    }

    if (key && pressed) {
      Key_Command *command = key_map_lookup(editor_key_map, key);
      if (command->proc && !ui_mouse_captured() && !ui_keyboard_captured()) {
        command->proc();
      }
    }
  }

  Vector2Int mouse_position = g_input.mouse_position;

  editor->gizmo_axis_hover = (Axis)-1;
  if (editor->active_selection) {
    // r_picker_render_gizmo(g_picker);
    // Pid gizmo_id = picker_get_id(g_picker, mouse_position);
    // if (gizmo_id != 0xFFFFFFFF) {
    //   editor->gizmo_axis_hover = (Axis)gizmo_id;
    // }
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
      Vector3 distance = (ray - ray_start);

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

  // if (editor->gizmo_axis_hover != -1) {
  //   if (mouse_clicked(0)) {
  //     editor->gizmo_axis_active = editor->gizmo_axis_hover;
  //     editor->gizmo_mouse_start = mouse_position;
  //   }
  // } else if (editor->gizmo_axis_active == -1){
  //   if (mouse_clicked(0)) {
  //     picker_render(g_picker);
  //     Pid id = picker_get_id(g_picker, g_input.mouse_position);
  //     if (id != 0xFFFFFFFF) {
  //       Entity *e = lookup_entity(id);
  //       editor->hover_entity = e;
  //     }
  //   }
  // }

  //@Note Draw
  Entity *selected_entity = editor->active_selection;

  draw_scene();

  set_shader(shader_mesh);
  bind_uniform(shader_mesh, str8_lit("Constants"));

  set_sampler(str8_lit("diffuse_sampler"), sampler_linear);
  set_texture(str8_lit("diffuse_texture"), sun_icon_texture);
  set_blend_state(blend_state_alpha);

  for (Sun *sun : manager->by_type._Sun) {
    Matrix4 rotation_matrix = rotate_rh(sun->theta, editor->camera.up);
    Matrix4 world_matrix = translate(sun->position) * translate(sun->offset) * rotation_matrix;
    Matrix4 xform = editor->camera.transform * world_matrix;

    set_constant(str8_lit("xform"), xform);
    set_constant(str8_lit("color"), make_vec4(1.0f));
    apply_constants();
     
    Vector3 right = editor->camera.right;
    Vector3 up = cross_product(right, editor->camera.forward);
    f32 size = 0.5f;
    Vertex_XNCUU p0 = Vertex_XNCUU(-size * right - size * up, Vector3(), make_vec4(1.0f), Vector2(0, 1));
    Vertex_XNCUU p1 = Vertex_XNCUU( size * right - size * up, Vector3(), make_vec4(1.0f), Vector2(1, 1));
    Vertex_XNCUU p2 = Vertex_XNCUU( size * right + size * up, Vector3(), make_vec4(1.0f), Vector2(1, 0));
    Vertex_XNCUU p3 = Vertex_XNCUU(-size * right + size * up, Vector3(), make_vec4(1.0f), Vector2(0, 0));

    Vertex_XNCUU vertices[6];
    vertices[0] = p0;
    vertices[1] = p1;
    vertices[2] = p2;
    vertices[3] = p0;
    vertices[4] = p2;
    vertices[5] = p3;
 
    ID3D11Buffer *vertex_buffer = make_vertex_buffer(vertices, ArrayCount(vertices), sizeof(Vertex_XNCUU));
    UINT stride = sizeof(Vertex_XNCUU), offset = 0;
    d3d->device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
    d3d->device_context->Draw((UINT)ArrayCount(vertices), 0);
    vertex_buffer->Release();
  }

  set_blend_state(blend_state_default);

  // Draw selection
  set_shader(shader_mesh);
  bind_uniform(shader_mesh, str8_lit("Constants"));
  set_rasterizer(rasterizer_wireframe);
  set_depth_state(depth_state_default);
  set_texture(str8_lit("diffuse_texture"), d3d->fallback_tex);

  Vector4 strobe_color = lerp(Vector4(0.8f, 0.f, 0.8f, 1.f), Vector4(1.f, 0.5f, 1.f, 1.f), editor->select_strobe_t / editor->select_strobe_max);

  for (int i = 0; i < editor->selections.count; i++) {
    Entity *e = editor->selections[i];
    if (!e->mesh) continue;

    Matrix4 xform = editor->camera.transform * translate(e->position) * translate(e->offset) * rotate_rh(e->theta, Vector3(0, 1, 0));
    set_constant(str8_lit("xform"), xform);
    set_constant(str8_lit("color"), strobe_color);
    apply_constants();

    draw_wireframe_mesh(e->mesh);
  }
  set_rasterizer(rasterizer_default);

  //@Note Draw Gizmos
  if (editor->active_selection) {
    R_D3D11_State *d3d = r_d3d11_state();

    set_depth_state(depth_state_default);

    set_rasterizer(rasterizer_no_cull);

    set_shader(shader_mesh);
    bind_uniform(shader_mesh, str8_lit("Constants"));

    set_sampler(str8_lit("diffuse_sampler"), sampler_linear);

    f32 gizmo_scale_factor = Abs(length(editor->camera.origin - editor->active_selection->position) * 0.5f);
    gizmo_scale_factor = ClampBot(gizmo_scale_factor, 1.0f);

    Matrix4 world_matrix = translate(selected_entity->position) * scale(make_vec3(gizmo_scale_factor));
    Matrix4 xform = editor->camera.transform * world_matrix;
    set_constant(str8_lit("xform"), xform);
    set_texture(str8_lit("diffuse_texture"), d3d->fallback_tex);

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

      set_constant(str8_lit("color"), color);
      apply_constants();

      UINT stride = sizeof(Vertex_XNCUU), offset = 0;
      d3d->device_context->IASetVertexBuffers(0, 1, &mesh->vertex_buffer, &stride, &offset);
      d3d->device_context->Draw((UINT)mesh->vertices.count, 0);
    }
  }

  editor->select_strobe_t += (editor->select_strobe_target == 0.0f) ? -get_frame_delta(): get_frame_delta();
  if (editor->select_strobe_target == 0.0f && editor->select_strobe_t <= 0.0f) {
    editor->select_strobe_target = editor->select_strobe_max;
  }
  if (editor->select_strobe_target == editor->select_strobe_max && editor->select_strobe_t >= editor->select_strobe_max) {
    editor->select_strobe_target = 0.0f;
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

internal void entity_from_prototype(Entity *entity, Entity_Prototype *prototype) {
  entity->prototype_id = prototype->id;
  entity->kind  = prototype->entity.kind;
  entity->flags = prototype->entity.flags;
  entity->mesh = prototype->entity.mesh;
  entity->offset = prototype->entity.offset;
}

internal Entity *entity_from_prototype(Entity_Prototype *prototype) {
  Entity *entity = entity_make(prototype->entity.kind);
  entity_from_prototype(entity, prototype);
  return entity;
}

internal void editor_present_ui() {
  Editor *editor = g_editor;
  Editor_Panel *panel = editor->panel;
  Entity_Panel *entity_panel = editor->entity_panel;

  Entity_Manager *manager = get_entity_manager();

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
      reset_manager();
      String8 name = str8(panel->edit_load_world->buffer, panel->edit_load_world->buffer_len);
      load_world(name);
      editor->panel->expand_load_world = false;
    }
  }

  ui_set_next_pref_width(ui_txt(2.0f));
  ui_set_next_pref_height(ui_txt(4.0f));
  if (ui_clicked(ui_button(str8_lit("New")))) {
    reset_manager();
    World *world = new World();
    set_world(world);
  }

  ui_pop_parent();

  //@Note Entity Panel
  {
    Entity *e = editor->active_selection;
    Entity_Panel *panel = editor->entity_panel;

    f32 field_height = 20.0f;
    f32 panel_width = 280.0f;
    f32 panel_x = g_viewport->dimension.x - panel_width;
    f32 panel_y = 0;
    Vector4 panel_color = Vector4(0.24f, 0.86f, 0.59f, 1.0f);

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
        if (ui_released(ui_button(str8_lit("Instantiate")))) {
          Raycast raycast_result = {};
          raycast(editor->camera.origin, editor->camera.forward, 100.0f, &raycast_result);

          Entity_Prototype *prototype = editor->prototype_array[editor->prototype_idx];
          Entity *instance = entity_from_prototype(prototype);
          if (raycast_result.hit) {
            instance->set_position(raycast_result.hit_position + Vector3(0, 1, 0));
          } else {
            instance->set_position(raycast_result.hit_position);
          }

          single_select_entity(editor, instance);
        }
      }

      if (e && panel->active_tab == ENTITY_TAB_FIRST) {
        Auto_Array<Entity_Field*> &fields = panel->entity_fields[e->kind];
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
          Vector4 last_color = panel->override_color;
          UI_PrefWidth(ui_pct(1.0f))
            UI_PrefHeight(ui_pixels(field_height))
            UI_BackgroundColor(Vector4(1, 1, 1, 1))
            UI_TextColor(Vector4(0, 0, 0, 1))
          {
            Vector4 slider_color = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

            UI_Box *cont;
            cont = ui_box_create(UI_BOX_FLAG_DRAW_BACKGROUND, 0);
            ui_set_next_fixed_x(0);
            ui_set_next_fixed_y(0);
            ui_set_next_parent(cont);
            ui_slider(&panel->override_color.x, 0.0f, 1.0f, slider_color, str8_lit("R##color_r"));

            cont = ui_box_create(UI_BOX_FLAG_DRAW_BACKGROUND, 0);
            ui_set_next_fixed_x(0);
            ui_set_next_fixed_y(0);
            ui_set_next_parent(cont);
            ui_slider(&panel->override_color.y, 0.0f, 1.0f, slider_color, str8_lit("G##color_g"));

            cont = ui_box_create(UI_BOX_FLAG_DRAW_BACKGROUND, 0);
            ui_set_next_fixed_x(0);
            ui_set_next_fixed_y(0);
            ui_set_next_parent(cont);
            ui_slider(&panel->override_color.z, 0.0f, 1.0f, slider_color, str8_lit("B##color_b"));

            cont = ui_box_create(UI_BOX_FLAG_DRAW_BACKGROUND, 0);
            ui_set_next_fixed_x(0);
            ui_set_next_fixed_y(0);
            ui_set_next_parent(cont);
            ui_slider(&panel->override_color.w, 0.0f, 1.0f, slider_color, str8_lit("A##color_a"));
          }

          if (last_color != panel->override_color) {
            for (int i = 0; i < editor->selections.count; i++) {
              Entity *e = editor->selections[i];
              e->override_color = panel->override_color;
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

    if (e->kind == ENTITY_SUN && key_down(OS_KEY_CONTROL) && key_pressed(OS_KEY_L)) {
      Sun *sun = static_cast<Sun*>(e);
      sun->light_direction = editor->camera.forward;
    }
  }

  // Color Wheel
  {
    f32 widget_width = 200.0f;
    f32 widget_height = 200.0f;
    f32 x = g_viewport->dimension.x - widget_width;
    f32 y = 600.0f;
    ui_set_next_fixed_x(x);
    ui_set_next_fixed_y(y);
    ui_set_next_fixed_width(widget_width);
    ui_set_next_fixed_height(widget_height);
    ui_set_next_background_color(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    UI_Box *widget = ui_box_create(UI_BOX_FLAG_DRAW_BACKGROUND | UI_BOX_FLAG_CLICKABLE, str8_lit("##colorwheel"));
    UI_Signal sig = ui_signal_from_box(widget);
  }
}
