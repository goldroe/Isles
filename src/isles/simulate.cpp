
internal void update_guy_mirror(Guy *guy) {
  World *world = get_world();
  guy->mirror_id = 0;

  Auto_Array<Entity*> mirrors;
  for (int i = 0; i < world->entities.count; i++) {
    if (world->entities[i]->kind == ENTITY_MIRROR) {
      mirrors.push(world->entities[i]);
    }
  }

  for (int i = 0; i < mirrors.count; i++) {
    Entity *mirror = mirrors[i];

    Vector3 forward = to_vec3(rotate_rh(-mirror->theta, Vector3(0, 1, 0)) * Vector4(1, 0, 0, 1));
    Vector3 dist = guy->position - mirror->position;
    if (dist.y != 0) break;
    f32 dx = forward.x > 0 ? 1.0f : -1.0f;
    f32 dz = forward.z > 0 ? 1.0f : -1.0f;

    Vector3 position = mirror->position;
    if ((int)mirror->position.z == (int)guy->position.z && (dist.x > 0 == dx > 0)) {
      // Move X
      for (;;) {
        position.x += dx;
        Entity *e = find_entity_at(world, position);
        if (e) {
          if (e == guy) {
            guy->mirror_id = mirror->id;
          }
          break;
        }
      }
    } else if ((int)mirror->position.x == (int)guy->position.x && (dist.z > 0 == dz > 0)) {
      // Move Z
      for (;;) {
        position.z += dz;
        Entity *e = find_entity_at(world, position);
        if (e) {
          if (e == guy) {
            guy->mirror_id = mirror->id;
          }
          break;
        }
      }
    }
  }

  mirrors.clear();

  Entity *mirror = lookup_entity(guy->mirror_id);
  if (mirror) {
    Vector3 mirror_forward = forward_from_theta(-mirror->theta);
    Vector3 direction = guy->position - mirror->position;
    int distance = (int)Abs(length(direction));

    if (direction.x) {
      f32 dz = roundf(mirror_forward.z);
      Vector3 target = mirror->position;
      target.z += distance * dz;
      Vector3 new_position = mirror->position;
      new_position.z += dz;

      for (;;) {
        Entity *e = find_entity_at(get_world(), new_position);
        if (e) {
          new_position.z -= dz;
          break;
        }
        if (new_position == target) {
          break;
        }
        new_position.z += dz;
      }

      guy->reflect_target = target;
      guy->reflect_position = new_position;

    } else if (direction.z) {
      f32 dx = roundf(mirror_forward.x);
      Vector3 target = mirror->position;
      target.x += distance * dx;
      Vector3 new_position = mirror->position;
      new_position.x += dx;

      for (;;) {
        Entity *e = find_entity_at(get_world(), new_position);
        if (e) {
          new_position.x -= dx;
          break;
        }
        if (new_position == target) {
          break;
        }
        new_position.x += dx;
      }

      guy->reflect_target = target;
      guy->reflect_position = new_position;
    }
  }

  if (key_pressed(OS_KEY_SPACE) && lookup_entity(guy->mirror_id)) {
    Action *move = action_make(ACTION_TELEPORT);
    move->actor_id = guy->id;
    move->from = guy->position;
    move->to = guy->reflect_position;
    guy->set_position(guy->reflect_position);
    play_effect("swosh.flac");
  }
}

internal bool move_entity(Entity *e, Vector3 distance) {
  Vector3 new_position = e->position + distance;
  Entity *wall = find_entity_at(get_world(), new_position);
  Entity *below = find_entity_at(get_world(), new_position - Vector3(0, 1, 0));

  bool valid_move = true;

  Action *move = action_make(ACTION_MOVE);
  move->actor_id = e->id;
  move->from = e->position;
  move->to = e->position + distance;

  if (wall) {
    if (wall->flags & ENTITY_FLAG_PUSHABLE) {
      valid_move = move_entity(wall, distance);
    } else {
      valid_move = false;
    }
  } else {
    if (e->kind == ENTITY_GUY && !below) {
      valid_move = false;
    }
  }

  if (valid_move) {
    e->position = new_position;
    // if (!below) {
    //   e->position.y -= 1;
    // }
  } else {
    action_pop();
  }

  return valid_move;
}

