
global UI_State *ui_g_state;

internal void *ui_alloc(u64 bytes) {
  void *memory = calloc(1, bytes);
  return memory;
}

internal void ui_set_state(UI_State *ui_state) {
  ui_g_state = ui_state;
}

internal Arena *ui_build_arena() {
  return ui_g_state->arena;
}

global char *ui_g_icon_kind_strings[] = {
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z"
};
internal String8 ui_string_from_icon_kind(UI_Icon_Kind kind, const char *end) {
    char *icon_string = ui_g_icon_kind_strings[kind];
    String8 result = str8_pushf(ui_build_arena(), "%s%s", icon_string, end);
    return result;
}

internal bool ui_keyboard_captured() {
  return ui_g_state->keyboard_captured;
}

internal bool ui_mouse_captured() {
  return ui_g_state->mouse_captured;
}

internal void ui_set_keyboard_captured() {
  ui_g_state->keyboard_captured = true;
}

internal void ui_set_mouse_captured() {
  ui_g_state->mouse_captured = true;
}

internal UI_Box* ui_top_parent()            { return ui_g_state->parent_stack.top(); }
internal UI_Size ui_top_pref_width()        { return ui_g_state->pref_width_stack.top(); }
internal UI_Size ui_top_pref_height()       { return ui_g_state->pref_height_stack.top(); }
internal f32     ui_top_fixed_x()           { return ui_g_state->fixed_x_stack.top(); }
internal f32     ui_top_fixed_y()           { return ui_g_state->fixed_y_stack.top(); }
internal f32     ui_top_fixed_width()       { return ui_g_state->fixed_width_stack.top(); }
internal f32     ui_top_fixed_height()      { return ui_g_state->fixed_height_stack.top(); }
internal f32     ui_top_padding_x()         { return ui_g_state->padding_x_stack.top(); }
internal f32     ui_top_padding_y()         { return ui_g_state->padding_y_stack.top(); }
internal f32     ui_top_text_padding()      { return ui_g_state->text_padding_stack.top(); }
internal UI_Text_Align ui_top_text_align()  { return ui_g_state->text_align_stack.top(); }
internal Vector4 ui_top_border_color()      { return ui_g_state->border_color_stack.top(); }
internal Vector4 ui_top_background_color()  { return ui_g_state->background_color_stack.top(); }
internal Vector4 ui_top_text_color()        { return ui_g_state->text_color_stack.top(); }
internal Axis    ui_top_child_layout_axis() { return ui_g_state->child_layout_axis_stack.top(); }
internal Font*   ui_top_font()              { return ui_g_state->font_stack.top(); }
internal UI_Box_Flags ui_top_box_flags()    { return ui_g_state->box_flags_stack.top(); }

internal void ui_push_parent(UI_Box *v)           { ui_g_state->parent_stack.push(v); }
internal void ui_push_pref_width(UI_Size v)       { ui_g_state->pref_width_stack.push(v); }
internal void ui_push_pref_height(UI_Size v)      { ui_g_state->pref_height_stack.push(v); }
internal void ui_push_fixed_x(f32 v)              { ui_g_state->fixed_x_stack.push(v); }
internal void ui_push_fixed_y(f32 v)              { ui_g_state->fixed_y_stack.push(v); }
internal void ui_push_fixed_width(f32 v)          { ui_g_state->fixed_width_stack.push(v); }
internal void ui_push_fixed_height(f32 v)         { ui_g_state->fixed_height_stack.push(v); }
internal void ui_push_padding_x(f32 v)            { ui_g_state->padding_x_stack.push(v); }
internal void ui_push_padding_y(f32 v)            { ui_g_state->padding_y_stack.push(v); }
internal void ui_push_text_padding(f32 v)         { ui_g_state->text_padding_stack.push(v); }
internal void ui_push_text_align(UI_Text_Align v) { ui_g_state->text_align_stack.push(v); }
internal void ui_push_background_color(Vector4 v) { ui_g_state->background_color_stack.push(v); }
internal void ui_push_border_color(Vector4 v)     { ui_g_state->border_color_stack.push(v); }
internal void ui_push_text_color(Vector4 v)       { ui_g_state->text_color_stack.push(v); }
internal void ui_push_child_layout_axis(Axis v)   { ui_g_state->child_layout_axis_stack.push(v); }
internal void ui_push_font(Font *v)               { ui_g_state->font_stack.push(v); }
internal void ui_push_box_flags(UI_Box_Flags v)   { ui_g_state->box_flags_stack.push(v); }

