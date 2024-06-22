#include "ListNode.hh"
#include "mmedia/draw.hh"

using namespace gm;

sf::Vector2f ListNode::transform_coords(const sf::Vector2f &pos) {
  auto [px, py] = pos;
  auto &&transform = getInverseTransform();
  return transform.transformPoint(px, py);
}
void ListNode::update(sf::Time dt, const sf::Vector2f &pos) {
  auto t_pos = transform_coords(pos);
  for (auto &&i : cld)
    i.update(dt, t_pos);
}
bool ListNode::handleEvent(const sf::Event &event, const sf::Vector2f &pos) {
  auto t_pos = transform_coords(pos);
  return !std::all_of(cld.begin(), cld.end(), [&event, &pos](auto &&i) {
    return i.handleEvent(event, pos);
  });
}

void draw(sf::RenderTarget &target, const ListNode &t,
          sf::RenderStates states) {
  states.transform *= t.getTransform();
  for (auto &&i : t.cld)
    ::draw(target, i, states);
}