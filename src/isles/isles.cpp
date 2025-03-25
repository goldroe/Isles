//@Todos
// Jungle themed
// 10001 spikes inspired
// Bugs, Snakes, Etc
// Explorer types
// Rivers
// Golden skulls
// Spike/arrow traps
// sand blocks
// water
// falling
// gui
// death screen
// undo system

global Arena *permanent_arena;
global Arena *temporary_arena;

global Game_State *game_state;
global World *g_world;

global Font *default_font;

internal inline Game_State *get_game_state() {
  return game_state;
}

internal inline World *get_world() {
  return g_world;
}

internal inline void set_world(World *world) {
  g_world = world;
}

internal inline f32 get_frame_delta() {
  return game_state->dt;
}

internal char *string_from_entity_kind(Entity_Kind kind) {
  switch (kind) {
  default:
    return "";
  case ENTITY_GUY:
    return "Guy";
  case ENTITY_BLOCK:
    return "Block";
  case ENTITY_ARROW:
    return "Arrow";
  case ENTITY_MIRROR:
    return "Mirror";
  } 
}

internal char *string_from_entity_flag(Entity_Flags flags) {
  switch (flags) {
  default:
    return "";
  case ENTITY_FLAG_PUSHABLE:
    return "PUSHABLE";
  case ENTITY_FLAG_STATIC:
    return "STATIC";
  }
}

internal void remove_grid_entity(World *world, Entity *entity) {
  for (int i = 0; i < world->grid.count; i++) {
    if (world->grid[i] == entity) {
      world->grid.remove(i);
      break;
    }
  }
}

internal World *load_world(String8 file_name) {
  World *world = new World();
  world->name = str8_copy(permanent_arena, path_file_name(file_name));

  Byte_Buffer *buffer = new Byte_Buffer();
  
  OS_Handle file = os_open_file(file_name, OS_AccessFlag_Read);
  if (os_valid_handle(file)) {
    buffer->len = (size_t)os_read_entire_file(file, (void **)&buffer->buffer);
    os_close_handle(file);
  }

  bool valid = false;
  if (buffer->get_byte() == 'L' && buffer->get_byte() == 'V' && buffer->get_byte() == 'L') {
    valid = true;
  } 

  if (!valid) {
    fprintf(stderr, "ERROR LOADING '%s'\n", (char *)file_name.data);
    return nullptr;
  }

  Pid max_id = 0;
  u32 entity_count = buffer->get_le32();

  for (u32 i = 0; i < entity_count; i++) {
    u64 id = buffer->get_le64();
    Entity_Kind kind = (Entity_Kind)buffer->get_le16();
    u64 flags = buffer->get_le64();
    u64 prototype_id = buffer->get_le64();
    int x = buffer->get_le32();
    int y = buffer->get_le32();
    int z = buffer->get_le32();
    f32 theta = buffer->get_f32();

    Entity_Prototype *prototype = entity_prototype_lookup(prototype_id);
    Entity *entity = entity_from_prototype(prototype);
    entity->id = (Pid)id;
    entity->flags = flags;
    entity->set_position(Vector3Int(x, y, z));
    entity->set_theta(theta);

    if (entity->kind == ENTITY_GUY) {
      world->guy = static_cast<Guy*>(entity);
    }

    world->grid.push(entity);

    if (id > max_id) max_id = (Pid)id;
  }

  game_state->next_pid = max_id + 1;

  if (!world->guy) {
    Entity_Prototype *prototype = entity_prototype_lookup("Guy");
    Entity *guy = entity_from_prototype(prototype);
    guy->set_position(Vector3Int(0, 2, 0));
    guy->set_theta(0);
    world->guy = static_cast<Guy*>(guy);
    world->grid.push(guy);
  }

  return world;
}

