#if !defined(MANAGER_H)
#define MANAGER_H

struct Entity_Manager {
  struct {
    // Auto_Array<Inanimate*> _Inanimate;
    Auto_Array<Guy*>    _Guy;
    Auto_Array<Mirror*> _Mirror;
    Auto_Array<Sun*>    _Sun;
  } by_type;
  Auto_Array<Entity*> entities;

  Pid next_pid = 1;
};


internal void reset_manager();
internal void entity_push(Entity_Manager *manager, Entity *e);

#endif //MANAGER_H
