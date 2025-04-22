#ifndef ENTITY_H
#define ENTITY_H

enum Entity_Kind : u16 {
  ENTITY_NIL,
  ENTITY_GUY,
  ENTITY_INANIMATE,
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

struct Animation_State {
  f32 t = 0;
  Animation *animation = nullptr;
  Matrix4 bone_transforms[MAX_BONES];
};

struct Triangle_Mesh;

struct Entity {
  Entity_Kind kind;
  u64 flags;
  Pid id;
  u64 prototype_id;

  Vector3 position;
  Vector3 visual_position;

  Vector3 rotation;
  Vector3 visual_rotation;

  Vector3 offset;

  f32 theta;
  f32 theta_target;

  b32 use_override_color;
  Vector4 override_color;

  String8 mesh_name;
  Triangle_Mesh *mesh;

  // animation state
  Animation_State *animation_state;

  b32 to_be_destroyed;

  void update();

  inline void set_position(Vector3 p) {
    position = p;
    visual_position = p;
  }
  inline void set_position(Axis axis, f32 p) {
    position[axis] = p;
    visual_position[axis] = p;
  }

  inline void set_theta(f32 t) {
    theta = t;
    theta_target = t;
  }
};

struct Guy : Entity {
  Vector3 forward;
  Pid mirror_id;
};

struct Reflection_Node;

struct Mirror : Entity {
  Vector3 reflection_vectors[2];
  Reflection_Node *node;
};

struct Sun : Entity {
  Vector3 light_direction;
  Matrix4 light_view;
  Matrix4 light_projection;
  Matrix4 light_space_matrix;
};

#endif // ENTITY_H
