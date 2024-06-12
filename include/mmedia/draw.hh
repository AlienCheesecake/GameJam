#pragma once
#include "Animator.hh"
#include <SFML/Graphics.hpp>

template <typename T> void draw(sf::RenderTarget &trgt, T &&drawable) {
  trgt.draw(std::forward<T>(drawable));
}

template <> void draw<mmed::Animator &>(sf::RenderTarget &, mmed::Animator &);

template <>
void draw<mmed::CharacterAnimation &>(sf::RenderTarget &rd,
                                mmed::CharacterAnimation &char_anim);