internal AABB get_world_bounds(World *world) {
  AABB bounds = {};

  for (int i = 0; i < world->entities.count; i++) {
    Entity *e = world->entities[i];
    if (e->kind == ENTITY_SUN) continue;
    if (e->position.x < bounds.min.x) bounds.min.x = (f32)e->position.x;
    if (e->position.y < bounds.min.y) bounds.min.y = (f32)e->position.y;
    if (e->position.z < bounds.min.z) bounds.min.z = (f32)e->position.z;
    if (e->position.x > bounds.max.x) bounds.max.x = (f32)e->position.x;
    if (e->position.y > bounds.max.y) bounds.max.y = (f32)e->position.y;
    if (e->position.z > bounds.max.z) bounds.max.z = (f32)e->position.z;
  }

  return bounds;
}

internal void update_sun(Sun *sun) {
  World *world = get_world();
  AABB bounds = world->bounding_box;
  Vector3 center = get_aabb_center(bounds);
  Vector3 bound_dim = get_aabb_dimension(bounds);
  f32 radius = get_max_elem(bound_dim);

  sun->light_direction = normalize(sun->light_direction);

  Vector3 light_position = center;
  Vector3 light_target = light_position + sun->light_direction;

  sun->light_projection = ortho_rh_zo(center.x - radius, center.x + radius, center.y - radius, center.y + radius, center.z - radius, center.z + radius);
  // sun->light_projection = ortho_rh_zo(center.x - bound_dim.x, center.x + bound_dim.x, center.y - bound_dim.y, center.y + bound_dim.y, center.z - bound_dim.z, center.z + bound_dim.z);
  sun->light_view = look_at_rh(light_position, light_target, Vector3(0, 1, 0));
  sun->light_space_matrix = sun->light_projection * sun->light_view;
}

void update_guy(Guy *guy) {
  update_guy_mirror(guy);

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
    Vector3 right = normalize(cross(forward, Vector3(0, 1, 0)));

    if (forward_dt) {
      move_direction = forward * (f32)forward_dt;
    } else if (right_dt) {
      move_direction = right * (f32)right_dt;
    }

    moved = move_entity(guy, move_direction);

    {
      f32 theta = atan2f((f32)-move_direction.z, (f32)move_direction.x);
      guy->theta_target = theta;
    }


    if (moved) {
      play_effect("tile_0.ogg");
    }
  }

  if (key_pressed(OS_KEY_Z)) {
    undo();
  }
}

internal void update_entity(Entity *e, f32 dt) {
  switch (e->kind) {

  case ENTITY_GUY:
  {
    Guy *guy = static_cast<Guy*>(e);
    update_guy(guy);
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

internal void simulate() {
  World *world = get_world();
  if (!game_state->editing && !game_state->paused) {
    Vector2 mouse_delta = get_mouse_delta();
    update_camera_orientation(&game_state->camera, mouse_delta);
    update_camera_position(&game_state->camera);
  }

  if (!game_state->editing && !game_state->paused) {
    //@Note Update entities
    for (int i = 0; i < world->entities.count; i++) {
      Entity *e = world->entities[i];
      update_entity(e, game_state->dt);
    }
  }

  {
    Sun *sun = get_sun(world);
    if (sun) update_sun(sun);
  }

  world->bounding_box = get_world_bounds(world);
}

internal void update_and_render(OS_Event_List *events, OS_Handle window_handle, f32 dt) {
  Editor *editor = get_editor();

  audio_engine->origin = game_state->camera.origin;
  audio_engine->update();

  ui_g_state->animation_dt = dt;

  r_d3d11_update_dirty_shaders();
    
  Vector2 window_dim = os_get_window_dim(window_handle);

  game_state->dt = dt / 1000.0f;
  game_state->window_dim = to_vec2i(window_dim);

  g_viewport->dimension = window_dim;
  g_viewport->window_handle = window_handle;

  os_set_cursor(OS_Cursor_Arrow);

  f32 aspect = (f32)game_state->window_dim.x / (f32)game_state->window_dim.y;
  game_state->camera.projection_matrix = perspective_rh(game_state->camera.fov, aspect, 0.1f, 1000.0f);
  editor->camera.projection_matrix     = perspective_rh(editor->camera.fov, aspect, 0.1f, 1000.0f);

  input_begin(window_handle, events);

  // draw_begin(window_handle);

  r_resize_render_target(game_state->window_dim);

  r_d3d11_begin(window_handle);

  r_clear_color(0.0f, 0.0f, 0.0f, 1.0f);

  for (int i = 0; i < get_world()->entities.count; i++) {
    Entity *e = get_world()->entities[i];
    if (e->to_be_destroyed) {
      get_world()->entities.remove(i);
      entity_free(e);
    }
  }

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

  simulate();
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

