global Audio_Engine *audio_engine;

#define ERRCHECK(E) if((E) != FMOD_OK) DebugTrap()

void Audio_Engine::init() {
  int sample_rate = 44100;
  float distance_factor = 1.0f;
  int max_channels = 1024;

  ERRCHECK(FMOD::Studio::System::create(&studio_system));
  ERRCHECK(studio_system->getCoreSystem(&system));
  ERRCHECK(system->setSoftwareFormat(sample_rate, FMOD_SPEAKERMODE_STEREO, 0));
  ERRCHECK(system->set3DSettings(1.0f, distance_factor, 0.5f));
  ERRCHECK(studio_system->initialize(max_channels, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0));
  // ERRCHECK(system->getMasterChannelGroup(&channel_group));
}

FMOD::Sound *Audio_Engine::load_sound(std::string name) {
  std::string file_name = std::string("data/audio/") + name;
  auto it = sound_map.find(name);
  if (it != sound_map.end()) {
    return it->second;
  }

  FMOD_MODE mode = 0;
  // mode |= FMOD_3D;
  mode |= FMOD_LOOP_OFF;
  mode |= FMOD_CREATESTREAM;

  FMOD::Sound *sound = nullptr;
  const char *name_cstr = file_name.c_str();
  FMOD_RESULT error = system->createSound(name_cstr, mode, nullptr, &sound);
  if (error) printf("%s:%s", name_cstr, FMOD_ErrorString(error));
  sound_map.insert({name, sound});
  return sound;
}

void Audio_Engine::play_sound(std::string name, Vector3 position) {
  FMOD::Sound *sound = nullptr;

  auto found = sound_map.find(name);
  if (found == sound_map.end()) {
    sound = load_sound(name);
  } else {
    sound = found->second;
  }

  FMOD::Channel *channel = nullptr;
  ERRCHECK(system->playSound(sound, nullptr, true, &channel));

  if (channel) {
    FMOD_MODE mode;
    sound->getMode(&mode);
    if (mode & FMOD_3D) {
      FMOD_VECTOR v = {position.x, position.y, position.z};
      ERRCHECK(channel->set3DAttributes(&v, nullptr));
    }
    ERRCHECK(channel->setVolume(0.1f));
    ERRCHECK(channel->setPaused(false));
  }
}

void Audio_Engine::update() {
  std::vector<std::unordered_map<std::string, FMOD::Channel*>::iterator> stopped_channels;
  for (auto it = channel_map.begin(); it != channel_map.end(); it++) {
    bool playing = false;
    it->second->isPlaying(&playing);
    if (!playing) {
      stopped_channels.push_back(it); 
    }
  }

  for (auto& it : stopped_channels) {
    channel_map.erase(it);
  }

  ERRCHECK(studio_system->update());
}

#undef ERRCHECK
