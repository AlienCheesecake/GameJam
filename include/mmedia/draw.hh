#pragma once
#include "Animator.hh"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <concepts>
#include <cstddef>
#include <iterator>

template <typename T>
  requires requires(T t, sf::RenderTarget &rt) { t.draw(rt); }
sf::RenderTarget &operator<<(sf::RenderTarget &rt, T &&t) {
  std::forward<T>(t).draw(rt);
  return rt;
}

template<typename T>
requires requires (T t, sf::RenderTarget &rt) {t->draw(rt);} 
sf::RenderTarget &operator<<(sf::RenderTarget &rt, T &&t) {
   std::forward<T>(t)->draw(rt);
   return rt;
}

template <typename T>
  requires requires(T t, sf::RenderTarget &rt) { rt.draw(t); }
sf::RenderTarget &operator<<(sf::RenderTarget &rt, T &&t) {
  rt.draw(std::forward<T>(t));
  return rt;
}

#if 0
template <typename T>
  requires requires(T t, sf::RenderTarget &rt) { rt.draw(*t); }
sf::RenderTarget &operator<<(sf::RenderTarget &rt, T &&t) {
  rt.draw(*std::forward<T>(t));
  return rt;
}
#endif

template <typename T>
sf::RenderTarget &operator<<(sf::RenderTarget &rt, T &&t) {
  ::draw(rt, std::forward<T>(t));
  return rt;
}


namespace mmed{
template <typename T> struct RenderIterator {
  using iterator_category = std::output_iterator_tag;
  using value_type = void;
  using difference_type = ptrdiff_t;
  using pointer = void;
  using reference = void;

  sf::RenderTarget *rt;
  RenderIterator(sf::RenderTarget *rtt = nullptr) : rt(rtt) {}
  RenderIterator(sf::RenderTarget &rtt) : rt(&rtt) {}
  RenderIterator(const RenderIterator &ri) : rt(ri.rt) {}
  RenderIterator &operator=(const T &value) {
    *rt << value;
    return *this;
  }
  RenderIterator &operator=(const RenderIterator &ri) {
    rt = ri.rt;
    return *this;
  }

  RenderIterator &operator++() { return *this; }
  RenderIterator &operator++(int) { return *this; }
  RenderIterator &operator*() { return *this; }
};
}