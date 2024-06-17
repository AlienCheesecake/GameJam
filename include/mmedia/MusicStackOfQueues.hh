#pragma once
#include "MusicPlayer.hh"
#include <SFML/Audio.hpp>
#include <queue>
#include <stack>

namespace mmed {
struct MusicDef {
  std::string path;
  bool is_loop;
};

struct MusicQueue {
  template <typename Iter>
    requires std::same_as<MusicDef,
                          typename std::iterator_traits<Iter>::value_type>
  MusicQueue(Iter fst, Iter lst) : queue(fst, lst) {
    play_front();
  }
  MusicQueue(std::initializer_list<MusicDef> il);
  void update();

  void play_front();

private:
  std::queue<MusicDef> queue;
  mmed::MusicPlayer &plr = mmed::MusicPlayer::getInstance();
};

struct MusicStackOfQueues {
  void update();
  void push(MusicQueue &&q);
  void push(const MusicQueue &q);
  template <typename... Args> void emplace(Args &&...args) {
    stack.emplace(std::forward<Args>(args)...);
    auto &&plr = MusicPlayer::getInstance();
    stack.top().play_front();
  }
  void pop();

private:
  std::stack<MusicQueue> stack;
};

struct NMusicField {
  MusicStackOfQueues &sck;
  NMusicField(MusicStackOfQueues &stack, std::initializer_list<MusicDef> il);
  ~NMusicField();
};
} // namespace mmed