#pragma once
#include "Animator.hh"
#include <SFML/Graphics.hpp>

template <typename T> void draw(sf::RenderTarget &trgt, T &&drawable) {
  trgt.draw(std::forward<T>(drawable));
}

template <> void draw<Animator &>(sf::RenderTarget &, Animator &);

template <>
void draw<CharacterAnimation &>(sf::RenderTarget &rd,
                                CharacterAnimation &char_anim);
