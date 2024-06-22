#pragma once
#include "Node2D.hh"
#include <SFML/Graphics.hpp>

namespace gm {
struct ListNode : sf::Transformable {
  sf::Vector2f transform_coords(const sf::Vector2f &pos);
  std::vector<gm::Node2D> cld;
  void update(sf::Time dt, const sf::Vector2f &pos);
  bool handleEvent(const sf::Event &event, const sf::Vector2f &pos);
};
}

void draw(sf::RenderTarget &target, const gm::ListNode &t,
          sf::RenderStates states = sf::RenderStates::Default);