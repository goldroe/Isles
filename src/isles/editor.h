#ifndef EDITOR_H
#define EDITOR_H

struct World;
struct Picker;

enum Gizmo_Kind {
  GIZMO_NIL = -1,
  GIZMO_TRANSLATE,
  GIZMO_ROTATE,
  GIZMO_SCALE,
  GIZMO_COUNT
};

struct Entity_Prototype {
  char *name;
  u64 id;
  Entity entity;
};

enum UI_Widget_Kind {
  UI_WIDGET_NIL,
  UI_WIDGET_LINE_EDIT,
};

struct UI_Widget {
  UI_Widget_Kind kind;
};

struct UI_Line_Edit {
  String8 name;
  UI_Box *box = nullptr;
  UI_Signal sig;

  u8 *buffer = nullptr;
  int buffer_pos = 0;
  int buffer_len = 0;
  int buffer_capacity = 0;
};

// struct UI_Button {
// };

struct Editor_Panel {
  b32 expand_load_world = false;
  UI_Line_Edit *edit_load_world = nullptr;
  b32 expand_saveas = false;
  UI_Line_Edit *edit_saveas = nullptr;
};

struct Entity_Panel {
  UI_Line_Edit *position_x = nullptr;
  UI_Line_Edit *position_y = nullptr;
  UI_Line_Edit *position_z = nullptr;
  UI_Line_Edit *theta = nullptr;
  UI_Line_Edit *color_r = nullptr;
  UI_Line_Edit *color_g = nullptr;
  UI_Line_Edit *color_b = nullptr;
};

struct Editor {
  std::unordered_map<u64,Entity_Prototype*> prototype_table;
  Auto_Array<Entity_Prototype*> prototype_array;

  Gizmo_Kind active_gizmo = GIZMO_TRANSLATE;
  Axis gizmo_axis_hover = AXIS_X;
  Axis gizmo_axis_active;
  Vector2Int gizmo_mouse_start = Vector2Int(0, 0);

  Editor_Panel *panel = nullptr;
  Entity_Panel *entity_panel = nullptr;
  Entity *selected_entity = nullptr;
  int prototype_idx = 0;

  Camera camera;

  Triangle_Mesh *gizmo_meshes[GIZMO_COUNT][3];

  void update_entity_panel();
  void select_entity(Entity *e);
};

internal Picker *make_picker(int width, int height);
internal inline Editor *get_editor();
internal Entity_Prototype *entity_prototype_lookup(u64 prototype_id);

#endif // EDITOR_H
