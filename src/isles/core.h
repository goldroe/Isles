#ifndef CORE_H
#define CORE_H

typedef u32 Pid;

struct AABB {
  Vector3 min;
  Vector3 max;
};

enum Axis {
  AXIS_X,
  AXIS_Y,
  AXIS_Z,
  AXIS_COUNT
};

enum Direction {
  NORTH,
  EAST,
  SOUTH,
  WEST
};

struct Viewport {
  OS_Handle window_handle;
  Vector2 dimension;
};

struct Camera {
  Vector3 origin;
  Vector3 forward;
  Vector3 right;
  Vector3 up;
  f32 yaw;
  f32 pitch;
  // f32 roll;
  f32 fov;
  f32 near;
  f32 far;
  Matrix4 view_matrix;
  Matrix4 projection_matrix;
  Matrix4 transform;

  void update_euler_angles(f32 Yaw, f32 Pitch);
};

struct Byte_Buffer {
  u8 *buffer = nullptr;
  size_t len = 0;
  size_t capacity = 0;
  size_t ptr = 0;

  void advance();
  void put_byte(u8 b);
  void put_le16(u16 v);
  void put_le32(u32 v);
  void put_le64(u64 v);
  void put_f32(f32 v);
  void put_string(String8 string);

  u8  get_byte();
  u16 get_le16();
  u32 get_le32();
  u64 get_le64();
  f32 get_f32();
};

struct Particle_Pt {
  Vector3 position;
  Vector4 color;
  Vector2 scale;
};

struct Vertex_3D {
  Vector3 position;
  Vector4 color;
};

struct Vertex_XCUU {
  Vector3 position;
  Vector4 color;
  Vector2 uv;

  Vertex_XCUU() {}
  Vertex_XCUU(Vector3 p, Vector4 c, Vector2 uv) : position(p), color(c), uv(uv) {}
};

struct Vertex_XNCUU {
  Vector3 position;
  Vector3 normal;
  Vector4 color;
  Vector2 uv;

  Vertex_XNCUU() {}
  Vertex_XNCUU(Vector3 p, Vector3 n, Vector4 c, Vector2 uv) : position(p), normal(n), color(c), uv(uv) {}
};

struct Vertex_ARGB {
  Vector2 position;
  Vector2 uv;
  ARGB argb;
};

enum Token_Kind {
  TOKEN_EOF,

  TOKEN_COMMA,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_COLON,
  TOKEN_COLON2,

  TOKEN_NAME = 255,
  TOKEN_INT,
  TOKEN_FLOAT,
  TOKEN_STRING,
};

struct Token {
  Token_Kind kind;
  char *start;
  union {
    String8 name;
    String8 string;
    u64 int_val;
    f32 float_val;
  };
};

struct Lexer {
  char *stream;
  Token token;
};

enum Value_Kind {
  VALUE_NIL,
  VALUE_INT,
  VALUE_FLOAT,
  VALUE_STRING,
};

struct Value {
  Value_Kind kind;
  union {
    u64 int_val;
    f64 float_val;
    String8 string;
  };
};

template <typename T>
T range_clamp(T value, T min, T max) {
  if (value < min) {
    return max;
  } else if (value > max) {
    return min;
  } else {
    return value;
  }
}


ALLOCATOR_PROC(heap_allocator_proc);
internal Allocator heap_allocator();

#endif // CORE_H
