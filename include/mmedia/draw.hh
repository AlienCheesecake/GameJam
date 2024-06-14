#pragma once
#include "Animator.hh"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <concepts>

template <typename T>
  requires requires(T t, sf::RenderTarget &rt, sf::RenderStates rs) {
    t.draw(rt, rs);
  }
void draw(sf::RenderTarget &rt, T &&t,
          sf::RenderStates rs = sf::RenderStates::Default) {
  std::forward<T>(t).draw(rt, rs);
}

template <typename T>
  requires requires(T t, sf::RenderTarget &rt, sf::RenderStates rs) { 
    rt.draw(t, rs);
  }
void draw(sf::RenderTarget &rt, T &&t, sf::RenderStates rs = sf::RenderStates::Default) {
  rt.draw(std::forward<T>(t), rs);
}

template<typename T>
requires requires (T t, sf::RenderTarget &rt) {
  t.draw(rt);
}
void draw(sf::RenderTarget &rt, T &&t, sf::RenderStates rs = sf::RenderStates::Default) {
  t.draw(rt);
}