#include "dd.hh"
#include "button.hh"
#include "mmedia/draw.hh"

void FollowAnim::update(sf::Time dt, const sf::Vector2f &pos) {
  anim.sp_.setPosition(pos);
  anim.update(dt);
}

void FollowAnim::draw(sf::RenderTarget &rt, sf::RenderStates states) const {
  ::draw(rt, anim, states);
}

using namespace mmed::gmui;

DD::DD(const mmed::CharacterAnimation &btn_anim, const sf::RectangleShape &rs,
       const mmed::CharacterAnimation &anim, std::function<void()> prs,
       std::function<void()> rls)
    : btn_(
          [this, prs] {
            prs();
            flw_anim.check = true;
          },
          [this, rls] {
            rls();
            flw_anim.check = false;
          },
          btn_anim, rs),
      flw_anim({anim}, false) {
  flw_anim.t.anim.select_anim("follow");
  flw_anim.t.anim.restart();
}

sf::Vector2f DD::inner_pos(const sf::Vector2f &p) {
  auto [px, py] = p;
  auto transform = getInverseTransform();
  return transform.transformPoint(px, py);
}

bool DD::update(sf::Time dt, const sf::Vector2f &pos) {
  btn_.update(dt, inner_pos(pos));
  return true;
}

bool DD::follow_update(sf::Time dt, const sf::Vector2f &pos) {
  auto [x, y] = pos;
  flw_anim.update(dt, pos);
  return true;
}

bool DD::handleEvent(const sf::Event &ev, const sf::Vector2f &pos) {
  btn_.handleEvent(ev, pos);
  return true;
}

void DD::draw(sf::RenderTarget &target, sf::RenderStates states) const {
  ::draw(target, btn_, states);
}

void DD::follow_draw(sf::RenderTarget &target, sf::RenderStates states) const {
  ::draw(target, flw_anim, states);
}