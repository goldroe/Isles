#if !defined(MESH_H)
#define MESH_H

#define MAX_BONES 100
#define MAX_BONE_WEIGHTS 4

struct Texture;
struct Triangle_Mesh;

struct Vertex_Skinned {
  Vector3 position;
  Vector3 normal;
  Vector2 uv;
  int bone_ids[MAX_BONE_WEIGHTS];
  f32 bone_weights[MAX_BONE_WEIGHTS];
};

struct Material {
  Texture *texture;
};

struct Bone_Info {
  int id;
  Matrix4 offset_matrix;
};

struct Key_Vector {
  f64 time;
  Vector3 value;
};

struct Key_Quat {
  f64 time;
  Quaternion value;
};

struct Triangle_List_Info {
  int material_index;
  u32 vertices_count;
  u32 first_index;
};

enum Mesh_Flags {
  MESH_FLAG_ANIMATED = (1<<0),
};

struct Bone {
  std::string name;
  int id;
  Matrix4 local_transform;

  Auto_Array<Key_Vector> translations;
  Auto_Array<Key_Quat> rotations;
  Auto_Array<Key_Vector> scalings; 
};

struct Animation_Node {
  std::string name;
  Matrix4 transform;
  Auto_Array<Animation_Node*> children;
};

struct Animation {
  std::string name;
  f64 duration;
  f64 ticks_per_second;

  Triangle_Mesh *mesh;
  Auto_Array<Bone*> bones;
};

struct Triangle_Mesh {
  Mesh_Flags flags;
  String8 name;
  String8 file_path;

  Auto_Array<Vector3> vertices;
  Auto_Array<Vector2> uvs;
  Auto_Array<Vector3> normals;

  Auto_Array<Triangle_List_Info> triangle_list_info;
  Auto_Array<Material*> materials;

  // Vertex_XNCUU
  ID3D11Buffer *vertex_buffer = nullptr;

  // Skinned data
  b32 animated = 0;
  int bone_counter = 0;
  std::unordered_map<std::string, Bone_Info> bone_info_map;
  std::unordered_map<std::string, Animation*> animation_map;
  Animation_Node *anim_root = nullptr;
  ID3D11Buffer *skinned_vertex_buffer = nullptr;
  ID3D11Buffer *skinned_index_buffer = nullptr;
  Auto_Array<u32> skinned_indices;
  Auto_Array<Vertex_Skinned> skinned_vertices;
};

internal void set_vertex_bone_data(Vertex_Skinned *vertex, int bone_id, f32 weight);
internal void read_animation(const aiScene *scene, Triangle_Mesh *mesh);
internal void read_animation_node(aiNode *node, Animation_Node *anim_node);

internal Triangle_Mesh *gen_plane_mesh(Vector2 size);
internal Triangle_Mesh *gen_cube_mesh();

#endif // MESH_H
