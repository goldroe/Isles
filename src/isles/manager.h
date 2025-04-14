#if !defined(MANAGER_H)
#define MANAGER_H

struct Entity_Manager {
  struct {
    struct {
      // Auto_Array<Inanimate*> Inanimate;
      Auto_Array<Guy*> Guy;
      Auto_Array<Mirror*> Mirror;
      Auto_Array<Sun*> Sun;
    } _type;
    Auto_Array<Entity*> all;
  } entities;

  Pid next_pid = 1;
};

internal void entity_push(Entity_Manager *manager, Entity *e);

#endif //MANAGER_H