internal void ui_pop_parent()            { ui_g_state->parent_stack.pop(); }
internal void ui_pop_pref_width()        { ui_g_state->pref_width_stack.pop(); }
internal void ui_pop_pref_height()       { ui_g_state->pref_height_stack.pop(); }
internal void ui_pop_fixed_x()           { ui_g_state->fixed_x_stack.pop(); }
internal void ui_pop_fixed_y()           { ui_g_state->fixed_y_stack.pop(); }
internal void ui_pop_fixed_width()       { ui_g_state->fixed_width_stack.pop(); }
internal void ui_pop_fixed_height()      { ui_g_state->fixed_height_stack.pop(); }
internal void ui_pop_padding_x()         { ui_g_state->padding_x_stack.pop(); }
internal void ui_pop_padding_y()         { ui_g_state->padding_y_stack.pop(); }
internal void ui_pop_text_padding()      { ui_g_state->text_padding_stack.pop(); }
internal void ui_pop_text_align()        { ui_g_state->text_align_stack.pop(); }
internal void ui_pop_background_color()  { ui_g_state->background_color_stack.pop(); }
internal void ui_pop_border_color()      { ui_g_state->border_color_stack.pop(); }
internal void ui_pop_text_color()        { ui_g_state->text_color_stack.pop(); }
internal void ui_pop_child_layout_axis() { ui_g_state->child_layout_axis_stack.pop(); }
internal void ui_pop_font()              { ui_g_state->font_stack.pop(); }
internal void ui_pop_box_flags()         { ui_g_state->box_flags_stack.pop(); }

internal void ui_set_next_parent(UI_Box *v)           { ui_g_state->parent_stack.set_next(v); }
internal void ui_set_next_pref_width(UI_Size v)       { ui_g_state->pref_width_stack.set_next(v); }
internal void ui_set_next_pref_height(UI_Size v)      { ui_g_state->pref_height_stack.set_next(v); }
internal void ui_set_next_fixed_x(f32 v)              { ui_g_state->fixed_x_stack.set_next(v); }
internal void ui_set_next_fixed_y(f32 v)              { ui_g_state->fixed_y_stack.set_next(v); }
internal void ui_set_next_fixed_width(f32 v)          { ui_g_state->fixed_width_stack.set_next(v); }
internal void ui_set_next_fixed_height(f32 v)         { ui_g_state->fixed_height_stack.set_next(v); }
internal void ui_set_next_padding_x(f32 v)            { ui_g_state->padding_x_stack.set_next(v); }
internal void ui_set_next_padding_y(f32 v)            { ui_g_state->padding_y_stack.set_next(v); }
internal void ui_set_next_text_padding(f32 v)         { ui_g_state->text_padding_stack.set_next(v); }
internal void ui_set_next_text_align(UI_Text_Align v) { ui_g_state->text_align_stack.set_next(v); }
internal void ui_set_next_background_color(Vector4 v) { ui_g_state->background_color_stack.set_next(v); }
internal void ui_set_next_border_color(Vector4 v)     { ui_g_state->border_color_stack.set_next(v); }
internal void ui_set_next_text_color(Vector4 v)       { ui_g_state->text_color_stack.set_next(v); }
internal void ui_set_next_child_layout_axis(Axis v)   { ui_g_state->child_layout_axis_stack.set_next(v); }
internal void ui_set_next_font(Font *v)               { ui_g_state->font_stack.set_next(v); }
internal void ui_set_next_box_flags(UI_Box_Flags v)   { ui_g_state->box_flags_stack.set_next(v); }

internal void ui_box_equip_draw_proc(UI_Box *box, UI_Box_Draw_Proc *proc, void *user_data) {
  box->custom_draw_proc = proc;
  box->custom_draw_data = user_data;
}

internal inline UI_Size ui_pixels(f32 v) {
  UI_Size size = {};
  size.kind = UI_SIZE_PIXELS;
  size.value = v;
  return size;
}

internal inline UI_Size ui_txt(f32 padding) {
  UI_Size size = {};
  size.kind = UI_SIZE_TEXT;
  return size;
}

internal inline UI_Size ui_pct() {
  UI_Size size = {};
  size.kind = UI_SIZE_PARENT_PCT;
  return size;
}

internal inline UI_Size ui_children_sum() {
  UI_Size size = {};
  size.kind = UI_SIZE_CHILDREN_SUM;
  return size;
}

internal inline UI_Size ui_pct(f32 v) {
  UI_Size size = {};
  size.kind = UI_SIZE_PARENT_PCT;
  size.value = v;
  return size;
}

