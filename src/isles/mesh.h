#if !defined(MESH_H)
#define MESH_H

struct Texture;

struct Material {
  Texture *texture;
};

struct Triangle_List_Info {
  int material_index;
  u32 vertices_count;
  u32 first_index;
};

struct Triangle_Mesh {
  String8 name;
  String8 file_path;

  Auto_Array<Vector3> vertices;
  Auto_Array<Vector2> uvs;
  Auto_Array<Vector3> normals;

  Auto_Array<Triangle_List_Info> triangle_list_info;
  Auto_Array<Material*> materials;

  // Vertex_XNCUU
  ID3D11Buffer *vertex_buffer = nullptr;
};


internal Triangle_Mesh *generate_plane_mesh(Vector2 size);

#endif // MESH_H
