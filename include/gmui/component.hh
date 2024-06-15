#pragma once
#include <SFML/Graphics.hpp>

namespace mmed {
namespace gmui {
struct Component : sf::Transformable, sf::Drawable {
  using ptr = std::shared_ptr<Component>;
  virtual bool handleEvent(const sf::Event &, const sf::Vector2f &pos) = 0;
  virtual bool update(sf::Time, const sf::Vector2f &pos) = 0;
  virtual void draw(sf::RenderTarget &target,
                    sf::RenderStates states) const = 0;
  virtual ~Component() = default;
};
} // namespace gui
} // namespace mmed