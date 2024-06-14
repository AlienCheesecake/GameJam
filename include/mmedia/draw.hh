#pragma once
#include "Animator.hh"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <concepts>


template <typename T>
  requires requires(T t, sf::RenderTarget &rt) { t.draw(rt); }
sf::RenderTarget& operator<<(sf::RenderTarget &rt, T &&t) {
  std::forward<T>(t).draw(rt);
  return rt;
}

template <typename T>
requires requires (T t, sf::RenderTarget &rt) {
  rt.draw(t);
}
sf::RenderTarget& operator<<(sf::RenderTarget &rt, T &&t) {
  rt.draw(std::forward<T>(t));
  return rt;
}

template <typename T>
sf::RenderTarget &operator<<(sf::RenderTarget &rt, T &&t) {
  ::draw(rt, std::forward<T>(t));
  return rt;
}
