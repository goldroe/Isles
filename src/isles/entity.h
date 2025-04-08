#ifndef ENTITY_H
#define ENTITY_H

enum Entity_Kind : u16 {
  ENTITY_NIL,
  ENTITY_GUY,
  ENTITY_BLOCK,
  ENTITY_MIRROR,
  ENTITY_SUN,
  ENTITY_COUNT
};

enum Entity_Flags : u64 {
  ENTITY_FLAG_NIL = 0,
  ENTITY_FLAG_STATIC   = (1<<0),
  ENTITY_FLAG_PUSHABLE = (1<<1),
};
EnumDefineFlagOperators(Entity_Flags);

Entity_Flags entity_flag_array[] = { ENTITY_FLAG_PUSHABLE, ENTITY_FLAG_STATIC, };


struct Triangle_Mesh;

struct Entity {
  Entity_Kind kind;
  u64 flags;
  Pid id;
  u64 prototype_id;

  Vector3Int position;
  Vector3 visual_position;

  Vector3 rotation;
  Vector3 visual_rotation;

  f32 theta;
  f32 theta_target;

  Triangle_Mesh *mesh;
  
  b32 use_override_color;
  Vector4 override_color;

  b32 to_be_destroyed;

  void update();

  inline void set_position(Vector3Int p) {
    position = p;
    visual_position = Vector3((f32)p.x, (f32)p.y, (f32)p.z);
  }
  inline void set_position(Axis axis, int p) {
    position[axis] = p;
    visual_position[axis] = (f32)p;
  }

  inline void set_theta(f32 t) {
    theta = t;
    theta_target = t;
  }
};

struct Guy : Entity {
  Vector3 forward;
  Pid mirror_id;
  Vector3Int reflect_target;
  Vector3Int reflect_position;
};

struct Sun : Entity {
  Vector3 light_direction;
  Matrix4 light_view;
  Matrix4 light_projection;
  Matrix4 light_space_matrix;
};

#endif // ENTITY_H
