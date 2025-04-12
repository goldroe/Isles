
internal Key make_key(u16 code, Key_Mod mod) {
  Key key = (Key)code;
  key |= mod;
  return key;
}

internal Key_Map *create_key_map(String8 name) {
  Key_Map *key_map = new Key_Map();
  key_map->name = name;
  MemoryZero(key_map->commands, MAX_KEYMAP_CAP * sizeof(Key_Command));
  return key_map;
}

internal void key_map_bind(Key_Map *map, Key key, Key_Command command) {
  map->commands[key] = command;
}

internal void key_map_bind(Key_Map *map, Key key, String8 name, Key_Proc *proc) {
  Key_Command command = {};
  command.name = name;
  command.proc = proc;
  key_map_bind(map, key, command);
}

internal Key_Command *key_map_lookup(Key_Map *map, Key key) {
  Key_Command *command = &map->commands[key];
  return command;
}
