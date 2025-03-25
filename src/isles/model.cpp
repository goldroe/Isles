global Assimp::Importer assimp_importer;

internal inline Vector3 get_ai_vertex(aiVector3D v) {
  Vector3 result = {v.x, v.y, v.z};
  return result; 
}

internal inline Vector2 get_ai_texcoord(aiVector3D v) {
  Vector2 result = {v.x, v.y};
  return result; 
}

internal inline Vector4 get_ai_color(aiColor4D v) {
  Vector4 result = { v.r, v.g, v.b, v.a};
  return result;
}

internal Model *load_model(std::string file_name) {
  u32 import_flags = aiProcess_Triangulate;
  import_flags |= aiProcess_FlipUVs;

  const aiScene *scene = assimp_importer.ReadFile(file_name.data(), import_flags);
  if (!scene) {
    fprintf(stderr, "Failed to load mesh %s: %s\n", file_name.data(), assimp_importer.GetErrorString());
  }

  std::string model_path = get_parent_path(file_name);

  Model *model = new Model();

  for (int mesh_idx = 0; mesh_idx < (int)scene->mNumMeshes; mesh_idx++) {
    Mesh *mesh = new Mesh();
    model->meshes.push(mesh);

    aiMesh *ai_mesh = scene->mMeshes[mesh_idx];
    mesh->positions.reserve(ai_mesh->mNumVertices);
    mesh->tex_coords.reserve(ai_mesh->mNumVertices);

    for (u32 i = 0; i < ai_mesh->mNumFaces; i++) {
      aiFace *face = ai_mesh->mFaces + i;
      Assert(face->mNumIndices <= 3);

      u32 i0 = face->mIndices[0];
      u32 i1 = face->mIndices[1];
      u32 i2 = face->mIndices[2];
        
      mesh->positions.push(get_ai_vertex(ai_mesh->mVertices[i0]));
      mesh->positions.push(get_ai_vertex(ai_mesh->mVertices[i1]));
      mesh->positions.push(get_ai_vertex(ai_mesh->mVertices[i2]));
      if (ai_mesh->HasTextureCoords(0)) {
        mesh->tex_coords.push(get_ai_texcoord(ai_mesh->mTextureCoords[0][i0]));
        mesh->tex_coords.push(get_ai_texcoord(ai_mesh->mTextureCoords[0][i1]));
        mesh->tex_coords.push(get_ai_texcoord(ai_mesh->mTextureCoords[0][i2]));
      }
      if (ai_mesh->HasVertexColors(0)) {
        mesh->colors.push(get_ai_color(ai_mesh->mColors[0][i0]));
        mesh->colors.push(get_ai_color(ai_mesh->mColors[0][i1]));
        mesh->colors.push(get_ai_color(ai_mesh->mColors[0][i2]));
      }
      if (ai_mesh->HasNormals()) {
        mesh->normals.push(get_ai_vertex(ai_mesh->mNormals[i0]));
        mesh->normals.push(get_ai_vertex(ai_mesh->mNormals[i1]));
        mesh->normals.push(get_ai_vertex(ai_mesh->mNormals[i2]));
      }
    }

    mesh->has_vertex_colors = ai_mesh->HasVertexColors(0);
    mesh->has_normals = ai_mesh->HasNormals();

    aiMaterial *material = scene->mMaterials[ai_mesh->mMaterialIndex];
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
      aiString name;
      if (material->GetTexture(aiTextureType_DIFFUSE, 0, &name) == AI_SUCCESS) {

        String8 material_file_name = str8((u8 *)name.data, name.length);
        if (path_is_relative(material_file_name)) {
          model_path += "/" + std::string((char *)material_file_name.data);
          material_file_name = str8((u8 *)model_path.data(), model_path.size());
        }

        Texture *texture = r_create_texture_from_file(material_file_name);
        mesh->material.texture = texture;
        // printf("material:%s\n", name.data);
        // printf("Error loading texture:%s\n", material_file.data());
      }
    }

    aiColor4D diffuse_color;
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse_color) == AI_SUCCESS) {
      mesh->material.diffuse_color.x = diffuse_color.r;
      mesh->material.diffuse_color.y = diffuse_color.g;
      mesh->material.diffuse_color.z = diffuse_color.b;
      mesh->material.diffuse_color.w = diffuse_color.a;
    } else {
      mesh->material.diffuse_color = {1, 1, 1, 1}; 
    }
  }

  return model;
}

