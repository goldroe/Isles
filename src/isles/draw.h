#ifndef DRAW_H
#define DRAW_H

#include <vector>

enum Draw_Bucket_Kind {
  DRAW_BUCKET_IMMEDIATE,
  DRAW_BUCKET_QUAD,
  DRAW_BUCKET_COUNT
};

struct Draw_Immediate_Batch {
  Draw_Immediate_Batch *prev;
  Draw_Immediate_Batch *next;

  Texture *texture;
  Matrix4 world;
  Matrix4 view;
  Matrix4 projection;
  Auto_Array<Vertex_3D> vertices;
};

struct Draw_Immediate_Batch_List {
  Draw_Immediate_Batch *first;
  Draw_Immediate_Batch *last;
  int count;
};

struct Draw_Immediate_Bucket {
  Vector3 light_dir;
  R_Depth_State_Kind depth_state;
  Draw_Immediate_Batch_List batches;
  b32 clear_depth_buffer;
  R_Blend_State_Kind blend_state;
};

struct Draw_Bucket {
  Draw_Bucket *next = nullptr;
  Draw_Bucket *prev = nullptr;

  int stack_gen = 0;

  Draw_Bucket_Kind kind;
  union {
    Draw_Immediate_Bucket immediate;
  };
};

struct Draw_Bucket_List {
  Draw_Bucket *first;
  Draw_Bucket *last;
  int count;
};

struct Draw_State {
  Arena *arena = nullptr;
  Draw_Bucket_List bucket_list;
  int next_stack_gen = 0;
};

#endif // DRAW_H
