
internal Entity* find_support(Entity *e) {
  Entity *support = find_entity_at(e->position - Vector3(0, 1, 0));
  return support;
}

internal Mirror *get_facing_mirror(Guy *guy) {
  Mirror *mirror = nullptr;
  AABB bounds = game_state->bounding_box;
  Vector3 position = guy->position + guy->forward;
  while (in_bounds(bounds, position)) {
    Entity *entity = find_entity_at(position);
    if (entity) {
      if (entity->kind == ENTITY_MIRROR) {
        mirror = static_cast<Mirror*>(entity);
      }
      break;
    }
    position += guy->forward;
  }
  return mirror;
}

internal void maybe_mirror_teleport(Guy *guy) {
  Mirror *mirror = static_cast<Mirror*>(lookup_entity(guy->mirror_id));
  if (!mirror) return;

  f32 dist = Abs(distance(mirror->position, guy->position));
  Vector3 position = mirror->position;

  Vector3 last_dir = get_nearest_axis(guy->forward);
  while (dist > 0) {
    Vector3 outgoing = (Abs(last_dir.x) > Abs(last_dir.z)) ? mirror->reflection_vectors[1] : mirror->reflection_vectors[0];
    last_dir = outgoing;

    f32 mirror_dist = 0;
    Reflection_Node *next_node = (Abs(outgoing.x) > Abs(outgoing.z)) ? mirror->node->reflect_x : mirror->node->reflect_z;
    if (next_node) mirror_dist = Abs(distance(next_node->mirror->position, mirror->position));

    if (!next_node || dist < mirror_dist) {
      position = mirror->position + outgoing * dist;
      dist = 0;
    } else {
      position = mirror->position + outgoing * mirror_dist;
      dist -= mirror_dist;
    }

    if (next_node) mirror = next_node->mirror;
  }

  if (position != guy->position) {
    Action *move = action_make(ACTION_TELEPORT);
    move->actor_id = guy->id;
    move->from = guy->position;
    move->to = position;
    play_effect("swosh.flac");
    guy->set_position(position);
  }
}

internal void compute_reflection_graph() {
  Reflection_Graph *graph = game_state->reflection_graph;

  Entity_Manager *manager = get_entity_manager();
  AABB bounds = game_state->bounding_box;

  for (Reflection_Node *node : graph->nodes) {
    delete node;
  }

  graph->visiting.reset_count();
  graph->nodes.reset_count();

  for (Mirror *mirror : manager->by_type._Mirror) {
    Reflection_Node *node = new Reflection_Node();
    node->mirror = mirror;
    graph->nodes.push(node);
    graph->visiting.push(node);
    mirror->node = node;
  }

  while (graph->visiting.count > 0) {
    Reflection_Node *node = graph->visiting.back();
    Mirror *mirror = node->mirror;
    // X
    if (!node->reflect_x) {
      Vector3 dir = mirror->reflection_vectors[0];
      Vector3 position = mirror->position + dir;
      while (in_bounds(bounds, position)) {
        Entity *entity = find_entity_at(position);
        if (entity) {
          if (entity->kind == ENTITY_MIRROR) {
            Mirror *mirror = static_cast<Mirror*>(entity);
            // facing toward
            if (dot_product(node->mirror->reflection_vectors[0], mirror->reflection_vectors[0]) < 0) {
              node->reflect_x = mirror->node;
              mirror->node->reflect_x = node;
            }
          }
          break;
        }
        position += dir;
      }
    }

    // Z
    if (!node->reflect_z) {
      Vector3 dir = mirror->reflection_vectors[1];
      Vector3 position = mirror->position + dir;
      while (in_bounds(bounds, position)) {
        Entity *entity = find_entity_at(position);
        if (entity) {
          if (entity->kind == ENTITY_MIRROR) {
            Mirror *mirror = static_cast<Mirror*>(entity);
            if (dot_product(node->mirror->reflection_vectors[1], mirror->reflection_vectors[1]) < 0) {
              node->reflect_z = mirror->node;
              mirror->node->reflect_z = node;
            }
          }
          break;
        }
        position += dir;
      }
    }


    graph->visiting.pop();
  }
}