internal void serialize_world(World *world, String8 file_name) {
  Byte_Buffer *buffer = new Byte_Buffer();
  buffer->put_string(str8_lit("LVL"));

  buffer->put_le32((u32)world->grid.count);

  for (int i = 0; i < world->grid.count; i++) {
    Entity *e = world->grid[i];
    buffer->put_le64(e->id);
    buffer->put_le16(e->kind);
    buffer->put_le64(e->flags);
    buffer->put_le64(e->prototype_id);

    buffer->put_le32(e->position.x);
    buffer->put_le32(e->position.y);
    buffer->put_le32(e->position.z);

    buffer->put_f32(e->theta);
  }

  OS_Handle file = os_open_file(str8_lit("0.lvl"), OS_AccessFlag_Write);
  os_write_file(file, buffer->buffer, buffer->len);
  os_close_handle(file);
}

internal void *entity_alloc(int bytes) {
  void *memory = calloc(1, bytes);
  return memory;
}

internal void entity_free(Entity *entity) {
  free(entity);
}

internal void entity_push(Entity *e) {
  e->id = game_state->next_pid++;
  game_state->entities.push(e);
}

internal Entity *entity_make(Entity_Kind kind) {
  Entity *e = nullptr;
  switch (kind) {
  default:
    e = (Entity *)entity_alloc(sizeof(Entity));
    break;
  case ENTITY_GUY:
    e = (Guy *)entity_alloc(sizeof(Guy));
    break;
  }
  e->kind = kind;
  entity_push(e);
  return e;
}

internal Entity *lookup_entity(Pid id) {
  if (id == 0) return nullptr;
  for (int i = 0; i < game_state->entities.count; i++) {
    Entity *e = game_state->entities[i];
    if (e->id == id) return e;
  }
  return nullptr;
}
    
internal Entity *find_entity_at(World *world, Vector3Int position) {
  for (int i = 0; i < world->grid.count; i++) {
    Entity *wall = world->grid[i];
    if (wall->position == position) return wall;
  }
  return nullptr;
}

internal void update_guy_mirror(Guy *guy) {
  World *world = get_world();
  guy->mirror_id = 0;

  Auto_Array<Entity*> mirrors;
  for (int i = 0; i < world->grid.count; i++) {
    if (world->grid[i]->kind == ENTITY_MIRROR) {
      mirrors.push(world->grid[i]);
    }
  }

  for (int i = 0; i < mirrors.count; i++) {
    Entity *mirror = mirrors[i];

    Vector3 forward = to_vector3(rotate_rh(-mirror->theta, Vector3(0, 1, 0)) * Vector4(1, 0, 0, 1));
    Vector3Int dist = guy->position - mirror->position;
    if (dist.y != 0) break;
    int dx = forward.x > 0 ? 1 : -1;
    int dz = forward.z > 0 ? 1 : -1;

    Vector3Int position = mirror->position;
    if (mirror->position.z == guy->position.z && (dist.x > 0 == dx > 0)) {
      // Move X
      for (;;) {
        position.x += dx;
        Entity *e = find_entity_at(world, position);
        if (e) {
          if (e == guy) {
            guy->mirror_id = mirror->id;
          }
          break;
        }
      }
    } else if (mirror->position.x == guy->position.x && (dist.z > 0 == dz > 0)) {
      // Move Z
      for (;;) {
        position.z += dz;
        Entity *e = find_entity_at(world, position);
        if (e) {
          if (e == guy) {
            guy->mirror_id = mirror->id;
          }
          break;
        }
      }
    }
  }

  mirrors.clear();

  if (key_pressed(OS_KEY_SPACE) && guy->mirror_id) {
    Entity *mirror = lookup_entity(guy->mirror_id);
    if (mirror) {
      Vector3 mirror_forward = to_vector3(rotate_rh(-mirror->theta,  Vector3(0, 1, 0)) * Vector4(1, 0, 0, 1));
      Vector3 direction = to_vector3(guy->position - mirror->position);
      int distance = (int)Abs(magnitude(direction));

      if (direction.x) {
        Vector3Int position = guy->position;
        position.x = mirror->position.x;
        position.z += distance*(int)roundf(mirror_forward.z);
        guy->set_position(position);
      } else if (direction.z) {
        Vector3Int position = guy->position;
        position.z = mirror->position.z;
        position.x += distance*(int)roundf(mirror_forward.x);
        guy->set_position(position);
      }
    }
  }
}

