#pragma once
#include <SFML/Audio/Music.hpp>
#include <string>

namespace mmed {
class MusicPlayer : private sf::Music {
  MusicPlayer() = default;
  std::string path_;
  bool is_paused = false;

  using sf::Music::play;
  using sf::Music::pause;
public:
  static MusicPlayer &getInstance();
  using sf::Music::stop;
  using sf::Music::setVolume;
  using sf::Music::setPitch;
  using sf::Music::setLoop;
  using sf::Music::getStatus;
  
  void play(const std::string_view path);
  void set_pause(bool is_pause);
  bool paused() const noexcept;
  const std::string_view path() const noexcept;
  MusicPlayer(const MusicPlayer &) = delete;
  MusicPlayer& operator=(const MusicPlayer &) = delete;
};

struct MusicPauseField final {
  bool was_paused;
  MusicPauseField();
  ~MusicPauseField();
};

struct MusicField final {
  bool was_paused;
  std::string path_, prev_path_;
  MusicField(const std::string_view path);
  ~MusicField();
};

} // namespace mmedia