internal void signal_world_step() {
  Game_State *state = get_game_state();
  state->can_world_step = false;
  state->world_step_dt = 0;
}

internal bool move_entity(Entity *e, Vector3 distance) {
  World *world = get_world();
  Vector3 new_position = e->position + distance;
  Entity *wall = find_entity_at(new_position);

  if (wall) {
    if (wall->flags & ENTITY_FLAG_PUSHABLE) {
      bool valid = move_entity(wall, distance);
      if (!valid) goto INVALID_MOVE;
    } else {
      goto INVALID_MOVE;
    }
  }

  Entity *support = find_support(e);
  if (!support && new_position.y == 0.0f) {
    goto INVALID_MOVE;
  }

  Action *move = action_make(ACTION_MOVE);
  move->actor_id = e->id;
  move->from = e->position;
  move->to = e->position + distance;
  e->position = new_position;

  signal_world_step();

  return true;

INVALID_MOVE:
  return false;
}

internal AABB get_world_bounds() {
  Entity_Manager *manager = get_entity_manager();
  AABB bounds = {};
  for (Entity *entity : manager->entities) {
    if (entity->kind == ENTITY_SUN) continue;
    if (entity->position.x < bounds.min.x) bounds.min.x = (f32)entity->position.x;
    if (entity->position.y < bounds.min.y) bounds.min.y = (f32)entity->position.y;
    if (entity->position.z < bounds.min.z) bounds.min.z = (f32)entity->position.z;
    if (entity->position.x > bounds.max.x) bounds.max.x = (f32)entity->position.x;
    if (entity->position.y > bounds.max.y) bounds.max.y = (f32)entity->position.y;
    if (entity->position.z > bounds.max.z) bounds.max.z = (f32)entity->position.z;
  }
  return bounds;
}

internal void update_sun(Sun *sun) {
  World *world = get_world();
  AABB bounds = get_game_state()->bounding_box;
  Vector3 center = get_aabb_center(bounds);
  Vector3 bound_dim = get_aabb_dimension(bounds);
  f32 radius = get_max_elem(bound_dim);

  sun->light_direction = normalize(sun->light_direction);

  Vector3 light_position = center;
  Vector3 light_target = light_position + sun->light_direction;

  sun->light_projection = ortho_rh_zo(center.x - radius, center.x + radius, center.y - radius, center.y + radius, center.z - radius, center.z + radius);
  sun->light_view = look_at_rh(light_position, light_target, Vector3(0, 1, 0));
  sun->light_space_matrix = sun->light_projection * sun->light_view;
}

internal Bone *find_bone(Animation *animation, std::string name) {
  for (int i = 0; i < animation->bones.count; i++) {
    if (animation->bones[i]->name == name) {
      return animation->bones[i];
    }
  }
  return nullptr;
}

internal inline f64 compute_bone_lerp_factor(f64 t, f64 last, f64 next) {
  f64 result = (t - last) / (next - last);
  return result;
}

internal int bone_get_translation_key(Bone *bone, f32 t) {
  for (int i = 0; i < bone->translations.count - 1; i++) {
    if (bone->translations[i].time <= t && t <= bone->translations[i + 1].time) {
      return i;
    }
  }
  return 0;
}

internal int bone_get_scale_key(Bone *bone, f32 t) {
  for (int i = 0; i < bone->scalings.count - 1; i++) {
    if (bone->scalings[i].time <= t && t <= bone->scalings[i + 1].time) {
      return i;
    }
  }
  return 0;
}

internal int bone_get_rotation_key(Bone *bone, f32 t) {
  for (int i = 0; i < bone->rotations.count - 1; i++) {
    if (bone->rotations[i].time <= t && t <= bone->rotations[i + 1].time) {
      return i;
    }
  }
  return 0;
}

internal Matrix4 bone_lerp_translation(Bone *bone, f32 t) {
  int key = bone_get_translation_key(bone, t);
  Key_Vector last_key = bone->translations[key];
  Key_Vector next_key = bone->translations[key + 1];

  f64 lerp_factor = compute_bone_lerp_factor(t, last_key.time, next_key.time);
  Vector3 lerp_vector = lerp(last_key.value, next_key.value, (f32)lerp_factor);
  Matrix4 result = translate(lerp_vector);
  return result;
}