internal f32 measure_text_width(String8 string, Font *font) {
  f32 result = 0.0f;
  for (int i = 0; i < string.count; i++) {
    Glyph g = font->glyphs[string.data[i]];
    result += g.ax;
  }
  return result;
}

internal f32 measure_text_height(String8 string, Font *font) {
  f32 result = 0.0f;
  for (int i = 0; i < string.count; i++) {
    if (string.data[i] == '\n') {
      result += font->glyph_height;
    }
  }
  if (result < font->glyph_height) result = font->glyph_height;
  return result;
}

internal Vector2 ui_box_text_position(UI_Box *box) {
  Vector2 text_position = box->rect.tl + make_vec2(box->text_padding);

  switch (box->text_align) {
  case UI_TEXT_ALIGN_CENTER:
  {
    f32 box_width = box->rect.right - box->rect.left;
    f32 text_width = measure_text_width(box->text, box->font) + box->text_padding;
    f32 end = text_position.x + text_width;
    text_position.x += 0.5f * (box_width - text_width);
    text_position.x = ClampBot(text_position.x, box->rect.left);
    break;
  }
  case UI_TEXT_ALIGN_LEFT:
  {
    break;
  }
  case UI_TEXT_ALIGN_RIGHT:
  {
    // f32 box_width = box->rect.right - box->rect.left;
    f32 text_width = measure_text_width(box->text, box->font) + box->text_padding;
    text_position.x = box->rect.right - text_width;
    text_position.x = ClampBot(text_position.x, box->rect.left);
  }
  }

  return text_position;
}

internal inline void ui_set_hot_key(UI_Key key) {
  ui_g_state->hot_box_key = key; 
}

internal inline void ui_set_active_key(UI_Key key) {
  ui_g_state->active_box_key = key; 
}

internal inline void ui_set_focus_active_key(UI_Key key) {
  ui_g_state->focus_active_box_key = key; 
}

internal inline bool ui_hot_key_match(UI_Key key) {
  return ui_g_state->hot_box_key == key;
}

internal inline bool ui_active_key_match(UI_Key key) {
  return ui_g_state->active_box_key == key;
}

internal inline bool ui_focus_active_key_match(UI_Key key) {
  return ui_g_state->focus_active_box_key == key;
}

internal String8 ui_hash_part(String8 string) {
  String8 result = string;
  for (int i = 0; i < string.count - 1; i++) {
    if (string.data[i] == '#' && string.data[i + 1] == '#') {
      result = str8(string.data + i, string.count - i);
      break;
    }
  }
  return result;
}

internal UI_Key ui_get_seed_key(UI_Box *parent) {
  UI_Key seed = 0;
  UI_Box *ancestor = parent;
  while (ancestor) {
    if (ancestor->key != 0) {
      seed = ancestor->key;
      break;
    }
    ancestor = ancestor->parent;
  }
  return seed;
}

internal UI_Key ui_hash_from_string(UI_Key seed, String8 string) {
  UI_Key key = seed;
  for (int i = 0; i < string.count; i++) {
    key = ((key << 5) + key) + string.data[i];
  }
  return key;
}

internal UI_Box *ui_find_box(UI_Key key) {
  UI_Box *found = nullptr;
  auto it = ui_g_state->box_table.find(key);
  if (it != ui_g_state->box_table.end()) {
    found = it->second;
  }
  return found;
}

internal void ui_push_box(UI_Key key, UI_Box *box) {
  ui_g_state->box_table.insert({key, box});
}

