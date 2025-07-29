#if !defined(ISLES_H)
#define ISLES_H

struct Entity;
struct Guy;

struct Audio_Settings {
  int master_volume;
  int sfx_volume;
  int music_volume;
};

struct Game_Settings {
  Audio_Settings audio_settings; 
};

struct Frame_Buffer;

struct Picker {
  Vector2 dimension;
  Frame_Buffer *frame_buffer;
  void *staging_texture;
};

struct Raycast {
  Entity *hit;
  Vector3 hit_position;
  Face face;
};

struct World {
  String8 name;
  // Auto_Array<Entity*> entities;
};

struct Game_State {
  Vector2Int window_dim;
  f32 dt;
  Camera camera;
  b32 paused = 0;
  b32 editing = 0;

  AABB bounding_box;
  b32 can_world_step = 0;
  f32 world_step_dt = 0;

  Reflection_Graph *reflection_graph;
};

internal inline Game_Settings *get_settings();

internal char *string_from_entity_kind(Entity_Kind kind);
internal char *string_from_entity_flag(Entity_Flags flags);

internal Entity *entity_make(Entity_Kind kind);
internal void update_camera_position(Camera *camera);
internal inline f32 get_frame_delta();
internal Entity *lookup_entity(Pid id);
internal inline Game_State *get_game_state();

internal inline World *get_world();
internal inline void set_world(World *world);


internal inline Arena *get_permanent_arena();

internal World *load_world(String8 file_name);

internal void save_world(World *world);
internal void save_world(World *world, String8 name);

internal Entity *find_entity_at(Vector3 position);


internal inline Entity_Manager *get_entity_manager();

internal void init_game();

#endif // ISLES_H
