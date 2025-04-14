
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

internal void entity_push(Entity_Manager *manager, Entity *entity) {
  switch (entity->kind) {
  default:
  case ENTITY_GUY:
    manager->entities._type.Guy.push(static_cast<Guy*>(entity));
    break;
  case ENTITY_MIRROR:
    manager->entities._type.Mirror.push(static_cast<Mirror*>(entity));
    break;
  case ENTITY_SUN:
    manager->entities._type.Sun.push(static_cast<Sun*>(entity));
    break;
  }
  manager->entities.all.push(entity);

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

  ENTITY_REMOVE(manager->entities._type.Guy);
  ENTITY_REMOVE(manager->entities._type.Mirror);
  ENTITY_REMOVE(manager->entities._type.Sun);
  ENTITY_REMOVE(manager->entities._type.Guy);
  ENTITY_REMOVE(manager->entities.all);

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
  }
  e->kind = kind;
  e->override_color = Vector4(1, 1, 1, 1);
  entity_push(manager, e);
  return e;
}

internal Entity *lookup_entity(Pid id) {
  Entity_Manager *manager = get_entity_manager();
  if (id == 0) return nullptr;
  for (int i = 0; i < manager->entities.all.count; i++) {
    Entity *e = manager->entities.all[i];
    if (e->id == id) return e;
  }
  return nullptr;
}
    
internal Entity *find_entity_at(Vector3 position) {
  Entity_Manager *manager = get_entity_manager();

  Vector3Int p0 = to_vec3i(position);
  for (int i = 0; i < manager->entities.all.count; i++) {
    Entity *e = manager->entities.all[i];
    Vector3Int p1 = to_vec3i(e->position);
    if (p0 == p1) return e;
  }
  return nullptr;
}
