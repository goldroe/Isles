#ifndef AUDIO_H
#define AUDIO_H

struct Audio_Engine {
  FMOD::Studio::System *studio_system = nullptr;
  FMOD::System *system = nullptr;
  FMOD::ChannelGroup *master_group;

  std::unordered_map<std::string, FMOD::Sound*> sound_map;
  std::unordered_map<std::string, FMOD::Channel*> channel_map;

  Vector3 origin;

  void init();

  FMOD::Sound *load_sound(std::string name);
  void play_sound(std::string name, Vector3 position);
  void update();
};

#endif // AUDIO_H