internal Matrix4 bone_lerp_scale(Bone *bone, f32 t) {
  int key = bone_get_scale_key(bone, t);
  Key_Vector last_key = bone->scalings[key];
  Key_Vector next_key = bone->scalings[key + 1];

  f64 lerp_factor = compute_bone_lerp_factor(t, last_key.time, next_key.time);
  Vector3 lerp_vector = lerp(last_key.value, next_key.value, (f32)lerp_factor);
  Matrix4 result = scale(lerp_vector);
  return result;
}

internal Matrix4 bone_lerp_rotation(Bone *bone, f32 t) {
  int key = bone_get_rotation_key(bone, t);
  Key_Quat last_key = bone->rotations[key];
  Key_Quat next_key = bone->rotations[key + 1];

  f64 lerp_factor = compute_bone_lerp_factor(t, last_key.time, next_key.time);
  Quaternion quat = slerp(last_key.value, next_key.value, (f32)lerp_factor);
  Matrix4 result = matrix_from_quaternion(quat);
  return result;
}

internal void update_bone(Bone *bone, f32 t) {
  Matrix4 translation = bone_lerp_translation(bone, t);
  Matrix4 scale = bone_lerp_scale(bone, t);
  Matrix4 rotation = bone_lerp_rotation(bone, t);
  bone->local_transform = translation * scale * rotation;
  // bone->local_transform = make_matrix4(1.0f);
}

internal void compute_bone_transform(Animation_State *state, Animation_Node *node, Matrix4 parent_transform) {
  Matrix4 node_transform = node->transform;

  Bone *bone = find_bone(state->animation, node->name);
  if (bone) {
    f32 t = (f32)(state->t * state->animation->ticks_per_second);
    update_bone(bone, t);
    node_transform = bone->local_transform;
  }

  Matrix4 global_transform = parent_transform * node_transform;

  Animation *animation = state->animation;
  if (animation->mesh->bone_info_map.find(node->name) != animation->mesh->bone_info_map.end()) {
    Bone_Info bone_info = animation->mesh->bone_info_map[node->name];
    if (bone_info.id < MAX_BONES) {
      state->bone_transforms[bone_info.id] = global_transform * bone_info.offset_matrix;
    }
  }

  for (int i = 0; i < node->children.count; i++) {
    compute_bone_transform(state, node->children[i], global_transform);
  }
}

internal void compute_bone_transform(Animation_State *state) {
  compute_bone_transform(state, state->animation->mesh->anim_root, make_matrix4(1.0f));
}

internal void update_animation_state(Animation_State *state, f32 dt) {
  compute_bone_transform(state);

  state->t += dt;
  if (state->t > (state->animation->duration / state->animation->ticks_per_second)) {
    state->t = 0;
  }
}

internal void update_guy(Guy *guy, f32 dt) {
  //@Note Move
  Camera *camera = &game_state->camera;

  int forward_dt = 0;
  int right_dt = 0;
  if (key_pressed(OS_KEY_UP)) {
    forward_dt += 1;
  }
  if (key_pressed(OS_KEY_DOWN)) {
    forward_dt -= 1;
  }
  if (key_pressed(OS_KEY_LEFT)) {
    right_dt += -1;
  }
  if (key_pressed(OS_KEY_RIGHT)) {
    right_dt += 1;
  }

  bool moving = forward_dt || right_dt;
  bool moved = false;

  if (moving) {
    Vector3 move_direction = Vector3();

    Vector3 forward = camera->forward;
    forward.y = 0;
    forward = get_nearest_axis(forward);
    Vector3 right = normalize(cross_product(forward, Vector3(0, 1, 0)));

    if (forward_dt) {
      move_direction = forward * (f32)forward_dt;
    } else if (right_dt) {
      move_direction = right * (f32)right_dt;
    }

    moved = move_entity(guy, move_direction);

    guy->theta_target = atan2f((f32)-move_direction.z, (f32)move_direction.x);

    if (moved) {
      play_effect("tile_0.ogg");
    }
  }

  if (key_pressed(OS_KEY_Z)) {
    undo();
  }

  guy->forward = forward_from_theta(-guy->theta_target);
  Mirror *mirror = get_facing_mirror(guy);
  guy->mirror_id = mirror ? mirror->id : 0;

  if (key_pressed(OS_KEY_SPACE)) {
    maybe_mirror_teleport(guy);
  }

  //idle
  if (!guy->animation_state) {
    guy->animation_state = new Animation_State;
    Matrix4 m = make_matrix4(1.0f);
    for (int i = 0; i < MAX_BONES; i++) {
      guy->animation_state->bone_transforms[i] = m;
    }
  } 
  guy->animation_state->animation = guy->mesh->animation_map.begin()->second;
  update_animation_state(guy->animation_state, dt);
}