internal bool move_entity(Entity *e, Vector3Int distance) {
  Vector3Int new_position = e->position + distance;
  Entity *wall = find_entity_at(get_world(), new_position);
  Entity *below = find_entity_at(get_world(), new_position - Vector3Int::Up());

  bool valid_move = true;

  if (wall) {
    if (wall->flags & ENTITY_FLAG_PUSHABLE) {
      valid_move = move_entity(wall, distance);
    } else {
      valid_move = false;
    }
  } else {
    if (e->kind == ENTITY_GUY && !below) {
      valid_move = false;
    }
  }

  if (valid_move) {
    e->position = new_position;
    if (!below) {
      e->position.y -= 1;
    }
  }

  return valid_move;
}

void update_guy(Guy *guy) {
  update_guy_mirror(guy);

  //@Note Move
  Camera *camera = &game_state->camera;

  int forward_dt = 0;
  int right_dt = 0;
  if (key_pressed(OS_KEY_UP)) {
    forward_dt += -1;
  }
  if (key_pressed(OS_KEY_DOWN)) {
    forward_dt += 1;
  }
  if (key_pressed(OS_KEY_LEFT)) {
    right_dt += -1;
  }
  if (key_pressed(OS_KEY_RIGHT)) {
    right_dt += 1;
  }

  bool moving = forward_dt || right_dt;
  bool moved = false;

  if (moving) {
    Vector3 move_direction = {0, 0, 0};
    if (forward_dt) {
      move_direction = {0, 0, (f32)forward_dt};
    } else if (right_dt) {
      move_direction = {(f32)right_dt, 0, 0};
    }

    f32 theta = atan2f(-move_direction.z, move_direction.x);
    if (theta < 0) theta += 2 * PI;
    guy->theta_target = theta;
    moved = move_entity(guy, truncate(move_direction));
  }

  if (moved) {
    update_guy_mirror(guy);
  }
}

void Entity::update() {
  switch (kind) {
  case ENTITY_GUY:
  {
    Guy *guy = static_cast<Guy*>(this);
    update_guy(guy);
    break;
  }
  }

  if (!(flags & ENTITY_FLAG_STATIC)) {
    f32 move_speed = 8.0f;
    if (Abs(visual_position.x - position.x) > 0.001f) {
      visual_position.x += (position.x - visual_position.x) * move_speed * game_state->dt;
    } else {
      visual_position.x = (f32)position.x;
    }
    if (Abs(visual_position.y - position.y) > 0.001f) {
      visual_position.y += (position.y - visual_position.y) * move_speed * game_state->dt;
    } else {
      visual_position.y = (f32)position.y;
    }
    if (Abs(visual_position.z - position.z) > 0.001f) {
      visual_position.z += (position.z - visual_position.z) * move_speed * game_state->dt;
    } else {
      visual_position.z = (f32)position.z;
    }

    f32 rot_speed = 8.0f;
    f32 theta_dt = theta_target - theta;
    if (Abs(theta_dt) > 0.001f) {
      theta += theta_dt * rot_speed * game_state->dt;
    } else {
      theta = (f32)theta_target;
    }
  }
}

