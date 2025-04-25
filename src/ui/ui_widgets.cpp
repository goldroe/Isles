internal void ui_text(String8 text) {
  UI_Box *box = ui_box_create(UI_BOX_FLAG_TEXT_ELEMENT, 0);
  box->text = text;
}

internal void ui_textf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  String8 text = str8_pushfv(ui_build_arena(), fmt, args); 
  va_end(args);
  ui_text(text);
}

internal UI_Signal ui_button(String8 string) {
  UI_Box *box = ui_box_create(UI_BOX_FLAG_CLICKABLE | UI_BOX_FLAG_TEXT_ELEMENT, string);

  UI_Signal sig = ui_signal_from_box(box);
  if (sig.flags & UI_SIGNAL_FLAG_RELEASED) {
    ui_set_active_key(0);
  }
  return sig;
}

internal UI_Signal ui_buttonf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  String8 string = str8_pushfv(ui_build_arena(), fmt, args); 
  va_end(args);
  UI_Signal sig = ui_button(string);
  return sig;
}

struct UI_Draw_Text_Edit {
  u64 txt_pos;
  u64 txt_count;
};

internal UI_BOX_DRAW(ui_draw_text_edit) {
  UI_Draw_Text_Edit *text_edit = (UI_Draw_Text_Edit *)user_data;

  UI_Draw_Batch *batch = ui_top_draw_batch();

  f32 cursor_width = 2.0f;
  Vector2 cursor = ui_box_text_position(box);
  for (int i = 0; i < text_edit->txt_pos; i++) {
    u8 c = box->text.data[i];
    cursor.x += box->font->glyphs[c].ax;
  }

  UI_Rect cursor_rect = {};
  cursor_rect.tl = cursor;
  cursor_rect.br = cursor_rect.tl + Vector2(cursor_width, box->font->glyph_height);

  Vector4 cursor_color = box->text_color;

  if (ui_active_key_match(box->key)) {
    draw_ui_rect(batch, cursor_rect, cursor_color);
  }
}

internal UI_Signal ui_text_edit(String8 name, void *buffer, int max_buffer_capacity, int *buffer_pos, int *buffer_count) {
  // ui_set_next_text_align(UI_TEXT_ALIGN_LEFT);
  UI_Box *box = ui_box_create(UI_BOX_FLAG_TEXT_ELEMENT |
    UI_BOX_FLAG_CLICKABLE |
    UI_BOX_FLAG_KEYBOARD_CONTROL |
    UI_BOX_FLAG_DRAW_BORDER,
    name);

  UI_Signal sig = ui_signal_from_box(box);

  int pos = *buffer_pos, count = *buffer_count;

  if (sig.text.data && (*sig.text.data != '\n' && *sig.text.data != '\r' && *sig.text.data != '\b')) {
    u64 text_count = Min(sig.text.count, max_buffer_capacity - count);
    if (pos == text_count) {
      MemoryCopy((u8 *)buffer + pos, sig.text.data, text_count);
    } else {
      MemoryCopy((u8*)buffer + pos + text_count, (u8*)buffer + pos, text_count);
      MemoryCopy((u8*)buffer + pos, sig.text.data, text_count);
    }
    pos += (int)text_count;
    count += (int)text_count;
  }

  if (sig.flags & UI_SIGNAL_FLAG_PRESSED) {
    switch (sig.key) {
    case OS_KEY_ENTER:
      ui_set_active_key(0);
      break;
    case OS_KEY_BACKSPACE:
      if (pos > 0) {
        MemoryCopy((u8*)buffer + pos - 1, (u8*)buffer + pos, count - pos);
        pos -= 1;
        count -= 1;
      }
      break;
    case OS_KEY_LEFT:
      if (pos > 0) {
        pos -= 1;
      }
      break;
    case OS_KEY_RIGHT:
      if (pos < count) {
        pos += 1;
      }
      break;
    }
  }

  if (sig.flags & UI_SIGNAL_FLAG_CLICKED) {
    Vector2 mouse = ui_mouse();
    Vector2 start = ui_box_text_position(box);
    Vector2 cursor = start;
    f32 end = start.x + measure_text_width(str8((u8 *)buffer, count), box->font);

    for (int i = 0; i < count; i++) {
      u8 c = ((u8 *)buffer)[i];
      Glyph g = box->font->glyphs[c];

      if (mouse.x >= cursor.x && mouse.x <= cursor.x + g.ax) {
        pos = i;
        break;
      }
      cursor.x += g.ax;
    }

    if (mouse.x < start.x) {
      pos = 0;
    }
    if (mouse.x > end) {
      pos = count;
    }
  }

  ((u8 *)buffer)[count] = 0;
  box->text = str8((u8 *)buffer, count);

  *buffer_count = count;
  *buffer_pos = pos;

  UI_Draw_Text_Edit *draw_data = push_array(ui_build_arena(), UI_Draw_Text_Edit, 1);
  draw_data->txt_pos = pos;
  draw_data->txt_count = count;
  ui_box_equip_draw_proc(box, ui_draw_text_edit, draw_data);
  return sig;
}

