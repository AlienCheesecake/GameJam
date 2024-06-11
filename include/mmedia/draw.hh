#pragma once
#include <SFML/Graphics.hpp>
#include "Animator.hh"

template <typename T> void draw(sf::RenderTarget &trgt, T &&drawable) {
  trgt.draw(std::forward<T>(drawable));
}

template<> void draw<Animator &>(sf::RenderTarget &, Animator &);
