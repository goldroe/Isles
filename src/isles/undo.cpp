global Undo_Stack *undo_stack;

internal Action *action_alloc() {
  Action *result = nullptr;  
  if (undo_stack->free_nodes) {
    result = undo_stack->free_nodes;
    SLLStackPop(undo_stack->free_nodes);
  } else {
    result = push_array(undo_stack->arena, Action, 1);
    memset(result, 0, sizeof(Action));
  }
  SLLStackPush(undo_stack->top, result);
  undo_stack->count++;
  return result;
}

internal Action *action_make(Action_Kind kind) {
  Action *result = action_alloc();
  result->kind = kind;
  return result;
}

internal void action_pop() {
  Action *node = undo_stack->top;
  SLLStackPop(undo_stack->top);
  undo_stack->count--;
  SLLStackPush(undo_stack->free_nodes, node);
}

internal void undo() {
  Entity_Manager *manager = get_entity_manager();
  Guy *guy = manager->entities._type.Guy[0];

  while (undo_stack->top) {
    Action *action = undo_stack->top;
    switch (action->kind) {
    case ACTION_MOVE:
    {
      Entity *actor = lookup_entity(action->actor_id);
      if (actor) {
        actor->set_position(action->to);
        actor->position = action->from;
      }
      break;
    }
    case ACTION_TELEPORT:
    {
      Entity *actor = lookup_entity(action->actor_id);
      if (actor) {
        actor->set_position(action->from);
      }
      break;
    }
    }

    action_pop();
    if (action->actor_id == guy->id) {
      break;
    }
  }
}
