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

internal inline Matrix4 to_matrix4(aiMatrix4x4 m) {
  Matrix4 M;
  M.columns[0] = Vector4(m.a1, m.b1, m.c1, m.d1);
  M.columns[1] = Vector4(m.a2, m.b2, m.c2, m.d2);
  M.columns[2] = Vector4(m.a3, m.b3, m.c3, m.d3);
  M.columns[3] = Vector4(m.a4, m.b4, m.c4, m.d4);
  return M;
}

internal Triangle_Mesh *load_mesh(std::string file_name) {
  u32 import_flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals;

  const aiScene *scene = assimp_importer.ReadFile(file_name.c_str(), import_flags);
  if (!scene) {
    logprint("Failed to load mesh '%s'.\n", file_name.c_str());
    logprint("Error: %s\n", assimp_importer.GetErrorString());
    return nullptr;
  }

  bool has_animation = scene->mNumAnimations > 0;

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

        Texture *texture = create_texture_from_file(material_file_name, TEXTURE_FLAG_GENERATE_MIPS);
        material->texture = texture;
      } else {
        logprint("Failed to load diffuse texture '%s'.\n", name.data);
      }
    }

    mesh->materials.push(material);
  }

  Auto_Array<Vertex_XNCUU> vertices;
  vertices.reserve(mesh->vertices.count);
  for (int i = 0; i < mesh->vertices.count; i++) {
    Vertex_XNCUU vertex;
    vertex.position = mesh->vertices[i];
    vertex.normal = mesh->normals[i];
    vertex.color = Vector4(1, 1, 1, 1);
    vertex.uv = mesh->uvs[i];
    vertices.push(vertex);
  }
  mesh->vertex_buffer = make_vertex_buffer(vertices.data, vertices.count, sizeof(Vertex_XNCUU));

  //@Note Animation

  read_animation(scene, mesh);

  return mesh;
}

internal Bone *bone_create(std::string name, int id, aiNodeAnim *channel) {
  Bone *bone = new Bone;
  bone->name = name;
  bone->id = id;
  bone->local_transform = make_matrix4(1.0f);

  bone->translations.reserve(channel->mNumPositionKeys);
  for (uint i = 0; i < channel->mNumPositionKeys; i++) {
    aiVectorKey v = channel->mPositionKeys[i];
    Key_Vector key;
    key.time = v.mTime;
    key.value = to_vec3(v.mValue);
    bone->translations.push(key);
  }

  bone->rotations.reserve(channel->mNumRotationKeys);
  for (uint i = 0; i < channel->mNumRotationKeys; i++) {
    aiQuatKey v = channel->mRotationKeys[i];
    Key_Quat key;
    key.time = v.mTime;
    key.value = Quaternion(v.mValue.x, v.mValue.y, v.mValue.z, v.mValue.w);
    bone->rotations.push(key);
  }

  bone->scalings.reserve(channel->mNumScalingKeys);
  for (uint i = 0; i < channel->mNumScalingKeys; i++) {
    aiVectorKey v = channel->mScalingKeys[i];
    Key_Vector key;
    key.time = v.mTime;
    key.value = to_vec3(v.mValue);
    bone->scalings.push(key);
  }

  return bone;
}

