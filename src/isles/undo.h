#ifndef UNDO_H
#define UNDO_H

enum Action_Kind {
  ACTION_NIL,
  ACTION_MOVE,
  ACTION_TELEPORT,
};

struct Action {
  Action *next;
  Action_Kind kind;
  Pid actor_id;

  Vector3Int from;
  Vector3Int to;
};

struct Undo_Stack {
  Arena *arena = nullptr;
  Action *top = nullptr;
  Action *free_nodes = nullptr;
  int count = 0;
};

#endif // UNDO_H
