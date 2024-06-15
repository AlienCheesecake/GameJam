#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <concepts>

template<typename T>
concept t_draw_with_states = requires (T t, sf::RenderTarget &rt, sf::RenderStates rs) {
  t.draw(rt, rs);
};

template<typename T>
concept t_draw_without_states = requires (T t, sf::RenderTarget &rt) {
  t.draw(rt);
};

template<typename T>
concept rt_draw = requires (T t, sf::RenderTarget &rt) {
  rt.draw(t);
};

template <t_draw_with_states T>
void draw(sf::RenderTarget &rt, T &&t,
          sf::RenderStates rs = sf::RenderStates::Default) {
  std::forward<T>(t).draw(rt, rs);
}

template<typename T>
requires (t_draw_without_states<T> && !t_draw_with_states<T>)
void draw(sf::RenderTarget &rt, T &&t, sf::RenderStates rs = sf::RenderStates::Default) {
  t.draw(rt);
}

template<typename T>
requires (!t_draw_with_states<T> && !t_draw_without_states<T> && rt_draw<T>)
void draw(sf::RenderTarget &rt, T &&t, sf::RenderStates rs = sf::RenderStates::Default) {
  rt.draw(t, rs);
}