internal UI_Box *ui_box_create(UI_Box_Flags flags, UI_Key key) {
  UI_Box *box = ui_find_box(key);

  bool first_frame = box == nullptr;
  bool is_transient = (key == 0);

  if (first_frame) {
    box = (UI_Box *)ui_alloc(sizeof(UI_Box));
    if (!is_transient) {
      ui_push_box(key, box);
    }
  }

  //@Note Reset temp params
  box->parent = nullptr;
  box->first = nullptr;
  box->last = nullptr;
  box->next = nullptr;
  box->prev = nullptr;
  box->fixed_size = {0, 0};
  box->fixed_position = {0, 0};
  box->custom_draw_proc = nullptr;

  UI_Box *parent = ui_top_parent();
  if (parent) {
    DLLPushBack(parent->first, parent->last, box, next, prev);
  }
  box->parent = parent;

  box->key = key;
  box->flags = flags;

  box->font = ui_top_font();
  box->child_layout_axis = ui_top_child_layout_axis();
  box->size[AXIS_X] = ui_top_pref_width();
  box->size[AXIS_Y] = ui_top_pref_height();
  box->text_padding = ui_top_text_padding();
  box->text_align = ui_top_text_align();
  box->padding[AXIS_X] = ui_top_padding_x();
  box->padding[AXIS_Y] = ui_top_padding_y();
  box->background_color = ui_top_background_color();
  box->text_color = ui_top_text_color();
  box->border_color = ui_top_border_color();

  if (!ui_g_state->box_flags_stack.empty()) {
    box->flags |= ui_top_box_flags();
  }

  if (!ui_g_state->fixed_x_stack.empty()) {
    box->flags |= UI_BOX_FLAG_FIXED_X;
    box->fixed_position.x = ui_top_fixed_x();
  }
  if (!ui_g_state->fixed_y_stack.empty()) {
    box->flags |= UI_BOX_FLAG_FIXED_Y;
    box->fixed_position.y = ui_top_fixed_y();
  }

  if (!ui_g_state->fixed_width_stack.empty()) {
    box->flags |= UI_BOX_FLAG_FIXED_WIDTH;
    box->fixed_size.x = ui_top_fixed_width();
    box->size[0].kind = UI_SIZE_NIL;
  }
  if (!ui_g_state->fixed_height_stack.empty()) {
    box->flags |= UI_BOX_FLAG_FIXED_HEIGHT;
    box->fixed_size.y = ui_top_fixed_height();
    box->size[1].kind = UI_SIZE_NIL;
  }

  box->custom_draw_proc = nullptr;
  box->custom_draw_data = nullptr;

  f32 hot_rate = 0.01f * ui_g_state->animation_dt;
  bool is_hot    = ui_hot_key_match(key);
  bool is_active = ui_active_key_match(key);
  box->hot_t = (f32)is_hot;

  // box->hot_t    += hot_rate * ((f32)is_hot - box->hot_t);
  // if (Abs(box->hot_t - (f32)is_hot) < 0.01f) {
  //     box->hot_t = (f32)is_hot;
  // }
  // box->active_t += hot_rate * ((f32)is_active - box->active_t);

  ui_g_state->parent_stack.auto_pop();
  ui_g_state->font_stack.auto_pop();
  ui_g_state->pref_width_stack.auto_pop();
  ui_g_state->pref_height_stack.auto_pop();
  ui_g_state->fixed_width_stack.auto_pop();
  ui_g_state->fixed_height_stack.auto_pop();
  ui_g_state->fixed_x_stack.auto_pop();
  ui_g_state->fixed_y_stack.auto_pop();
  ui_g_state->text_padding_stack.auto_pop();
  ui_g_state->text_align_stack.auto_pop();
  ui_g_state->padding_x_stack.auto_pop();
  ui_g_state->padding_y_stack.auto_pop();
  ui_g_state->child_layout_axis_stack.auto_pop();
  ui_g_state->background_color_stack.auto_pop();
  ui_g_state->border_color_stack.auto_pop();
  ui_g_state->text_color_stack.auto_pop();
  ui_g_state->hover_color_stack.auto_pop();

  return box;
}

internal UI_Box *ui_box_create(UI_Box_Flags flags, String8 string) {
  UI_Box *parent = ui_top_parent();

  UI_Key seed = ui_get_seed_key(parent);
  String8 hash_part = ui_hash_part(string);
  UI_Key key = ui_hash_from_string(seed, string);
  UI_Box *box = ui_box_create(flags, key);

  if (flags & UI_BOX_FLAG_TEXT_ELEMENT) {
    box->text = string;
    if (hash_part.data != string.data) {
      box->text.count -= hash_part.count;
    }
  }
  return box;
}

internal UI_Box *ui_box_create_format(UI_Box_Flags flags, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  String8 string = str8_pushfv(ui_build_arena(), fmt, args);
  va_end(args);
  UI_Box *box = ui_box_create(flags, string);
  return box;
}

