#ifndef UI_CORE_H
#define UI_CORE_H

template<class T>
struct UI_Param {
  Auto_Array<T> elems;
  b32 pop_next = false;
  T nil_value = {};

  bool empty() {
    return elems.is_empty();
  }
  T top() {
    if (elems.count) return elems.back();
    else return nil_value;
  }
  void push(T v) {
    elems.push(v);
  }
  void pop() {
    elems.pop();
    pop_next = false;
  }
  void set_next(T v) {
    elems.push(v);
    pop_next = true;
  }
  void reset() {
    elems.reset_count();
    pop_next = false;
  }
  void auto_pop() {
    if (pop_next) pop();
  }
};

union UI_Rect {
  struct {
    f32 left, top, right, bottom;
  };
  struct {
    Vector2 tl, br;
  };
  struct {
    Vector2 coords[2];
  };
  float elements[4];
};

typedef u32 UI_Box_Flags;
enum {
  UI_BOX_FLAG_NIL = 0,
  UI_BOX_FLAG_CLICKABLE          = (1<<0),
  UI_BOX_FLAG_KEYBOARD_CONTROL   = (1<<1),
  UI_BOX_FLAG_TEXT_ELEMENT       = (1<<2),
  UI_BOX_FLAG_FIXED_X            = (1<<3),
  UI_BOX_FLAG_FIXED_Y            = (1<<4),
  UI_BOX_FLAG_FIXED_WIDTH        = (1<<5),
  UI_BOX_FLAG_FIXED_HEIGHT       = (1<<6),
  UI_BOX_FLAG_DRAW_BACKGROUND    = (1<<10),
  UI_BOX_FLAG_DRAW_HOT_EFFECTS   = (1<<11),
  UI_BOX_FLAG_DRAW_BORDER        = (1<<12),
};

typedef u64 UI_Key;

enum UI_Size_Kind {
  UI_SIZE_NIL,
  UI_SIZE_PIXELS,
  UI_SIZE_TEXT,
  UI_SIZE_PARENT_PCT,
  UI_SIZE_CHILDREN_SUM,
};

struct UI_Size {
  UI_Size_Kind kind;
  f32 value;
};

struct UI_Box;
#define UI_BOX_DRAW(name) void name(UI_Box *box, void *user_data)
typedef UI_BOX_DRAW(UI_Box_Draw_Proc);

struct UI_Box {
  UI_Box *parent;
  UI_Box *next;
  UI_Box *prev;
  UI_Box *first;
  UI_Box *last;

  UI_Key key;
  UI_Box_Flags flags;

  UI_Rect rect;
  String8 text;

  Vector2 fixed_position;
  Vector2 fixed_size;
  UI_Size size[2];
  Vector2 padding;
  f32 text_padding;
  Axis child_layout_axis;
  Font *font;
  Vector4 background_color;
  Vector4 text_color;
  Vector4 hot_color;
  f32 hot_t;
  f32 active_t;

  UI_Box_Draw_Proc *custom_draw_proc;
  void *custom_draw_data;
};

enum UI_Event_Kind {
  UI_EVENT_NIL,
  UI_EVENT_CLICK,
  UI_EVENT_PRESS,
  UI_EVENT_RELEASE,
  UI_EVENT_TEXT,
  UI_EVENT_COUNT
};

struct UI_Event {
  UI_Event *next;
  UI_Event_Kind kind;
  Vector2Int pos;
  OS_Key key;
  String8 text;
};

enum {
  UI_SIGNAL_FLAG_HOVER   = (1<<0),
  UI_SIGNAL_FLAG_CLICKED = (1<<1),
  UI_SIGNAL_FLAG_PRESSED = (1<<2),
  UI_SIGNAL_FLAG_RELEASED = (1<<3),
};
typedef u32 UI_Signal_Flags;

struct UI_Signal {
  UI_Signal_Flags flags;
  UI_Box *box;
  OS_Key key;
  String8 text;
};

struct UI_Vertex {
  Vector2 position;
  Vector2 uv;
  Vector4 color;
  Vector4 style;
};

struct UI_Draw_Batch {
  Font *font = nullptr;
  Auto_Array<UI_Vertex> vertices = {};
};

struct UI_Draw_Bucket {
  Matrix4 transform;
  Auto_Array<UI_Draw_Batch*> batches = {};
};

struct UI_State {
  Arena *arena = nullptr;

  Vector2Int window_dimension = {0, 0};
  Vector2Int mouse_position = {-1, -1};
  Auto_Array<UI_Event> events;

  f32 animation_dt = 0;

  b32 keyboard_captured = false;
  b32 mouse_captured = false;

  std::unordered_map<UI_Key,UI_Box*> box_table;

  UI_Box *root = nullptr;

  UI_Key hot_box_key = 0;
  UI_Key active_box_key = 0;
  UI_Key focus_active_box_key = 0;

  UI_Param<UI_Box*> parent_stack;
  UI_Param<Font*>   font_stack;
  UI_Param<UI_Size> pref_width_stack;
  UI_Param<UI_Size> pref_height_stack;
  UI_Param<f32>     fixed_width_stack;
  UI_Param<f32>     fixed_height_stack;
  UI_Param<f32>     fixed_x_stack;
  UI_Param<f32>     fixed_y_stack;
  UI_Param<f32>     text_padding_stack;
  UI_Param<f32>     padding_x_stack;
  UI_Param<f32>     padding_y_stack;
  UI_Param<Axis>    child_layout_axis_stack;
  UI_Param<Vector4> background_color_stack;
  UI_Param<Vector4> text_color_stack;
  UI_Param<Vector4> hover_color_stack;

  UI_Draw_Bucket *draw_bucket;
};

internal bool ui_input_captured();

#define ui_hover(sig) (sig.flags & UI_SIGNAL_FLAG_HOVER)
#define ui_clicked(sig) (sig.flags & UI_SIGNAL_FLAG_CLICKED)
#define ui_pressed(sig) (sig.flags & UI_SIGNAL_FLAG_PRESSED)

#endif // UI_CORE_H
