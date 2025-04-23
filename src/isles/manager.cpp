
global Entity_Manager *entity_manager;

internal inline Entity_Manager *get_entity_manager() {
  return entity_manager;
}


internal void *entity_alloc(int bytes) {
  void *memory = calloc(1, bytes);
  return memory;
}

internal void entity_free(Entity *entity) {
  free(entity);
}

internal void reset_manager() {
  Entity_Manager *manager = get_entity_manager();

  for (int i = 0; i < manager->entities.count; i++) {
    Entity *entity = manager->entities[i];
    entity_free(entity);
  }

  manager->by_type._Guy.clear();
  manager->by_type._Mirror.clear();
  manager->by_type._Sun.clear();
  manager->by_type._Particle_Source.clear();
  manager->entities.clear();
  manager->next_pid = 1;
}

internal void entity_push(Entity_Manager *manager, Entity *entity) {
  switch (entity->kind) {
  default:
    break;
  case ENTITY_GUY:
    manager->by_type._Guy.push(static_cast<Guy*>(entity));
    break;
  case ENTITY_MIRROR:
    manager->by_type._Mirror.push(static_cast<Mirror*>(entity));
    break;
  case ENTITY_SUN:
    manager->by_type._Sun.push(static_cast<Sun*>(entity));
    break;
  case ENTITY_PARTICLE_SOURCE:
    manager->by_type._Particle_Source.push(static_cast<Particle_Source*>(entity));
    break;
  }
  manager->entities.push(entity);

  entity->id = manager->next_pid++;
}

internal void remove_entities_to_be_destroyed() {
  Entity_Manager *manager = get_entity_manager();

#define ENTITY_REMOVE(Arr) \
    for (int i = 0; i < Arr.count; i++) { \
    Entity *entity = Arr[i]; \
    if (entity->to_be_destroyed) { \
      Arr.remove(i); \
      i--; \
    } \
  } \

  ENTITY_REMOVE(manager->by_type._Guy);
  ENTITY_REMOVE(manager->by_type._Mirror);
  ENTITY_REMOVE(manager->by_type._Sun);
  ENTITY_REMOVE(manager->by_type._Particle_Source);

  for (int i = 0; i < manager->entities.count; i++) {
    Entity *entity = manager->entities[i];   
    if (entity->to_be_destroyed) {
      entity_free(entity);
      manager->entities.remove(i);
      i--;
    }
  }

#undef ENTITY_REMOVE
}

internal Entity *entity_make(Entity_Kind kind) {
  Entity_Manager *manager = get_entity_manager();
  Entity *e = nullptr;
  switch (kind) {
  default:
    e = (Entity *)entity_alloc(sizeof(Entity));
    break;
  case ENTITY_GUY:
    e = (Entity *)entity_alloc(sizeof(Guy));
    break;
  case ENTITY_MIRROR:
    e = (Entity *)entity_alloc(sizeof(Mirror));
    break;
  case ENTITY_SUN:
    e = (Entity *)entity_alloc(sizeof(Sun));
    break;
  case ENTITY_PARTICLE_SOURCE:
    e = (Entity *)entity_alloc(sizeof(Particle_Source));
    break;
  }
  e->kind = kind;
  e->override_color = Vector4(1, 1, 1, 1);
  entity_push(manager, e);
  return e;
}

internal Entity *lookup_entity(Pid id) {
  Entity_Manager *manager = get_entity_manager();
  if (id == 0) return nullptr;
  for (Entity *e : manager->entities) {
    if (e->id == id) return e;
  }
  return nullptr;
}
    
internal Entity *find_entity_at(Vector3 position) {
  Entity_Manager *manager = get_entity_manager();

  Vector3Int p0 = to_vec3i(position);
  for (Entity *e : manager->entities) {
    Vector3Int p1 = to_vec3i(e->position);
    if (p0 == p1) return e;
  }
  return nullptr;
}
