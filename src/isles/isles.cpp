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

global Game_Settings *game_settings;

internal inline Arena *get_permanent_arena() {
  return permanent_arena;
}

internal inline Game_Settings *get_settings() {
  return game_settings;
}

internal inline Game_State *get_game_state() {
  return game_state;
}

internal inline World *get_world() {
  return g_world;
}

internal inline void set_world(World *world) {
  if (g_world) {
    g_world->entities.clear();
    delete g_world;
  }
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
  case ENTITY_MIRROR:
    return "Mirror";
  case ENTITY_SUN:
    return "Sun";
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
  for (int i = 0; i < world->entities.count; i++) {
    if (world->entities[i] == entity) {
      world->entities.remove(i);
      break;
    }
  }
}

internal Sun *get_sun(World *world) {
  Sun *sun = nullptr;
  for (int i = 0; i < world->entities.count; i++) {
    Entity *e = world->entities[i];
    if (e->kind == ENTITY_SUN) {
      sun = static_cast<Sun*>(e);
      break;
    }
  }
  return sun;
}

internal World *load_world(String8 name) {
  String8 file_name = path_join(get_permanent_arena(), str8_lit("data/worlds"), name);

  World *world = new World();
  world->name = str8_copy(permanent_arena, name);

  set_world(world);

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

  f32 x = buffer->get_f32();
  f32 y = buffer->get_f32();
  f32 z = buffer->get_f32();
  f32 yaw = buffer->get_f32();
  f32 pitch = buffer->get_f32();

  Camera *camera = &get_game_state()->camera;
  camera->origin = Vector3(x, y, z);
  camera->update_euler_angles(yaw, pitch);

  camera = &get_editor()->camera;
  camera->origin = Vector3(x, y, z);
  camera->update_euler_angles(yaw, pitch);

  Pid id = 1;

  u32 entity_count = buffer->get_le32();
  for (u32 i = 0; i < entity_count; i++) {
    Entity_Kind kind = (Entity_Kind)buffer->get_le16();
    u64 flags = buffer->get_le64();
    u64 prototype_id = buffer->get_le64();
    int x = buffer->get_le32();
    int y = buffer->get_le32();
    int z = buffer->get_le32();
    f32 theta = buffer->get_f32();

    Entity_Prototype *prototype = entity_prototype_lookup(prototype_id);
    Entity *entity = entity_from_prototype(prototype);
    entity->id = id;
    entity->flags = flags;
    entity->set_position(Vector3((f32)x, (f32)y, (f32)z));
    entity->set_theta(theta);

    entity->use_override_color = (b32)buffer->get_byte();
    entity->override_color.x = buffer->get_f32();
    entity->override_color.y = buffer->get_f32();
    entity->override_color.z = buffer->get_f32();
    entity->override_color.w = buffer->get_f32();

    switch (entity->kind) {
    case ENTITY_GUY:
    {
      world->guy = static_cast<Guy*>(entity);
      break;
    }

    case ENTITY_SUN:
    {
      Sun *sun = static_cast<Sun*>(entity);
      sun->light_direction.x = buffer->get_f32();
      sun->light_direction.y = buffer->get_f32();
      sun->light_direction.z = buffer->get_f32();
      break;
    }
    }

    world->entities.push(entity);
    id++;
  }

  world->next_pid = id + 1;

  if (!world->guy) {
    Entity_Prototype *prototype = entity_prototype_lookup("Guy");
    Entity *guy = entity_from_prototype(prototype);
    guy->set_position(Vector3(0, 2, 0));
    guy->set_theta(0);
    world->guy = static_cast<Guy*>(guy);
    world->entities.push(guy);
  }

  return world;
}

internal void save_world(World *world) {
  save_world(world, world->name);
}

