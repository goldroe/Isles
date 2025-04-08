global Assimp::Importer assimp_importer;

internal inline Vector2 to_vec2(aiVector3D v) {
  Vector2 result = Vector2(v.x, v.y);
  return result; 
}

internal inline Vector3 to_vec3(aiVector3D v) {
  Vector3 result = Vector3(v.x, v.y, v.z);
  return result; 
}

internal inline Vector4 to_vec4(aiColor4D v) {
  Vector4 result = Vector4(v.r, v.g, v.b, v.a);
  return result;
}

internal Triangle_Mesh *load_mesh(std::string file_name) {
  u32 import_flags = aiProcess_Triangulate;
  import_flags |= aiProcess_FlipUVs;
  import_flags |= aiProcess_GenNormals;

  const aiScene *scene = assimp_importer.ReadFile(file_name.c_str(), import_flags);
  if (!scene) {
    logprint("Failed to load mesh '%s'.\n", file_name.c_str());
    logprint("Error: %s\n", assimp_importer.GetErrorString());
    return nullptr;
  }

  Triangle_Mesh *mesh = new Triangle_Mesh();

  u32 total_vertices = 0;

  for (int mesh_idx = 0; mesh_idx < (int)scene->mNumMeshes; mesh_idx++) {
    aiMesh *amesh = scene->mMeshes[mesh_idx];

    Triangle_List_Info triangle_list_info;
    triangle_list_info.material_index = amesh->mMaterialIndex;
    triangle_list_info.first_index = total_vertices;

    for (u32 i = 0; i < amesh->mNumFaces; i++) {
      aiFace *face = amesh->mFaces + i;
      Assert(face->mNumIndices == 3);

      u32 i0 = face->mIndices[0];
      u32 i1 = face->mIndices[1];
      u32 i2 = face->mIndices[2];

      mesh->vertices.push(to_vec3(amesh->mVertices[i0]));
      mesh->vertices.push(to_vec3(amesh->mVertices[i1]));
      mesh->vertices.push(to_vec3(amesh->mVertices[i2]));

      if (amesh->HasTextureCoords(0)) {
        mesh->uvs.push(to_vec2(amesh->mTextureCoords[0][i0]));
        mesh->uvs.push(to_vec2(amesh->mTextureCoords[0][i1]));
        mesh->uvs.push(to_vec2(amesh->mTextureCoords[0][i2]));
      } else {
        mesh->uvs.push(Vector2(0, 0));
        mesh->uvs.push(Vector2(0, 0));
        mesh->uvs.push(Vector2(0, 0));
      }

      mesh->normals.push(to_vec3(amesh->mNormals[i0])); 
      mesh->normals.push(to_vec3(amesh->mNormals[i1])); 
      mesh->normals.push(to_vec3(amesh->mNormals[i2])); 
    }

    u32 vertices_count = amesh->mNumFaces * 3;
    triangle_list_info.vertices_count = vertices_count;
    total_vertices += vertices_count;
    mesh->triangle_list_info.push(triangle_list_info);
  }


  std::string mesh_path = get_parent_path(file_name);

  mesh->materials.reserve(scene->mNumMaterials);
  for (u32 i = 0; i < scene->mNumMaterials; i++) {
    Material *material = new Material();
    material->texture = nullptr;

    aiMaterial *amaterial = scene->mMaterials[i];
    if (amaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
      aiString name;
      if (amaterial->GetTexture(aiTextureType_DIFFUSE, 0, &name) == AI_SUCCESS) {
        std::string material_path = mesh_path;
        String8 material_file_name = str8((u8 *)name.data, name.length);
        if (path_is_relative(material_file_name)) {
          material_path += "/" + std::string((char *)material_file_name.data);
          material_file_name = str8((u8 *)material_path.data(), material_path.size());
        }

        Texture *texture = r_create_texture_from_file(material_file_name, TEXTURE_FLAG_GENERATE_MIPS);
        material->texture = texture;
      } else {
        logprint("Failed to load diffuse texture '%s'.\n", name.data);
      }
    }

    mesh->materials.push(material);
  }

  return mesh;
}

internal Triangle_Mesh *generate_plane_mesh(Vector2 size) {
  Vector2 half_size = 0.5f * size;

  Triangle_Mesh *mesh = new Triangle_Mesh();

  mesh->vertices.push(Vector3(-half_size.x,  0.0f, -half_size.y));
  mesh->vertices.push(Vector3(half_size.x,   0.0f, -half_size.y));
  mesh->vertices.push(Vector3(half_size.x,   0.0f, half_size.y));
  mesh->vertices.push(Vector3(-half_size.x,  0.0f, -half_size.y));
  mesh->vertices.push(Vector3(half_size.x,   0.0f, half_size.y));
  mesh->vertices.push(Vector3(-half_size.x,  0.0f, half_size.y));

  mesh->normals.push(Vector3(0.0f, 1.0f, 0.0f));
  mesh->normals.push(Vector3(0.0f, 1.0f, 0.0f));
  mesh->normals.push(Vector3(0.0f, 1.0f, 0.0f));
  mesh->normals.push(Vector3(0.0f, 1.0f, 0.0f));
  mesh->normals.push(Vector3(0.0f, 1.0f, 0.0f));
  mesh->normals.push(Vector3(0.0f, 1.0f, 0.0f));

  mesh->uvs.push((Vector2(0.0f, 0.0f)));
  mesh->uvs.push((Vector2(1.0f, 0.0f)));
  mesh->uvs.push((Vector2(1.0f, 1.0f)));
  mesh->uvs.push((Vector2(0.0f, 0.0f)));
  mesh->uvs.push((Vector2(1.0f, 1.0f)));
  mesh->uvs.push((Vector2(0.0f, 1.0f)));

  return mesh;
}
