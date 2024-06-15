#include "button.hh"
#include <map>
#include <string>

using namespace mmed::gmui;
namespace {
const std::map<Button::e_states, std::string> state_string = {
    {Button::e_states::D, "default"},
    {Button::e_states::P, "pressed"},
    {Button::e_states::R, "released"},
    {Button::e_states::H, "hovered"}};
}

Button::changed &Button::changed::operator=(Button::e_states &&x) {
  val = x, check = true;
  return *this;
}

Button::changed::operator bool() {
  auto tmp = check;
  check = false;
  return tmp;
}
Button::e_states Button::changed::get() { return val; }

Button::Button(std::function<void()> prs, std::function<void()> rls,
               const mmed::CharacterAnimation &anim,
               const sf::RectangleShape &rect)
    : prs_(prs), rls_(rls), anim_(anim), rect_(rect) {
  anim_.select_anim("default");
  anim_.restart();
}

bool Button::contains(const sf::Vector2f &p) {
  auto [px, py] = p;
  auto &&transform = getInverseTransform();
  auto [x, y] = transform.transformPoint(px, py);
  return rect_.getGlobalBounds().contains(x, y);
}

void Button::update(sf::Time dt, const sf::Vector2f &pos) {
  anim_.update(dt);
  bool b = cur_state;

  switch (cur_state.get()) {
  case D:
    if (contains(pos))
      cur_state = H;
    break;
  case H:
    if (!contains(pos))
      cur_state = D;
    break;
  case R:
    if (anim_.finished())
      rls_(), cur_state = D;
  default:
    break;
  }

  if (static_cast<bool>(cur_state)) {
    anim_.select_anim(state_string.at(cur_state.get()));
    anim_.restart();
  }
}

bool Button::handleEvent(const sf::Event &ev, const sf::Vector2f &pos) {
  bool b = cur_state;
  if (ev.type == sf::Event::MouseButtonPressed)
    switch (cur_state.get()) {
    case D:
    case H:
      if (contains(pos))
        prs_(), cur_state = P;
      break;
    default:
      break;
    }
  if (ev.type == sf::Event::MouseButtonReleased)
    switch (cur_state.get()) {
    case P:
      cur_state = R;
    default:
      break;
    }
  if (static_cast<bool>(cur_state)) {
    anim_.select_anim(state_string.at(cur_state.get()));
    anim_.restart();
  }
  return true;
}

void Button::draw(sf::RenderTarget &target, sf::RenderStates states) const {
  states.transform *= getTransform();
  ::draw(target, anim_, states);
}