internal void update_mirror(Mirror *mirror) {
  Vector3 direction = forward_from_theta(-mirror->theta_target);
  mirror->reflection_vectors[0] = Vector3(direction.x / Abs(direction.x), 0, 0);
  mirror->reflection_vectors[1] = Vector3(0, 0, direction.z / Abs(direction.z));
}

internal void emit(Particle_Source *source, int particle) {
  Particles *particles = &source->particles;
  particles->lifetimes[particle] = get_random_float(0.1f, 5.0f);
  particles->positions[particle] = source->position + get_random_vec3();
  particles->velocities[particle] = get_random_vec3(Vector3(-1, 1, -1), Vector3(1, 5, 1));
  particles->scales[particle] = make_vec2(get_random_float(0.5f, 0.5f));
  particles->colors[particle] = make_vec4(1.0f);
}

internal void init_particle_source(Particle_Source *source) {
  source->initialized = true;

  Particles *particles = &source->particles;
  int particle_count = 1024;
  particles->count = particle_count;

  particles->lifetimes = new f32[particle_count];
  particles->positions = new Vector3[particle_count];
  particles->velocities = new Vector3[particle_count];
  particles->scales = new Vector2[particle_count];
  particles->colors = new Vector4[particle_count];

  for (int i = 0; i < particles->count; i++) {
    emit(source, i);
  }
}

internal void update_particle_source(Particle_Source *source, f32 dt) {
  if (!source->initialized) {
    init_particle_source(source);
  }

  Particles *particles = &source->particles;

  for (int i = 0; i < particles->count; i++) {
    if (particles->lifetimes[i] < 0.001f) {
      emit(source, i);
    }
    particles->lifetimes[i] -= dt;
  }

  Vector3 forces = Vector3(0.f, -9.81f, 0.f);

  for (int i = 0; i < particles->count; i++) {
    particles->velocities[i] += forces * dt;
  }

  for (int i = 0; i < particles->count; i++) {
    particles->positions[i] += particles->velocities[i] * dt + 0.5f * forces * dt * dt;
  }
}

internal void update_entity(Entity *e, f32 dt) {
  switch (e->kind) {
  case ENTITY_MIRROR:
  {
    Mirror *mirror = static_cast<Mirror*>(e);
    update_mirror(mirror);
    break;
  }

  case ENTITY_SUN:
  {
    Sun *sun = static_cast<Sun*>(e);
    update_sun(sun);
    break;
  }

  case ENTITY_PARTICLE_SOURCE:
  {
    Particle_Source *particle_source = static_cast<Particle_Source*>(e);
    update_particle_source(particle_source, dt);
    break;
  }
  }

  if (!(e->flags & ENTITY_FLAG_STATIC)) {
    f32 move_speed = 8.0f;
    if (Abs(e->visual_position.x - e->position.x) > 0.001f) {
      e->visual_position.x += (e->position.x - e->visual_position.x) * move_speed * dt;
    } else {
      e->visual_position.x = (f32)e->position.x;
    }
    if (Abs(e->visual_position.y - e->position.y) > 0.001f) {
      e->visual_position.y += (e->position.y - e->visual_position.y) * move_speed * dt;
    } else {
      e->visual_position.y = (f32)e->position.y;
    }
    if (Abs(e->visual_position.z - e->position.z) > 0.001f) {
      e->visual_position.z += (e->position.z - e->visual_position.z) * move_speed * dt;
    } else {
      e->visual_position.z = (f32)e->position.z;
    }

    f32 rot_speed = 8.0f;
    e->theta = e->theta +  get_shortest_delta(e->theta, e->theta_target) * rot_speed * dt;
  }
}

