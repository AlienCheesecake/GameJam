#include "MusicPlayer.hh"
#include <SFML/Audio/Music.hpp>
#include <stdexcept>

using namespace mmed;

MusicPlayer &MusicPlayer::getInstance() {
  static MusicPlayer mp{};
  return mp;
}

void MusicPlayer::play(const std::string_view path) {
  if (path.empty())
    return;
  std::string tmp{path};
  if (!openFromFile(tmp))
    throw std::runtime_error("File \"" + tmp + "\" not found");
  std::swap(tmp, path_);
  setLoop(true);
  if (!paused())
    play();
}

void MusicPlayer::set_pause(bool is_pause) {
  if (is_pause)
    pause();
  else
    play();
  is_paused = is_pause;
}

bool MusicPlayer::paused() const noexcept {
  return is_paused;
}

const std::string_view MusicPlayer::path() const noexcept { return path_; }

MusicPauseField::MusicPauseField() {
  auto &&mp = MusicPlayer::getInstance();
  was_paused = mp.paused();
  mp.set_pause(true);
}
MusicPauseField::~MusicPauseField() {
  auto &&mp = MusicPlayer::getInstance();
  mp.set_pause(was_paused);
}

MusicField::MusicField(const std::string_view path)
    : path_(path) {
  auto &&mp = MusicPlayer::getInstance();
  prev_path_ = mp.path();
  was_paused = mp.paused();
  mp.set_pause(false);
  mp.play(path_);
}

MusicField::~MusicField() {
  auto &&mp = MusicPlayer::getInstance();
  mp.set_pause(was_paused);
  mp.play(prev_path_);
}