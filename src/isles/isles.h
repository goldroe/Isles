#ifndef SOKOBAN_H
#define SOKOBAN_H

struct Entity;
struct Guy;
struct Texture;

struct Audio_Settings {
  int master_volume;
  int sfx_volume;
  int music_volume;
};

struct Game_Settings {
  Audio_Settings audio_settings; 
};

struct Picker {
  Vector2Int dimension;

  void *texture;
  void *select_texture;
  void *render_target_view;
};

struct Raycast {
  Entity *hit;
  Vector3 hit_position;
  Face face;
};

struct Material {
  Texture *texture;
  Vector4 diffuse_color;
};

struct Mesh {
  Auto_Array<Vector3> positions;
  Auto_Array<Vector2> tex_coords;
  Auto_Array<Vector3> normals;

  Material material;

  b32 has_normals;
};

struct Model {
  Auto_Array<Mesh*> meshes;
};

struct World {
  String8 name;
  Auto_Array<Entity*> entities;
  Guy *guy = nullptr;
  Pid next_pid = 1;
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

#endif // SOKOBAN_H
