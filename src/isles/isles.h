#ifndef SOKOBAN_H
#define SOKOBAN_H

struct Entity;
struct Guy;
struct Texture;

struct Picker {
  b32 dirty;
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
  Auto_Array<Vector4> colors;

  Material material;

  b32 has_vertex_colors;
  b32 has_normals;
};

struct Model {
  Auto_Array<Mesh*> meshes;
};

struct World {
  String8 name;
  Auto_Array<Entity*> grid;
  Auto_Array<Entity*> entities;
  Guy *guy = nullptr;
};

struct Game_State {
  Vector2Int window_dim;
  f32 dt;
  Camera camera;

  b32 paused;
  b32 editing;

  Pid next_pid = 1;
  Auto_Array<Entity*> entities;
};

internal char *string_from_entity_kind(Entity_Kind kind);
internal char *string_from_entity_flag(Entity_Flags flags);

internal Entity *entity_make(Entity_Kind kind);
internal void update_camera_position(Camera *camera);
internal inline f32 get_frame_delta();
internal Entity *lookup_entity(Pid id);
internal inline Game_State *get_game_state();

internal inline World *get_world();
internal inline void set_world(World *world);

internal World *load_world(String8 file_name);
internal void serialize_world(World *world, String8 file_name);

internal void remove_grid_entity(World *world, Entity *entity);

#endif // SOKOBAN_H