internal void draw_world(World *world, Camera camera) {
  //@Note Draw world
  Vector3 light_dir = {0, 0, 0};
  light_dir.x = 10.0f;
  light_dir.y = 4.0f;

  draw_immediate_begin();

  Draw_Bucket *bucket = draw_top();
  bucket->immediate.depth_state = R_DEPTH_STENCIL_STATE_DEFAULT;
  bucket->immediate.light_dir = normalize(light_dir);

  for (int i = 0; i < world->grid.count; i++) {
    Entity *e = world->grid[i];
    Model *model = e->model;

    Matrix4 rotation_matrix = rotate_rh(e->theta, camera.up);
    Matrix4 world_matrix = translate(e->visual_position) * rotation_matrix * translate(-e->visual_position);
    Matrix4 view = camera.view_matrix;

    for (int mesh_idx = 0; mesh_idx < model->meshes.count; mesh_idx++) {
      Mesh *mesh = model->meshes[mesh_idx];

      Draw_Immediate_Batch *batch = push_immediate_batch(bucket);
      batch->world = world_matrix;
      batch->view = camera.view_matrix;
      batch->projection = camera.projection_matrix;
      batch->texture = mesh->material.texture;
            
      for (int vert_idx = 0; vert_idx < mesh->positions.count; vert_idx++) {
        Vector3 p = e->visual_position + mesh->positions[vert_idx];
        Vector4 color = mesh->has_vertex_colors ? mesh->colors[vert_idx] : mesh->material.diffuse_color;
        Vector2 uv = mesh->tex_coords[vert_idx];
        Vector3 normal = mesh->normals[vert_idx];
        draw_immediate_vertex(batch, p, normal, color, uv);
      }
    }
  }


  //@Note Draw mirror beams
  Entity *mirror = lookup_entity(world->guy->mirror_id);
  if (mirror) {
    draw_immediate_begin();

    Draw_Bucket *bucket = draw_top();
    bucket->immediate.depth_state = R_DEPTH_STENCIL_STATE_DEFAULT;
    bucket->immediate.blend_state = R_BLEND_STATE_ALPHA;

    Vector3 mirror_forward = to_vector3(rotate_rh(-mirror->theta,  Vector3(0, 1, 0)) * Vector4(1, 0, 0, 1));
    Vector3 direction = to_vector3(world->guy->position - mirror->position);
    int distance = (int)Abs(magnitude(direction));

    Vector3 beam_size = Vector3(1, 1, 1);
    Vector4 beam_color = Vector4(1, 1, 1, 0.5f);
    Vector3 beam_start = to_vector3(mirror->position);
    f32 dx = mirror_forward.x > 0 ? 1.0f : -1.0f;
    f32 dz = mirror_forward.z > 0 ? 1.0f : -1.0f;
    f32 beam_start_x = (f32)mirror->position.x + dx;
    f32 beam_start_z = (f32)mirror->position.z + dz;
    
    beam_start = to_vector3(mirror->position);
    beam_start.x = beam_start_x;

    Draw_Immediate_Batch *batch = push_immediate_batch(bucket);
    batch->world = translate(beam_start);
    batch->view = camera.view_matrix;
    batch->projection = camera.projection_matrix;
    batch->texture = nullptr;

    beam_size = Vector3(dx * (f32)distance, 1.0f, 1.0f);
    draw_rectangle(batch, Vector3(dx * -0.5f, 0.001f, -0.5f), beam_size, beam_color);

    beam_start = to_vector3(mirror->position);
    beam_start.z = beam_start_z;

    batch = push_immediate_batch(bucket);
    batch->world = translate(beam_start);
    batch->view = camera.view_matrix;
    batch->projection = camera.projection_matrix;
    batch->texture = nullptr;

    beam_size = Vector3(1.0f, 1.0f, dz * (f32)distance);
    draw_rectangle(batch, Vector3(dz * -0.5f, 0.001f, -0.5f), beam_size, beam_color);
  }
}

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
    if (tMaxX > 1 && tMaxY > 1 && tMaxZ > 1) break;

    // process block here
        
    Entity *entity = find_entity_at(get_world(), block);
    if (entity) {
      raycast->hit = entity;
      raycast->hit_position.x = (f32)block.x;
      raycast->hit_position.y = (f32)block.y;
      raycast->hit_position.z = (f32)block.z;
      return true;
    }
  }

  return false;
}

internal void update_camera_position(Camera *camera) {
  f32 forward_dt =  0.0f;
  f32 right_dt = 0.0f;
  f32 up_dt = 0.0f;
  if (key_down(OS_KEY_W)) forward_dt += 1;
  if (key_down(OS_KEY_S)) forward_dt -= 1;
  if (key_down(OS_KEY_D)) right_dt += 1;
  if (key_down(OS_KEY_A)) right_dt -= 1;
  if (key_down(OS_KEY_Q)) up_dt += 1;
  if (key_down(OS_KEY_E)) up_dt -= 1;

  update_camera_position(camera, forward_dt, right_dt, up_dt);

  Vector2 middle_delta = get_mouse_middle_drag_delta();
  if (middle_delta.x || middle_delta.y) {
    f32 speed = 0.1f;
    Vector3 new_origin = camera->origin;
    new_origin += camera->right * middle_delta.x * speed * get_frame_delta();
    new_origin -= camera->forward * middle_delta.y * speed * get_frame_delta();
    camera->origin = new_origin;
    update_camera_matrix(camera);
  }
}

