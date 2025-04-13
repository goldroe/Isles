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

struct Render_Target;

struct Picker {
  Vector2 dimension;
  Render_Target *render_target;
  void *staging_texture;
};

struct Raycast {
  Entity *hit;
  Vector3 hit_position;
  Face face;
};

struct World {
  String8 name;
  Auto_Array<Entity*> entities;
  Guy *guy = nullptr;
  Pid next_pid = 1;

  AABB bounding_box;
};

struct Game_State {
  Vector2Int window_dim;
  f32 dt;
  Camera camera;
  b32 paused;
  b32 editing;
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

internal void remove_grid_entity(World *world, Entity *entity);
internal Entity *find_entity_at(World *world, Vector3 position);


internal Sun *get_sun(World *world);


#endif // ISLES_H
