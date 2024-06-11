#pragma once
#include <SFML/Graphics.hpp>

struct Animation {
  std::string name, texture_path;
  uint64_t total_frame, columns;
  sf::Vector2i frame_size;
  sf::Time duration_;
  sf::Color mask_color;
  sf::IntRect get_box(uint64_t cur_frame) const;
  bool loop;
};

class AnimationManager final {
  std::map<std::string, Animation> m_Animations;
  AnimationManager() = default;

public:
  AnimationManager(AnimationManager &) = delete;
  void operator=(const AnimationManager &) = delete;

  static AnimationManager &getInstance();
  void loadFile(std::string_view file_name = "media/animations.json");
  static Animation &getAnimation(const std::string &id_name);
};

class Animator {
  Animation anim_;
  bool end_anim = true;
  sf::Time time;

public:
  sf::Sprite &sp_;
  Animator(sf::Sprite &sp, Animation anim = {});
  void switchAnimation(Animation anim);
  void update(sf::Time dt);
  void restart();
  void stop();
};
