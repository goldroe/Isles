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
  Font *icon_font;
  b32 expand_load_world = false;
  UI_Line_Edit *edit_load_world = nullptr;
  b32 expand_saveas = false;
  UI_Line_Edit *edit_saveas = nullptr;
};

enum Field_Kind {
  FIELD_NIL,
  FIELD_INT,
  FIELD_FLOAT,
  FIELD_VEC2,
  FIELD_VEC3,
  FIELD_VEC4,
};

struct Entity_Field {
  String8 name;
  b32 expand = 0;
  Field_Kind kind = FIELD_NIL;
  Auto_Array<UI_Line_Edit*> fields;
  b32 dirty = 0;
};

enum Entity_Tab {
  ENTITY_TAB_FIRST,
  ENTITY_TAB_COMMON,
  ENTITY_TAB_FLAGS,
  ENTITY_TAB_COUNT
};

struct Entity_Panel {
  Font *icon_font;
  b32 dirty;

  Entity_Tab active_tab = ENTITY_TAB_COMMON;

  Auto_Array<Entity_Field*> common_fields;
  Auto_Array<Entity_Field*> entity_fields[ENTITY_COUNT];

  // common
  Entity_Field *position_field = nullptr;
  Entity_Field *color_field = nullptr;
  Entity_Field *theta_field = nullptr;

  //sun
  Entity_Field *sun_dir_field = nullptr;
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

  Auto_Array<Entity*> selections;
  Entity *active_selection = nullptr;
  Entity *hover_entity = nullptr;
  
  int prototype_idx = 0;

  Camera camera;

  Triangle_Mesh *gizmo_meshes[GIZMO_COUNT][3];

  f32 select_strobe_t = 0;
  f32 select_strobe_max = 0;
  f32 select_strobe_target = 0;
};

internal Picker *make_picker(int width, int height);
internal inline Editor *get_editor();


internal Entity *entity_from_prototype(Entity_Prototype *prototype);
internal Entity_Prototype *entity_prototype_lookup(u64 prototype_id);

#endif // EDITOR_H
