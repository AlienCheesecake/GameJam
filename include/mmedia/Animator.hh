#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderStates.hpp>

namespace mmed {
struct Animation {
  std::string name, texture_path;
  uint64_t total_frame, columns;
  sf::Vector2i frame_size;
  sf::Time duration_;
  sf::Color mask_color;
  sf::IntRect get_box(uint64_t cur_frame) const;
  bool loop;
  static Animation one_frame_anim(const std::string &name,
                                  const std::string &path,
                                  sf::Color mask_clr = {0, 255, 0, 255});
};

class AnimationManager final {
  std::map<std::string, Animation> m_Animations;

public:
  void loadFile(const std::string &file_name = "media/animations.json");
  Animation &getAnimation(const std::string &id_name);
};

class Animator {
  Animation anim_;
  bool end_anim = true;
  sf::Time time;

public:
  sf::Sprite sp_;
  Animator(const sf::Sprite &sp, const Animation &anim = {});
  void switchAnimation(const Animation &anim);
  Animation getAnimation() const noexcept;
  void update(sf::Time dt);
  void restart();
  void stop();
  bool finished() const;
};

struct CharacterAnimation : private Animator {
  std::vector<Animation> anims_;
  CharacterAnimation(const std::vector<Animation> &anims, const sf::Sprite &sp);
  void select_anim(const std::string_view name);
  using Animator::restart;
  using Animator::sp_;
  using Animator::stop;
  using Animator::update;
  using Animator::finished;
  using Animator::getAnimation;
};

mmed::Animation one_frame_anim();
} // namespace mmed

void draw(sf::RenderTarget &rt, const mmed::Animator &anim, sf::RenderStates = sf::RenderStates::Default);
void draw(sf::RenderTarget &rt, const mmed::CharacterAnimation &ca, sf::RenderStates = sf::RenderStates::Default);