internal void ui_begin(OS_Handle window_handle, OS_Event_List *events) {
  UI_State *ui_state = ui_g_state;
    
  Vector2 window_dim = os_get_window_dim(window_handle);
  ui_state->window_dimension = {(int)window_dim.x, (int)window_dim.y};

  ui_state->events.reset_count();
  ui_state->mouse_captured = false;
  ui_state->keyboard_captured = false;

  if (ui_state->hot_box_key) {
    ui_set_mouse_captured();
  }

  if (ui_state->active_box_key) {
    ui_set_keyboard_captured();
  } 

  for (OS_Event *evt = events->first; evt; evt = evt->next) {
    UI_Event event = {};
    switch (evt->kind) {
    default:
      continue;
    case OS_EventKind_Press:
      event.kind = UI_EVENT_PRESS;
      event.key = evt->key;
      break;
    case OS_EventKind_Release:
      event.kind = UI_EVENT_RELEASE;
      event.key = evt->key;
      break;
    case OS_EventKind_MouseMove:
      ui_state->mouse_position = {evt->pos.x, evt->pos.y};
      break;
    case OS_EventKind_MouseUp:
      event.kind = UI_EVENT_RELEASE;
      event.key = evt->key;
      event.pos = {evt->pos.x, evt->pos.y};
      break;
    case OS_EventKind_MouseDown:
      event.kind = UI_EVENT_CLICK;
      event.key = evt->key;
      event.pos = {evt->pos.x, evt->pos.y};
      break;
    case OS_EventKind_Text:
      event.kind = UI_EVENT_TEXT;
      event.text = evt->text;
      break;
    }
    ui_state->events.push(event);
  }

  arena_clear(ui_g_state->arena);

  ui_push_pref_width(ui_pixels(window_dim.x));
  ui_push_pref_height(ui_pixels(window_dim.y));
  ui_push_padding_x(4.0f);
  ui_push_padding_y(4.0f);
  ui_push_text_padding(0.0f);
  ui_push_text_align(UI_TEXT_ALIGN_CENTER);
  ui_push_child_layout_axis(AXIS_Y);
  ui_push_background_color(Vector4(1, 1, 1, 1));
  ui_push_border_color(Vector4(0, 0, 0, 1));
  ui_push_text_color(Vector4(0, 0, 0, 1));

  UI_Box *root = ui_box_create(0, str8_lit("##ROOT"));
  ui_push_parent(root);
  ui_g_state->root = root;
}

internal bool rect_contains(UI_Rect rect, Vector2 p) {
  return rect.top    <= p.y &&
    rect.left   <= p.x &&
    rect.right  >  p.x &&
    rect.bottom >  p.y;
}

internal Vector2 ui_mouse() {
  Vector2Int m = ui_g_state->mouse_position;
  return Vector2((f32)m.x, (f32)m.y);
}

internal bool ui_mouse_press() {
  for (int i = 0; i < ui_g_state->events.count; i++) {
    UI_Event event = ui_g_state->events[i];
    if (event.kind == UI_EVENT_CLICK) {
      return true;
    }
  }
  return false;
}

internal bool ui_mouse_release() {
  for (int i = 0; i < ui_g_state->events.count; i++) {
    UI_Event event = ui_g_state->events[i];
    if (event.kind == UI_EVENT_RELEASE &&
      event.key == OS_KEY_LEFTMOUSE) {
      return true;
    }
  }
  return false;
}

internal void ui_end() {
  UI_State *ui_state = ui_g_state;
  ui_state->root = nullptr;

  ui_state->parent_stack.reset();
  ui_state->font_stack.reset();
  ui_state->pref_width_stack.reset();
  ui_state->pref_height_stack.reset();
  ui_state->fixed_width_stack.reset();
  ui_state->fixed_height_stack.reset();
  ui_state->fixed_x_stack.reset();
  ui_state->fixed_y_stack.reset();
  ui_state->text_align_stack.reset();
  ui_state->padding_x_stack.reset();
  ui_state->padding_y_stack.reset();
  ui_state->child_layout_axis_stack.reset();
  ui_state->background_color_stack.reset();
  ui_state->text_color_stack.reset();
  ui_state->hover_color_stack.reset();
  ui_state->border_color_stack.reset();
  ui_state->box_flags_stack.reset();

  if (ui_g_state->draw_bucket) {
    delete ui_g_state->draw_bucket;
    ui_g_state->draw_bucket = nullptr;
  }

  UI_Box *hot_box = ui_find_box(ui_g_state->hot_box_key);
  if (hot_box) {
    bool event_in_bounds = rect_contains(hot_box->rect, ui_mouse());

    if (!event_in_bounds) {
      ui_set_hot_key(0);
    }
  } else {
    ui_set_hot_key(0);
  }

  UI_Box *active_box = ui_find_box(ui_g_state->active_box_key);
  if (active_box) {
    if (!rect_contains(active_box->rect, ui_mouse()) && (ui_mouse_release() || ui_mouse_press())) {
      ui_set_active_key(0);
    }
  } else {
    ui_set_active_key(0);
  }
}

