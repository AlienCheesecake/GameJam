#pragma once
#include "button.hh"
#include "mmedia/Animator.hh"
#include "mmedia/draw.hh"
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Transformable.hpp>

struct FollowAnim final {
  mmed::Animator anim;
  void update(sf::Time dt, const sf::Vector2f &pos);
  void draw(sf::RenderTarget &rt, sf::RenderStates states) const;
};

template <typename T> struct BoolDrawable {
  T t;
  bool check = true;
  void update(sf::Time dt, const sf::Vector2f &pos) {
    if (check)
      t.update(dt, pos);
  }
  void update(sf::Time dt) {
    if (check)
      t.update(dt);
  }
  void draw(sf::RenderTarget &rt, sf::RenderStates states) const {
    if (check)
      ::draw(rt, t, states);
  }
  void handleEvent(const sf::Event &event, const sf::Vector2f &pos) {
    if (check)
      t.handleEvent(event, pos);
  }
  void handleEvent(const sf::Event &event) {
    if (check)
      t.handleEvent(event);
  }
};

namespace mmed::gmui {
class DD : public sf::Transformable {
public:
  Button btn_;
  DD(
      const mmed::CharacterAnimation &btn_anim, const sf::RectangleShape &rs,
      BoolDrawable<CharacterAnimation> &outsorce, std::string_view anim_name,
      std::function<void()> prs = [] {}, std::function<void()> rls = [] {});
  sf::Vector2f inner_pos(const sf::Vector2f &p);
  void update(sf::Time dt, const sf::Vector2f &pos);
  bool follow_update(sf::Time dt, const sf::Vector2f &pos);
  bool handleEvent(const sf::Event &ev, const sf::Vector2f &pos);
  void follow_draw(sf::RenderTarget &target, sf::RenderStates states) const;
};
} // namespace mmed::gmui

void draw(sf::RenderTarget &target, const mmed::gmui::DD &t, sf::RenderStates states = sf::RenderStates::Default);