internal void update_world_state() {
  Vector2 mouse_delta = get_mouse_delta();
  update_camera_orientation(&game_state->camera, mouse_delta);
  update_camera_position(&game_state->camera);

  //@Note Update entities
  for (int i = 0; i < game_state->entities.count; i++) {
    Entity *e = game_state->entities[i];
    e->update();
  }
}

internal UI_Line_Edit *ui_line_edit_create(String8 name) {
  UI_Line_Edit *result = new UI_Line_Edit();
  result->name = name;
  result->box = nullptr;
  result->sig = {};
  result->buffer = (u8 *)calloc(1, 256);
  result->buffer_len = 0;
  result->buffer_pos = 0;
  result->buffer_capacity = 256;
  return result;
}

internal void update_and_render(OS_Event_List *events, OS_Handle window_handle, f32 dt) {
  local_persist bool first_call = true;
  if (first_call) {
    first_call = false;

    UI_State *ui_state = new UI_State();
    ui_set_state(ui_state);
    ui_state->arena = arena_alloc(get_virtual_allocator(), MB(4));

    g_picker = new Picker();
    g_picker->dimension = {0, 0};
    g_picker->texture = nullptr;
    g_picker->select_texture = nullptr;
    g_picker->render_target_view = nullptr;

    game_state = new Game_State();

    game_state->next_pid = 1;

    game_state->paused = false;
    game_state->editing = false;

    Arena *temp = make_arena(get_malloc_allocator());
    default_font = load_font(temp, str8_lit("data/fonts/seguisb.ttf"), 14);

    Model *guy_model = load_model("data/models/Character/character.obj");
    Model *arrow_model = load_model("data/models/arrow.obj");
    Model *block_model = load_model("data/models/block.obj");
    Model *stone_model = load_model("data/models/stone.obj");
    Model *mirror_model = load_model("data/models/mirror/mirror.obj");

    editor = new Editor();
    {
      Entity_Panel *panel = new Entity_Panel();
      editor->entity_panel = panel;
      panel->position_x = ui_line_edit_create(str8_lit("X"));
      panel->position_y = ui_line_edit_create(str8_lit("Y"));
      panel->position_z = ui_line_edit_create(str8_lit("Z"));
      panel->theta      = ui_line_edit_create(str8_lit("Theta"));

      editor->gizmo_models[GIZMO_TRANSLATE][AXIS_X] = load_model("data/models/gizmo/translate_x.obj");
      editor->gizmo_models[GIZMO_TRANSLATE][AXIS_Y] = load_model("data/models/gizmo/translate_y.obj");
      editor->gizmo_models[GIZMO_TRANSLATE][AXIS_Z] = load_model("data/models/gizmo/translate_z.obj");
      editor->gizmo_models[GIZMO_ROTATE][AXIS_X] = load_model("data/models/gizmo/rotation_x.obj");
      editor->gizmo_models[GIZMO_ROTATE][AXIS_Y] = load_model("data/models/gizmo/rotation_y.obj");
      editor->gizmo_models[GIZMO_ROTATE][AXIS_Z] = load_model("data/models/gizmo/rotation_z.obj");
    }

    Entity_Prototype *guy_prototype = entity_prototype_create("Guy");
    guy_prototype->entity.kind = ENTITY_GUY;
    guy_prototype->entity.flags = ENTITY_FLAG_PUSHABLE;
    guy_prototype->entity.model = guy_model;

    Entity_Prototype *mirror_prototype = entity_prototype_create("Mirror");
    mirror_prototype->entity.kind = ENTITY_MIRROR;
    mirror_prototype->entity.flags = ENTITY_FLAG_PUSHABLE;
    mirror_prototype->entity.model = mirror_model;

    Entity_Prototype *block_prototype = entity_prototype_create("Block");
    block_prototype->entity.kind = ENTITY_BLOCK;
    block_prototype->entity.flags = ENTITY_FLAG_STATIC;
    block_prototype->entity.model = block_model;

    Entity_Prototype *stone_prototype = entity_prototype_create("Stone");
    stone_prototype->entity.kind = ENTITY_BLOCK;
    stone_prototype->entity.flags = ENTITY_FLAG_PUSHABLE;
    stone_prototype->entity.model = stone_model;

    Entity_Prototype *arrow_prototype = entity_prototype_create("Arrow");
    arrow_prototype->entity.kind = ENTITY_ARROW;
    arrow_prototype->entity.flags = ENTITY_FLAG_STATIC;
    arrow_prototype->entity.model = arrow_model;

    World *world = load_world(str8_lit("0.lvl"));
    set_world(world);

    Draw_State *draw_state = new Draw_State();
    draw_set_state(draw_state);

    game_state->camera.origin = Vector3(0, 0, 0);
    game_state->camera.up = Vector3(0, 1, 0);
    game_state->camera.forward = Vector3(0, 0, -1);
    game_state->camera.right = Vector3(1, 0, 0);
    game_state->camera.yaw = -PI * 0.5f;
    game_state->camera.pitch = 0;
    game_state->camera.fov = PI * 0.4f;

    game_state->camera.update_euler_angles(-PI * 0.5f, 0.0f);

    g_viewport = new Viewport();
    g_viewport->dimension.x = 1;
    g_viewport->dimension.y = 1;
    g_viewport->window_handle = window_handle;
  }

  ui_g_state->animation_dt = dt;

  r_d3d11_update_dirty_shaders();
    
  Vector2 window_dim = os_get_window_dim(window_handle);

  game_state->dt = dt / 1000.0f;
  game_state->window_dim = truncate(window_dim);

  g_viewport->window_handle = window_handle;
  if (g_viewport->dimension != window_dim) {
    g_viewport->dimension = window_dim;
  }

  if (g_picker->dimension != game_state->window_dim) {
    g_picker->dimension = game_state->window_dim;
    r_picker_init(g_picker);
  }

  os_set_cursor(OS_Cursor_Arrow);

  f32 aspect = (f32)game_state->window_dim.x / (f32)game_state->window_dim.y;
  game_state->camera.projection_matrix = perspective_rh(game_state->camera.fov, aspect, 0.1f, 1000.0f);
  editor->camera.projection_matrix     = perspective_rh(editor->camera.fov, aspect, 0.1f, 1000.0f);

  input_begin(window_handle, events);

  draw_begin(window_handle);

  r_clear_color(0.51f, 0.75f, 0.87f, 1.0f);
  // r_clear_color(0.0f, 0.0f, 0.0f, 1.0f);

  r_resize_render_target(game_state->window_dim);

  for (int i = 0; i < game_state->entities.count; i++) {
    Entity *e = game_state->entities[i];
    if (e->to_be_destroyed) {
      game_state->entities.remove(i);
      entity_free(e);
    }
  }

  if (game_state->paused) {
    input_set_mouse_capture(false);
    draw_world(get_world(), game_state->camera);
  }

  if (game_state->editing) {
    input_set_mouse_capture(false);
    update_editor();
  }

  if (!game_state->paused && !game_state->editing) {
    input_set_mouse_capture(true);
    update_world_state();
    draw_world(get_world(), game_state->camera);
  }

  if (key_pressed(OS_KEY_F1) ||
      key_pressed(OS_KEY_1)) {
    if (!game_state->editing) {
      game_state->editing = true; 
      editor->camera = game_state->camera;
      editor->update_entity_panel();
    } else {
      game_state->editing = false; 
    }
  }

  //@Note UI Layout
  ui_begin(window_handle, events);

  ui_push_font(default_font);
  ui_push_background_color(Vector4(0.55f, 0.22f, 0.63f, 1.0f));
  ui_push_text_color(Vector4(1.0f, 1.0f, 1.0f, 1.0f));

  if (game_state->editing) {
    editor_present_ui(); 
  }

  r_d3d11_render(window_handle, &g_draw_state->bucket_list);

  ui_layout_apply();
  draw_ui_layout();

  r_d3d11_state()->swap_chain->Present(1, 0);

  draw_end();

  input_end(window_handle);

  ui_end();
}