internal UI_Signal ui_signal_from_box(UI_Box *box) {
  UI_State *ui_state = ui_g_state;
    
  UI_Signal signal = {};
  signal.box = box;

  bool event_in_bounds = false;
  if (box->rect.top <= ui_state->mouse_position.y &&
    box->rect.left <= ui_state->mouse_position.x &&
    box->rect.right > ui_state->mouse_position.x &&
    box->rect.bottom > ui_state->mouse_position.y) {
    event_in_bounds = true;
  }

  if (event_in_bounds && (box->flags & UI_BOX_FLAG_CLICKABLE)) {
    signal.flags |= UI_SIGNAL_FLAG_HOVER;
    ui_set_hot_key(box->key);
    ui_set_mouse_captured();
  }

  for (int i = 0; i < ui_state->events.count; i++) {
    UI_Event event = ui_state->events[i];

    switch (event.kind) {
    case UI_EVENT_RELEASE:
    {
      if ((box->flags & UI_BOX_FLAG_CLICKABLE) && ui_active_key_match(box->key) &&
        event.key == OS_KEY_LEFTMOUSE) {
        signal.flags |= UI_SIGNAL_FLAG_RELEASED;
        ui_set_active_key(0);
      }
      break;
    }
    case UI_EVENT_PRESS:
    {
      if ((box->flags & UI_BOX_FLAG_KEYBOARD_CONTROL) && ui_active_key_match(box->key)) {
        signal.flags |= UI_SIGNAL_FLAG_PRESSED;
        signal.key = event.key;
        ui_set_keyboard_captured();
      }
      break;
    }
    case UI_EVENT_CLICK:
    {
      if ((box->flags & UI_BOX_FLAG_CLICKABLE) && ui_hot_key_match(box->key)) {
        ui_set_active_key(box->key);
        signal.flags |= UI_SIGNAL_FLAG_CLICKED;
        ui_set_mouse_captured();
      }
      break;
    }
    case UI_EVENT_TEXT:
    {
      if ((box->flags & UI_BOX_FLAG_KEYBOARD_CONTROL) && ui_active_key_match(box->key)) {
        signal.text = event.text;
        ui_set_keyboard_captured();
      }
      break;
    }
    }
  }

  return signal;
}

internal void ui_layout_calc_fixed(UI_Box *root, Axis axis) {
  if (!(root->flags & UI_BOX_FLAG_FIXED_WIDTH<<axis)) {
    f32 size = 0.0f;
    switch (root->size[axis].kind) {
    case UI_SIZE_PIXELS:
    {
      size  = root->size[axis].value;
      break;
    }
    case UI_SIZE_TEXT:
    {
      size = ((axis == AXIS_X) ? measure_text_width: measure_text_height)(root->text, root->font);
      size += 2.0f * root->text_padding;
      break;
    }
    }
    root->fixed_size[axis] = size;
  }

  for (UI_Box *child = root->first; child; child = child->next) {
    ui_layout_calc_fixed(child, axis);
  }
}

internal void ui_layout_calc_upward_dependent(UI_Box *root, Axis axis) {
  if (root->size[axis].kind == UI_SIZE_PARENT_PCT) {
    for (UI_Box *parent = root->parent; parent; parent = parent->parent) {
      bool found = false;
      if (parent->flags & (UI_BOX_FLAG_FIXED_WIDTH<<axis)) {
        found = true;
      }

      switch (parent->size[axis].kind) {
      case UI_SIZE_PIXELS:
      case UI_SIZE_TEXT:
      case UI_SIZE_PARENT_PCT:
        found = true;
        break;
      }

      if (found) {
        root->fixed_size[axis] = root->size[axis].value * parent->fixed_size[axis];
        break;
      }
    }
  }

  for (UI_Box *child = root->first; child; child = child->next) {
    ui_layout_calc_upward_dependent(child, axis);
  }
}

internal void ui_layout_calc_downward_dependent(UI_Box *root, Axis axis) {
  for (UI_Box *child = root->first; child; child = child->next) {
    ui_layout_calc_downward_dependent(child, axis);
  }

  if (root->size[axis].kind == UI_SIZE_CHILDREN_SUM) {
    f32 value = 0.0f;
    if (root->child_layout_axis == axis) {
      for (UI_Box *child = root->first; child; child = child->next) {
        value += child->fixed_size[axis];
      }
    } else {
      f32 max = 0.0f;
      for (UI_Box *child = root->first; child; child = child->next) {
        if (child->fixed_size[axis] > max) max = child->fixed_size[axis];
      }
      value = max;
    }
    root->fixed_size[axis] = value;
  }
}

