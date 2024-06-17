#include "MusicStackOfQueues.hh"
using namespace mmed;
MusicQueue::MusicQueue(std::initializer_list<MusicDef> il) : queue(il) {
  play_front();
}

void MusicQueue::update() {
  if (plr.getStatus() == sf::Music::Stopped && !queue.empty()) {
    queue.pop();
    play_front();
  }
}

void MusicQueue::play_front() {
  if (queue.empty())
    return;
  auto &&tmp = queue.front();
  plr.play(tmp.path);
  plr.setLoop(tmp.is_loop);
}

void MusicStackOfQueues::update() {
  if (!stack.empty())
    stack.top().update();
}

void MusicStackOfQueues::push(MusicQueue &&q) {
  stack.push(q);
  auto &&plr = MusicPlayer::getInstance();
  stack.top().play_front();
}

void MusicStackOfQueues::push(const MusicQueue &q) {
  stack.push(q);
  auto &&plr = MusicPlayer::getInstance();
  stack.top().play_front();
}

void MusicStackOfQueues::pop() {
  stack.pop();
  if (!stack.empty())
    stack.top().play_front();
}

NMusicField::NMusicField(MusicStackOfQueues &stack,
                         std::initializer_list<MusicDef> il)
    : sck(stack) {
  stack.emplace(il);
}

NMusicField::~NMusicField() { sck.pop(); }