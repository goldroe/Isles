#if !defined(KEYMAP_H)
#define KEYMAP_H

typedef u32 Key;
typedef u32 Key_Mod;

#define COMMAND(name) void name()
typedef COMMAND(Key_Proc);

struct Key_Command {
  String8 name;
  Key_Proc *proc;
};

#define KEYMOD_SHIFT    (1<<16)
#define KEYMOD_CONTROL  (1<<17)
#define KEYMOD_ALT      (1<<18)

#define MAX_KEYMAP_CAP (1 << (16 + 3))

struct Key_Map {
  String8 name;
  Key_Command commands[MAX_KEYMAP_CAP];
};

internal Key make_key(u16 code, Key_Mod mod);

#endif //KEYMAP_H