internal void ui_layout_place_rects(UI_Box *root, Axis axis) {
  f32 layout_position = 0.0f;
  for (UI_Box *child = root->first; child; child = child->next) {
    if (!(child->flags & UI_BOX_FLAG_FIXED_X<<axis)) {
      child->fixed_position[axis] = layout_position;
      if (root->child_layout_axis == axis) {
        layout_position += child->fixed_size[axis];
      }
    }
    child->rect.tl[axis] = root->rect.tl[axis] + child->fixed_position[axis];
    child->rect.br[axis] = child->rect.tl[axis] + child->fixed_size[axis];
  }

  for (UI_Box *child = root->first; child; child = child->next) {
    ui_layout_place_rects(child, axis);
  }
}

internal void ui_layout_apply() {
  UI_Box *root = ui_g_state->root;
  for (int axis = AXIS_X; axis <= AXIS_Y; axis++) {
    ui_layout_calc_fixed(root, (Axis)axis);
    ui_layout_calc_upward_dependent(root, (Axis)axis);
    ui_layout_calc_downward_dependent(root, (Axis)axis);
    ui_layout_place_rects(root, (Axis)axis);
  }
}

internal UI_Vertex ui_vertex(Vector2 p, Vector2 uv, Vector4 color, float omit_tex) {
  UI_Vertex result;
  result.position = p;
  result.uv = uv;
  result.color = color;
  result.style = {omit_tex, 0, 0, 0};
  return result; 
}

internal UI_Draw_Batch *draw_ui_new_batch() {
  UI_Draw_Batch *batch = new UI_Draw_Batch;
  ui_g_state->draw_bucket->batches.push(batch);
  return batch;
}

internal UI_Draw_Batch *ui_top_draw_batch() {
  UI_Draw_Batch *batch = ui_g_state->draw_bucket->batches.back();
  return batch;
}

internal UI_Rect ui_rect(f32 left, f32 top, f32 right, f32 bottom) {
  UI_Rect result = {};
  result.left = left;
  result.right = right;
  result.top = top;
  result.bottom = bottom;
  return result;
}

internal void draw_ui_rect(UI_Draw_Batch *batch, UI_Rect rect, Vector4 color) {
  UI_Vertex tl = ui_vertex(Vector2(rect.left,  rect.top),  Vector2(0, 0), color, 1.0f);
  UI_Vertex tr = ui_vertex(Vector2(rect.right, rect.top), Vector2(0, 0), color, 1.0f);
  UI_Vertex bl = ui_vertex(Vector2(rect.left,  rect.bottom), Vector2(0, 0), color, 1.0f);
  UI_Vertex br = ui_vertex(Vector2(rect.right, rect.bottom), Vector2(0, 0), color, 1.0f);

  batch->vertices.push(bl);
  batch->vertices.push(br);
  batch->vertices.push(tr);

  batch->vertices.push(bl);
  batch->vertices.push(tr);
  batch->vertices.push(tl);
}

internal void draw_ui_border(UI_Draw_Batch *batch, UI_Rect rect, Vector4 color) {
  UI_Rect left = ui_rect(rect.left, rect.top, rect.left + 1, rect.bottom);
  UI_Rect top = ui_rect(rect.left, rect.top, rect.right, rect.top + 1);
  UI_Rect right = ui_rect(rect.right - 1, rect.top, rect.right, rect.bottom);
  UI_Rect bottom = ui_rect(rect.left, rect.bottom - 1, rect.right, rect.bottom);

  draw_ui_rect(batch, left, color);
  draw_ui_rect(batch, right, color);
  draw_ui_rect(batch, top, color);
  draw_ui_rect(batch, bottom, color);
}

internal void draw_ui_glyph(UI_Draw_Batch *batch, Glyph g, Font *font, Vector2 position, Vector4 color) {
  UI_Rect dst = {};
  dst.left = position.x + g.bl;
  dst.right = dst.left + g.bx;
  dst.top = position.y - g.bt + font->ascend;
  dst.bottom = dst.top + g.by;

  UI_Rect src = {};
  src.left = g.tx;
  src.top = g.ty;
  src.right = src.left + (g.bx / (f32)font->width);
  src.bottom = src.top + (g.by / (f32)font->height);

  UI_Vertex tl = ui_vertex(Vector2(dst.left, dst.top),     Vector2(src.left, src.top), color, 0.0f);
  UI_Vertex tr = ui_vertex(Vector2(dst.right, dst.top),    Vector2(src.right, src.top), color, 0.0f);
  UI_Vertex bl = ui_vertex(Vector2(dst.left, dst.bottom),  Vector2(src.left, src.bottom), color, 0.0f);
  UI_Vertex br = ui_vertex(Vector2(dst.right, dst.bottom), Vector2(src.right, src.bottom), color, 0.0f);

  batch->vertices.push(bl);
  batch->vertices.push(br);
  batch->vertices.push(tr);

  batch->vertices.push(tr);
  batch->vertices.push(tl);
  batch->vertices.push(bl);
}

