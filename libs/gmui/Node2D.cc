#include "Node2D.hh"

void gm::Node2D::draw(sf::RenderTarget &target,
                             sf::RenderStates states) const {
  self->draw(target, states);
}

void gm::Node2D::update(sf::Time dt, const sf::Vector2f &pos) {
  self->update(dt, pos);
}

bool gm::Node2D::handleEvent(const sf::Event &event,
                                    const sf::Vector2f &pos) {
  return self->handleEvent(event, pos);
}

gm::Node2D::Node2D(const gm::Node2D &rhs)
    : self(rhs.self->copy()) {}
gm::Node2D &gm::Node2D::operator=(const gm::Node2D &rhs) {
  auto tmp = rhs;
  std::swap(*this, tmp);
  return *this;
}
