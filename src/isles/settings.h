#if !defined(SETTINGS_H)
#define SETTINGS_H

struct Tweak_Settings {
  String8 file_name;
  u64 file_last_write_time = 0;
  std::unordered_map<u64,Value> values;
};

internal Value get_setting_value(String8 name);
internal void init_tweak_settings();

#endif // SETTINGS_H