internal void draw_ui_text(UI_Draw_Batch *batch, String8 string, Font *font, Vector4 color, Vector2 start, Vector2 end) {
  Vector2 cursor = start;
  for (int i = 0; i < string.count; i++) {
    u8 c = string.data[i];
    if (c == '\n') {
      cursor.x = start.x;
      cursor.y += font->glyph_height;
      continue;
    }

    Glyph g = font->glyphs[c];

    draw_ui_glyph(batch, g, font, cursor, color);

    cursor.x += g.ax;
  }
}

internal void draw_ui_box(UI_Box *box) {
  UI_Draw_Bucket *bucket = ui_g_state->draw_bucket;

  UI_Draw_Batch *batch = nullptr;
  if (bucket->batches.count) {
    batch = bucket->batches.back();
  } else {
    batch = draw_ui_new_batch();
    batch->font = box->font;
  }

  if (batch->font != box->font) {
    batch = draw_ui_new_batch();
    batch->font = box->font;
  }

  if (box->flags & UI_BOX_FLAG_DRAW_BACKGROUND) {
    draw_ui_rect(batch, box->rect, box->background_color);
  }

  if (box->flags & UI_BOX_FLAG_DRAW_HOT_EFFECTS) {
    Vector4 hot_color = box->hot_t * Vector4(1, 1, 1, 0.3f);
    draw_ui_rect(batch, box->rect, hot_color);
  }


  if (box->flags & UI_BOX_FLAG_DRAW_BORDER) {
    draw_ui_border(batch, box->rect, box->border_color);
  }

  Vector2 text_position = ui_box_text_position(box);
  if (box->font) draw_ui_text(batch, box->text, box->font, box->text_color, text_position, box->rect.br);


  if (box->custom_draw_proc) {
    box->custom_draw_proc(box, box->custom_draw_data);
  }


  for (UI_Box *child = box->first; child; child = child->next) {
    draw_ui_box(child);
  }
}

internal void draw_ui_layout() {
  R_D3D11_State *d3d11_state = r_d3d11_state();

  ui_g_state->draw_bucket = new UI_Draw_Bucket();

  UI_Box *root = ui_g_state->root;
  draw_ui_box(root);

  UI_Draw_Bucket *bucket = ui_g_state->draw_bucket;

  set_shader(shader_rect);
  d3d11_state->device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  bind_uniform(current_shader, str8_lit("Constants"));

  Vector2Int window_dim = ui_g_state->window_dimension;
  Matrix4 projection = ortho_rh_zo(0.0f, (f32)window_dim.x, (f32)window_dim.y, 0.0f);

  set_constant(str8_lit("xform"), projection);
  apply_constants();
  
  set_sampler(str8_lit("diffuse_sampler"), SAMPLER_STATE_POINT);

  set_depth_state(DEPTH_STATE_DISABLE);
  set_blend_state(BLEND_STATE_ALPHA);
  set_rasterizer_state(RASTERIZER_STATE_TEXT);

  for (int i = 0; i < bucket->batches.count; i++) {
    UI_Draw_Batch *batch = bucket->batches[i];
    if (batch->vertices.count == 0) continue;
    
    set_texture(str8_lit("diffuse_texture"), batch->font->texture);

    ID3D11Buffer *vertex_buffer = nullptr;
    {
      D3D11_BUFFER_DESC desc = {};
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.ByteWidth = (UINT)batch->vertices.count * sizeof(UI_Vertex);
      desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
      D3D11_SUBRESOURCE_DATA data = {};
      data.pSysMem = batch->vertices.data;
      d3d11_state->device->CreateBuffer(&desc, &data, &vertex_buffer);
    }

    UINT stride = sizeof(UI_Vertex), offset = 0;
    d3d11_state->device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
    d3d11_state->device_context->Draw((UINT)batch->vertices.count, 0);
        
    vertex_buffer->Release();
    batch->vertices.clear();

    delete batch;
  }
  bucket->batches.clear();

  delete ui_g_state->draw_bucket;
  ui_g_state->draw_bucket = nullptr;
}