internal void simulate_world(f32 dt) {
  if (game_state->editing) return;

  compute_reflection_graph();

  Entity_Manager *manager = get_entity_manager();

  Vector2 mouse_delta = get_mouse_delta();
  update_camera_orientation(&game_state->camera, mouse_delta);
  update_camera_position(&game_state->camera);

  for (Entity *entity : manager->entities) {
    update_entity(entity, game_state->dt);
  }

  if (game_state->can_world_step) {
    Auto_Array<Entity*> unsupported;
    for (Entity *entity : manager->entities) {
      update_entity(entity, game_state->dt);
      if (entity->flags & ENTITY_FLAG_STATIC) continue;

      Entity *support = find_support(entity);
      if (!support && entity->position.y > 0.0f) {
        unsupported.push(entity);
      }
    }

    for (int i = 0; i < unsupported.count; i++) {
      Entity *e = unsupported[i];
      e->position.y -= 1;
      signal_world_step();
    }
    unsupported.clear();

  } else {
    game_state->world_step_dt += dt;
    if (game_state->world_step_dt > 0.2f) {
      game_state->can_world_step = true;
      game_state->world_step_dt = 0; 
    }
  }

  for (Guy *guy : manager->by_type._Guy) {
    update_guy(guy, dt);
  }

  get_game_state()->bounding_box = get_world_bounds();
}

internal void update_and_render(OS_Event_List *events, OS_Handle window_handle, f32 dt) {
  Editor *editor = get_editor();

  audio_engine->origin = game_state->camera.origin;
  audio_engine->update();

  r_d3d11_update_dirty_shaders();
    
  Vector2 window_dim = os_get_window_dim(window_handle);

  game_state->dt = dt;
  game_state->window_dim = to_vec2i(window_dim);

  g_viewport->dimension = window_dim;
  g_viewport->window_handle = window_handle;

  os_set_cursor(OS_Cursor_Arrow);

  f32 aspect = (f32)game_state->window_dim.x / (f32)game_state->window_dim.y;
  game_state->camera.projection_matrix = perspective_rh(game_state->camera.fov, aspect, 0.1f, 1000.0f);
  editor->camera.projection_matrix     = perspective_rh(editor->camera.fov, aspect, 0.1f, 1000.0f);

  input_begin(window_handle, events);

  r_resize_render_target(game_state->window_dim);

  r_d3d11_begin(window_handle);

  r_clear_color(0.0f, 0.0f, 0.0f, 1.0f);

  remove_entities_to_be_destroyed();

  if (game_state->paused) {
    input_set_mouse_capture(false);
  }

  if (game_state->editing) {
    input_set_mouse_capture(false);
    update_editor(events);
  }

  if (!game_state->paused && !game_state->editing) {
    input_set_mouse_capture(true);
  }

  simulate_world(dt);

  Entity_Manager *manager = get_entity_manager();
  for (Guy *guy : manager->by_type._Guy) {
    // Animation_Info *animation = guy->mesh->animation_info;
    // if (!animation) continue;
  }


  if (!game_state->editing) {
    draw_scene();
  }

  if (key_pressed(OS_KEY_F1) ||
      key_pressed(OS_KEY_1)) {
    if (!game_state->editing) {
      game_state->editing = true; 
      editor->camera = game_state->camera;
      update_entity_panel(editor);
    } else {
      game_state->editing = false; 
    }
  }

  //@Note UI Layout
  ui_begin(window_handle, events);

  ui_push_font(default_font);
  ui_push_background_color(Vector4(0.55f, 0.22f, 0.63f, 1.0f));
  ui_push_text_color(Vector4(1.0f, 1.0f, 1.0f, 1.0f));

  if (game_state->editing) {
    editor_present_ui(); 
  }

  ui_layout_apply();
  draw_ui_layout();

  r_d3d11_state()->swap_chain->Present(1, 0);

  input_end(window_handle);

  ui_end();
}

