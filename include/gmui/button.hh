#pragma once
#include "mmedia/Animator.hh"
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <functional>

namespace mmed {
namespace gmui {
struct Button : sf::Transformable {
  enum e_states { D, P, R, H };

  struct changed final {
  private:
    bool check = false;
    e_states val = D;

  public:
    changed &operator=(e_states &&x);
    operator bool();
    e_states get();
  };

private:
  changed cur_state;

  bool contains(const sf::Vector2f &p);

public:
  std::function<void()> prs_, rls_;
  mmed::CharacterAnimation anim_;
  sf::RectangleShape rect_;
  Button(std::function<void()> prs, std::function<void()> rls,
         const mmed::CharacterAnimation &anim, const sf::RectangleShape &rect);
  void update(sf::Time dt, const sf::Vector2f &pos);
  bool handleEvent(const sf::Event &ev, const sf::Vector2f &pos);
};
} // namespace gmui
} // namespace mmed

void draw(sf::RenderTarget &target, const mmed::gmui::Button &btn, sf::RenderStates states=sf::RenderStates::Default);