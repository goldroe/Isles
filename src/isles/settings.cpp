
global Tweak_Settings *tweak_settings;

internal Value get_tweak_setting(String8 name) {
  u64 hash = djb2_hash_string(name);
  auto it = tweak_settings->values.find(hash);
  Assert(it != tweak_settings->values.end());
  Value value = it->second;
  return value;
}

internal int get_tweak_setting_int(String8 name) {
  Value value = get_tweak_setting(name);
  Assert(value.kind == VALUE_INT);
  return (int)value.int_val;
}

internal String8 get_tweak_setting_string(String8 name) {
  Value value = get_tweak_setting(name);
  Assert(value.kind == VALUE_STRING);
  return value.string;
}

internal void insert_tweak_setting(String8 name, Value value) {
  u64 hash = djb2_hash_string(name);
  tweak_settings->values.insert({hash, value});
}

internal void init_tweak_settings() {
  tweak_settings = new Tweak_Settings();

  insert_tweak_setting(str8_lit("WindoWidth"), value_int(1600));
  insert_tweak_setting(str8_lit("WindoWidth"), value_int(900));

  OS_Handle file_handle = os_open_file(str8_lit("data/All.settings"), OS_AccessFlag_Read);
  String8 file_contents = str8_zero();
  if (os_valid_handle(file_handle)) {
    file_contents = os_read_file_string(file_handle);
    os_close_handle(file_handle);
  }

  Lexer *lexer = init_lexer(file_contents);

  for (;;) {
    Token name = next_token(lexer);
    if (name.kind == TOKEN_EOF) break;

    if (name.kind == TOKEN_NAME) {
      Token val_tok = next_token(lexer);
      Value value = {};
      switch (val_tok.kind) {
      case TOKEN_INT:
        value.kind = VALUE_INT;
        value.int_val = val_tok.int_val;
        break;
      case TOKEN_FLOAT:
        value.kind = VALUE_FLOAT;
        value.float_val = val_tok.float_val;
        break;
      case TOKEN_STRING: {
        value.kind = VALUE_STRING;
        value.string = val_tok.string;
        break;
      }
      }

      if (value.kind != VALUE_NIL) {
        insert_tweak_setting(name.name, value);
      }
    }
  }
}
 
