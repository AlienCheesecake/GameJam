#pragma once
#include "mmedia/draw.hh"
#include <SFML/Graphics.hpp>

namespace gm {
template <typename T>
concept game_component = requires(T t, sf::Event ev, const sf::Vector2f &pos,
                                  sf::RenderTarget &trgt, sf::Time dt) {
  ::draw(trgt, t);
  t.update(dt, pos);
  { t.handleEvent(ev, pos) } -> std::same_as<bool>;
};

class Node2D {
  struct INode2D {
    virtual std::unique_ptr<INode2D> copy() const = 0;
    virtual void update(sf::Time dt, const sf::Vector2f &pos) = 0;
    virtual bool handleEvent(const sf::Event &event,
                             const sf::Vector2f &pos) = 0;
    virtual void draw(sf::RenderTarget &target,
                      sf::RenderStates states) const = 0;
    virtual ~INode2D() = default;
  };
  std::unique_ptr<INode2D> self;

  template <typename T> struct ObjNode2D : INode2D {
    T data;
    ObjNode2D(T t) : data(std::move(t)) {}
    std::unique_ptr<INode2D> copy() const override {
      return std::make_unique<ObjNode2D>(*this);
    }

    void update(sf::Time dt, const sf::Vector2f &pos) override {
      data.update(dt, pos);
    }

    bool handleEvent(const sf::Event &event, const sf::Vector2f &pos) override {
      return data.handleEvent(event, pos);
    }

    void draw(sf::RenderTarget &target,
              sf::RenderStates states) const override {
      ::draw(target, data, states);
    }
  };

public:
  template <game_component T>
    requires(!std::same_as<T, Node2D>)
  Node2D(T t)
      : self(std::make_unique<ObjNode2D<T>>(std::move(t))) {}

  Node2D(const Node2D &rhs);
  Node2D &operator=(const Node2D &rhs);

  Node2D(Node2D &&rhs) = default;
  Node2D &operator=(Node2D &&rhs) = default;

  void draw(sf::RenderTarget &target,
            sf::RenderStates states = sf::RenderStates::Default) const;

  void update(sf::Time dt, const sf::Vector2f &pos);

  bool handleEvent(const sf::Event &event, const sf::Vector2f &pos);
};
} // namespace gm