internal void read_animation(const aiScene *scene, Triangle_Mesh *mesh) {
  if (scene->mNumAnimations == 0) return;

  mesh->animated = true;

  uint vertex_idx = 0;
  for (uint i = 0; i < scene->mNumMeshes; i++) {
    aiMesh *amesh = scene->mMeshes[i];
    Vertex_Skinned v;
    for (uint j = 0; j < amesh->mNumVertices; j++) {
      Vector3 p = to_vec3(amesh->mVertices[j]);
      Vector3 n = to_vec3(amesh->mNormals[j]);
      Vector2 uv = Vector2();
      if (amesh->HasTextureCoords(0)) {
        uv = to_vec2(amesh->mTextureCoords[0][j]);
      }
      v.position = p;
      v.normal = n;
      v.uv = uv;
      v.bone_weights[0] = v.bone_weights[1] = v.bone_weights[2] = v.bone_weights[3] = 0;
      v.bone_ids[0] = v.bone_ids[1] = v.bone_ids[2] = v.bone_ids[3] = -1;
      mesh->skinned_vertices.push(v); 
    }

    for (u32 j = 0; j < amesh->mNumBones; j++) {
      aiBone *abone = amesh->mBones[j];
      std::string bone_name = abone->mName.C_Str();
      int bone_id = -1;
      if (mesh->bone_info_map.find(bone_name) != mesh->bone_info_map.end()) {
        bone_id = mesh->bone_info_map[bone_name].id;
      } else {
        bone_id = mesh->bone_counter++;
        Bone_Info bone_info;
        bone_info.id = bone_id;
        bone_info.offset_matrix = to_matrix4(abone->mOffsetMatrix);
        mesh->bone_info_map.insert({bone_name, bone_info});
      }

      for (u32 w = 0; w < abone->mNumWeights; w++) {
        aiVertexWeight weight = abone->mWeights[w];
        Vertex_Skinned *vertex = &mesh->skinned_vertices.data[vertex_idx + weight.mVertexId];
        set_vertex_bone_data(vertex, bone_id, weight.mWeight);
      }
    }

    for (uint i = 0; i < amesh->mNumFaces; i++) {
      aiFace *face = amesh->mFaces + i;
      assert(face->mNumIndices == 3);
      mesh->skinned_indices.push(face->mIndices[0]);
      mesh->skinned_indices.push(face->mIndices[1]);
      mesh->skinned_indices.push(face->mIndices[2]);
    }

    vertex_idx += amesh->mNumVertices;
  }

  mesh->anim_root = new Animation_Node;

  for (uint i = 0; i < scene->mNumAnimations; i++) {
    aiAnimation *ai_anim = scene->mAnimations[i];

    std::string name = ai_anim->mName.C_Str();

    Animation *animation = new Animation();
    animation->mesh = mesh;
    animation->name = name;
    animation->duration = ai_anim->mDuration;
    animation->ticks_per_second = ai_anim->mTicksPerSecond;

    for (uint channel_idx = 0; channel_idx < ai_anim->mNumChannels; channel_idx++) {
      aiNodeAnim *channel = ai_anim->mChannels[channel_idx];
      std::string node_name = channel->mNodeName.data;

      if (mesh->bone_info_map.find(node_name) == mesh->bone_info_map.end()) {
        Bone_Info info = {};
        info.id = mesh->bone_counter++;
        mesh->bone_info_map.insert({node_name, info});
      }

      Bone *bone = bone_create(node_name, mesh->bone_info_map[node_name].id, channel);
      animation->bones.push(bone);
    }

    mesh->animation_map.insert({name, animation});
  }

  read_animation_node(scene->mRootNode, mesh->anim_root);


  mesh->skinned_vertex_buffer = make_vertex_buffer(mesh->skinned_vertices.data, mesh->skinned_vertices.count, sizeof(Vertex_Skinned));
  mesh->skinned_index_buffer = make_index_buffer(mesh->skinned_indices.data, mesh->skinned_indices.count * sizeof(u32));
}

internal void read_animation_node(aiNode *node, Animation_Node *anim_node) {
  anim_node->name = node->mName.data;
  anim_node->transform = to_matrix4(node->mTransformation);
  for (uint i = 0; i < node->mNumChildren; i++) {
    Animation_Node *child = new Animation_Node;
    read_animation_node(node->mChildren[i], child);
    anim_node->children.push(child);
  }
}

internal void set_vertex_bone_data(Vertex_Skinned *vertex, int bone_id, f32 weight) {
  for (int i = 0; i < MAX_BONE_WEIGHTS; i++) {
    if (vertex->bone_ids[i] == -1) {
      vertex->bone_ids[i] = bone_id;
      vertex->bone_weights[i] = weight;
      break;
    }
  }
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
