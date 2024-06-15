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
       BoolDrawable<CharacterAnimation> &outsorce, std::string_view anim_name,
       std::function<void()> prs, std::function<void()> rls)
    : btn_(
          [&outsorce, prs, anim_name] {
            prs();
            outsorce.t.select_anim(anim_name);
            outsorce.check = true;
          },
          [&outsorce, rls] {
            rls();
            outsorce.check = false;
          },
          btn_anim, rs) {
}

sf::Vector2f DD::inner_pos(const sf::Vector2f &p) {
  auto [px, py] = p;
  auto transform = getInverseTransform();
  return transform.transformPoint(px, py);
}

void DD::update(sf::Time dt, const sf::Vector2f &pos) {
  btn_.update(dt, inner_pos(pos));
}

bool DD::handleEvent(const sf::Event &ev, const sf::Vector2f &pos) {
  btn_.handleEvent(ev, inner_pos(pos));
  return true;
}

void DD::draw(sf::RenderTarget &target, sf::RenderStates states) const {
  states.transform *= getTransform();
  ::draw(target, btn_, states);
}