internal void serialize_entity(Byte_Buffer *buffer, Entity *e) {
  buffer->put_le16(e->kind);
  buffer->put_le64(e->flags);
  buffer->put_le64(e->prototype_id);

  buffer->put_le32((u32)e->position.x);
  buffer->put_le32((u32)e->position.y);
  buffer->put_le32((u32)e->position.z);

  buffer->put_f32(e->theta);

  buffer->put_byte((u8)e->use_override_color);
  buffer->put_f32(e->override_color.x);
  buffer->put_f32(e->override_color.y);
  buffer->put_f32(e->override_color.z);
  buffer->put_f32(e->override_color.w);

  switch (e->kind) {
  case ENTITY_SUN:
  {
    Sun *sun = static_cast<Sun*>(e);
    buffer->put_f32(sun->light_direction.x);
    buffer->put_f32(sun->light_direction.y);
    buffer->put_f32(sun->light_direction.z);
    break;
  }
  }
}

internal void save_world(World *world, String8 file_name) {
  String8 file_path = path_join(get_permanent_arena(), str8_lit("data/worlds"), file_name);

  Byte_Buffer *buffer = new Byte_Buffer();

  // Header
  buffer->put_string(str8_lit("LVL"));

  // Camera
  Camera camera = get_editor()->camera;
  buffer->put_f32(camera.origin.x);
  buffer->put_f32(camera.origin.y);
  buffer->put_f32(camera.origin.z);
  buffer->put_f32(camera.yaw);
  buffer->put_f32(camera.pitch);

  // Entities
  buffer->put_le32((u32)world->entities.count);

  for (int i = 0; i < world->entities.count; i++) {
    Entity *e = world->entities[i];
    serialize_entity(buffer, e);
  }

  OS_Handle file = os_open_file(file_path, OS_AccessFlag_Write);
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

internal Entity *entity_make(Entity_Kind kind) {
  Entity *e = nullptr;
  switch (kind) {
  default:
    e = (Entity *)entity_alloc(sizeof(Entity));
    break;
  case ENTITY_GUY:
    e = (Entity *)entity_alloc(sizeof(Guy));
    break;
  case ENTITY_SUN:
    e = (Entity *)entity_alloc(sizeof(Sun));
    break;
  }
  e->kind = kind;
  e->override_color = Vector4(1, 1, 1, 1);
  e->id = get_world()->next_pid++;
  return e;
}

internal Entity *lookup_entity(Pid id) {
  if (id == 0) return nullptr;
  World *world = get_world();
  for (int i = 0; i < world->entities.count; i++) {
    Entity *e = world->entities[i];
    if (e->id == id) return e;
  }
  return nullptr;
}
    
internal Entity *find_entity_at(World *world, Vector3 position) {
  Vector3Int p0 = to_vec3i(position);
  for (int i = 0; i < world->entities.count; i++) {
    Entity *wall = world->entities[i];
    Vector3Int p1 = to_vec3i(wall->position);
    if (p0 == p1) return wall;
  }
  return nullptr;
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

internal void init_game() {
  UI_State *ui_state = new UI_State();
  ui_set_state(ui_state);
  ui_state->arena = arena_alloc(get_virtual_allocator(), MB(4));

  g_picker = make_picker(1280, 720);

  game_state = new Game_State();
  game_state->paused = false;
  game_state->editing = false;

  Arena *temp = make_arena(get_malloc_allocator());
  default_font = load_font(temp, str8_lit("data/fonts/seguisb.ttf"), 16);
  default_fonts[FONT_DEFAULT] = default_font;

  Font *icon_font;
  u32 icon_font_glyphs[] = { 87, 120, 33, 49, 85, 68, 76, 82, 57, 48, 55, 56, 123, 125, 67, 70, 35 };
  icon_font = load_icon_font(temp, str8_lit("data/fonts/icons.ttf"), 24, icon_font_glyphs, ArrayCount(icon_font_glyphs));
  default_fonts[FONT_ICON] = icon_font;

  Triangle_Mesh *guy_mesh      = load_mesh("data/meshes/Character/character.obj");
  Triangle_Mesh *block_mesh    = load_mesh("data/meshes/tile.obj");
  Triangle_Mesh *stone_mesh    = load_mesh("data/meshes/stone.obj");
  Triangle_Mesh *mirror_mesh   = load_mesh("data/meshes/mirror/mirror.obj");
  Triangle_Mesh *crate_mesh    = load_mesh("data/meshes/Crate.obj");
  Triangle_Mesh *sand_mesh     = load_mesh("data/meshes/sand.obj");

  init_editor();

  Entity_Prototype *guy_prototype = entity_prototype_create("Guy");
  guy_prototype->entity.kind = ENTITY_GUY;
  guy_prototype->entity.flags = ENTITY_FLAG_PUSHABLE;
  guy_prototype->entity.mesh = guy_mesh;
  guy_prototype->entity.override_color = Vector4(1, 1, 1, 1);
  guy_prototype->entity.offset = Vector3(0.5f, 0.0f, 0.5f);

  Entity_Prototype *mirror_prototype = entity_prototype_create("Mirror");
  mirror_prototype->entity.kind = ENTITY_MIRROR;
  mirror_prototype->entity.flags = ENTITY_FLAG_PUSHABLE;
  mirror_prototype->entity.mesh = mirror_mesh;
  mirror_prototype->entity.override_color = Vector4(1, 1, 1, 1);

  Entity_Prototype *block_prototype = entity_prototype_create("Block");
  block_prototype->entity.kind = ENTITY_BLOCK;
  block_prototype->entity.flags = ENTITY_FLAG_STATIC;
  block_prototype->entity.mesh = block_mesh;
  block_prototype->entity.override_color = Vector4(1, 1, 1, 1);

  Entity_Prototype *stone_prototype = entity_prototype_create("Stone");
  stone_prototype->entity.kind = ENTITY_BLOCK;
  stone_prototype->entity.flags = ENTITY_FLAG_PUSHABLE;
  stone_prototype->entity.mesh = stone_mesh;
  stone_prototype->entity.override_color = Vector4(1, 1, 1, 1);

  Entity_Prototype *crate_prototype = entity_prototype_create("Crate");
  crate_prototype->entity.kind = ENTITY_BLOCK;
  crate_prototype->entity.flags = ENTITY_FLAG_PUSHABLE;
  crate_prototype->entity.mesh = crate_mesh;
  crate_prototype->entity.override_color = Vector4(1, 1, 1, 1);

  Entity_Prototype *sand_prototype = entity_prototype_create("Sand");
  sand_prototype->entity.kind = ENTITY_BLOCK;
  sand_prototype->entity.flags = ENTITY_FLAG_STATIC;
  sand_prototype->entity.mesh = sand_mesh;
  sand_prototype->entity.override_color = Vector4(1, 1, 1, 1);

  Entity_Prototype *sun_prototype = entity_prototype_create("Sun");
  sun_prototype->entity.kind = ENTITY_SUN;
  sun_prototype->entity.flags = ENTITY_FLAG_STATIC;
  sun_prototype->entity.mesh = nullptr;
  sun_prototype->entity.override_color = Vector4(0.9f, 0.84f, 0.1f, 1.0f);

  game_state->camera.origin = Vector3(0, 0, 0);
  game_state->camera.up = Vector3(0, 1, 0);
  game_state->camera.forward = Vector3(0, 0, -1);
  game_state->camera.right = Vector3(1, 0, 0);
  game_state->camera.yaw = -PI * 0.5f;
  game_state->camera.pitch = 0;
  game_state->camera.fov = PI * 0.28f;

  game_state->camera.update_euler_angles(-PI * 0.5f, 0.0f);

  load_world(str8_lit("1.lvl"));

  g_viewport = new Viewport();
  g_viewport->dimension.x = 1;
  g_viewport->dimension.y = 1;

  undo_stack = new Undo_Stack();
  undo_stack->arena = make_arena(get_malloc_allocator());

  //@Note Game settings
  game_settings = new Game_Settings();
  game_settings->audio_settings.master_volume = 100;
  game_settings->audio_settings.sfx_volume = 70;
  game_settings->audio_settings.music_volume = 80;

  audio_engine = new Audio_Engine();
  audio_engine->init();

  // play_music("439015_somepin_cavernous.mp3");
}