internal UI_Line_Edit *ui_line_edit_create(String8 name) {
  UI_Line_Edit *result = new UI_Line_Edit();
  result->name = name;
  result->box = nullptr;
  result->sig = {};
  result->buffer = (u8 *)calloc(1, 256);
  result->buffer_len = 0;
  result->buffer_pos = 0;
  result->buffer_capacity = 256;
  return result;
}

internal UI_Signal ui_line_edit(UI_Line_Edit *edit, String8 string) {
  UI_Signal signal = ui_text_edit(string, edit->buffer, edit->buffer_capacity, &edit->buffer_pos, &edit->buffer_len);
  return signal;
}

internal UI_Signal ui_line_editf(UI_Line_Edit *edit, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  String8 string = str8_pushfv(ui_build_arena(), fmt, args); 
  va_end(args);
  UI_Signal signal = ui_line_edit(edit, string);
  return signal;
}

struct UI_Draw_Slider {
  f32 min;
  f32 max;
  f32 value;
  Vector4 slider_color;
};

internal UI_BOX_DRAW(ui_draw_slider) {
  UI_Draw_Slider *draw_data = (UI_Draw_Slider *)user_data;

  UI_Draw_Batch *batch = ui_top_draw_batch();

  f32 r = (draw_data->value - draw_data->min) / (draw_data->max - draw_data->min);

  f32 w = 8.0f; 

  f32 x = box->rect.left + (box->rect.right - box->rect.left - w) * r;

  UI_Rect slider_rect = ui_rect(x, box->rect.top, x + w, box->rect.bottom);

  draw_ui_rect(batch, slider_rect, draw_data->slider_color);
}

internal void ui_slider(f32 *value, f32 min, f32 max, Vector4 slider_color, String8 name) {
  UI_Box *box = ui_box_create(UI_BOX_FLAG_TEXT_ELEMENT | UI_BOX_FLAG_DRAW_BACKGROUND | UI_BOX_FLAG_CLICKABLE, name);
  UI_Signal sig = ui_signal_from_box(box);

  if (ui_active_key_match(box->key)) {
    f32 w = box->rect.right - box->rect.left;
    f32 p = ui_mouse().x - box->rect.left;
    f32 v = p / w;
    *value = min + (max - min) * v;

    if (sig.flags & UI_SIGNAL_FLAG_RELEASED) {
      ui_set_active_key(0);
    }
  }

  *value = Clamp(*value, min, max);

  UI_Draw_Slider *draw_data = push_array(ui_build_arena(), UI_Draw_Slider, 1);
  draw_data->min = min;
  draw_data->max = max;
  draw_data->value = *value;
  draw_data->slider_color = slider_color;
  ui_box_equip_draw_proc(box, ui_draw_slider, draw_data);
}
