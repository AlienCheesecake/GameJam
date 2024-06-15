#include "mmedia/Animator.hh"
#include "mmedia/draw.hh"
#include "scdc/scene_compose.hh"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <cstdarg>
#include <functional>
#include <iostream>

namespace GUI {
struct Component : sf::Transformable, sf::Drawable {
  using ptr = std::shared_ptr<Component>;
  virtual bool handleEvent(const sf::Event &, const sf::Vector2f &pos) = 0;
  virtual bool update(sf::Time, const sf::Vector2f &pos) = 0;
  virtual void draw(sf::RenderTarget &target,
                    sf::RenderStates states) const = 0;
  virtual ~Component() = default;
};
template <typename T>
concept component_child = std::derived_from<T, Component>;
class Button : public Component {
  enum e_states { D, P, R, H };
  std::map<e_states, std::string> state_string = {
      {D, "default"}, {P, "pressed"}, {R, "released"}, {H, "hovered"}};

  struct changed final {
  private:
    bool check = false;
    e_states val = D;

  public:
    changed &operator=(e_states &&x) {
      val = x, check = true;
      return *this;
    }
    operator bool() {
      auto tmp = check;
      check = false;
      return tmp;
    }
    e_states get() { return val; }
  };
  changed cur_state;

public:
  std::function<void()> prs_, rls_;
  mmed::CharacterAnimation anim_;
  sf::RectangleShape rect_;
  Button(std::function<void()> prs, std::function<void()> rls,
         const mmed::CharacterAnimation &anim, const sf::RectangleShape &rect)
      : prs_(prs), rls_(rls), anim_(anim), rect_(rect) {
    anim_.select_anim("default");
    anim_.restart();
  }
  inline bool contains(const sf::Vector2f &p) {
    auto [px, py] = p;
    auto transform = getInverseTransform();
    auto [x, y] = transform.transformPoint(px, py);
    return rect_.getGlobalBounds().contains(x, y);
  }
  bool update(sf::Time dt, const sf::Vector2f &pos) override {
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
      anim_.select_anim(state_string[cur_state.get()]);
      anim_.restart();
    }
    return true;
  }
  bool handleEvent(const sf::Event &ev, const sf::Vector2f &pos) override {
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
      anim_.select_anim(state_string[cur_state.get()]);
      anim_.restart();
    }
    return true;
  }
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
    states.transform *= getTransform();
    ::draw(target, anim_, states);
  }
};
} // namespace GUI

struct FollowAnim {
  mmed::CharacterAnimation anim;
  void update(sf::Time dt, const sf::Vector2f &pos) {
    anim.sp_.setPosition(pos);
    anim.update(dt);
  }
  void draw(sf::RenderTarget &rt, sf::RenderStates states) const {
    ::draw(rt, anim, states);
  }
};

template <typename T> struct BoolDrawable {
  T t;
  bool check = true;
  void update(sf::Time dt, const sf::Vector2f &pos) {
    if (check)
      t.update(dt, pos);
  }
  void draw(sf::RenderTarget &rt, sf::RenderStates states) const {
    if (check)
      ::draw(rt, t, states);
  }
  void handleEvent(const sf::Event &event, const sf::Vector2f &pos) {
    if (check)
      t.handleEvent(event, pos);
  }
};

class DD : public GUI::Component {
  GUI::Button btn_;
  BoolDrawable<FollowAnim> flw_anim;

public:
  DD(
      const mmed::CharacterAnimation &btn_anim, const sf::RectangleShape &rs,
      const mmed::CharacterAnimation &anim, std::function<void()> prs = [] {},
      std::function<void()> rls = [] {})
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
        flw_anim({anim}, false) {}
  sf::Vector2f inner_pos(const sf::Vector2f &p) {
    flw_anim.t.anim.select_anim("follow");
    flw_anim.t.anim.restart();
    auto [px, py] = p;
    auto transform = getInverseTransform();
    return transform.transformPoint(px, py);
  }
  bool update(sf::Time dt, const sf::Vector2f &pos) override {
    btn_.update(dt, inner_pos(pos));
    return true;
  }
  bool follow_update(sf::Time dt, const sf::Vector2f &pos) {
    auto [x, y] = pos;
    flw_anim.update(dt, pos);
    return true;
  }
  bool handleEvent(const sf::Event &ev, const sf::Vector2f &pos) override {
    btn_.handleEvent(ev, pos);
    return true;
  }
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
    ::draw(target, btn_, states);
  }
  void follow_draw(sf::RenderTarget &target, sf::RenderStates states) const {
    ::draw(target, flw_anim, states);
  }
};

struct MenuScene : scdc::Scene {
  sf::View view;
  std::vector<mmed::Animation> button_anim = {
      mmed::AnimationManager::getAnimation("b1_0"),
      mmed::AnimationManager::getAnimation("b1_1"),
      mmed::AnimationManager::getAnimation("b1_2"),
      mmed::AnimationManager::getAnimation("b1_d"),
  };
  sf::RenderWindow &win_;
  struct tmp_view {
    sf::RenderWindow &win_;
    tmp_view(sf::RenderWindow &win, const sf::View &view) : win_(win) {
      win.setView(view);
    }
    ~tmp_view() { win_.setView(win_.getDefaultView()); }
  };

  GUI::Button btn;
  DD dd;
  MenuScene(scdc::SceneCompose &sc, sf::RenderWindow &win)
      : scdc::Scene(sc), win_(win),
        btn{[] { std::cout << "Foo" << std::endl; },
            [] { std::cout << "Bar" << std::endl; },
            mmed::CharacterAnimation(button_anim, {}),
            sf::RectangleShape{{100, 50}}},
        dd(
            mmed::CharacterAnimation{button_anim, {}},
            sf::RectangleShape{{100, 50}},
            mmed::CharacterAnimation{
                {mmed::Animation::one_frame_anim("follow", "images/start.png")},
                {}},
            [] { std::cout << "Foo\n"; }, [] { std::cout << "Bar\n"; }) {
    view.setViewport({.25, .25, .5, .5});
    btn.move(120, 100);
    btn.rotate(-30);
    btn.scale({2, 2});
  }

  void draw() override {
    {
      tmp_view tv(win_, view);
      ::draw(win_, btn);
      ::draw(win_, dd);
    }
    dd.follow_draw(win_, sf::RenderStates::Default);
  }
  sf::Vector2f world_pos() {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(win_);
    return win_.mapPixelToCoords(pixelPos);
  }
  bool update(sf::Time dt) override {
    {
      tmp_view tv(win_, view);
      btn.update(dt, world_pos());
      dd.update(dt, world_pos());
    }
    dd.follow_update(dt, world_pos());
    return true;
  }
  bool handleEvent(const sf::Event &event) override {
    tmp_view tv(win_, view);
    btn.handleEvent(event, world_pos());
    dd.handleEvent(event, world_pos());
    return true;
  }
};

int main() {
  scdc::SceneCompose sc;
  auto &&aniM = mmed::AnimationManager::getInstance();
  aniM.loadFile("animations.json");
  auto window = sf::RenderWindow{sf::VideoMode(500, 300), "Test Manager",
                                 sf::Style::Titlebar | sf::Style::Close};
  sc.pending_push<MenuScene>(window);
  sf::Clock clck;
  while (window.isOpen()) {
    auto dt = clck.restart();
    sf::Event event;
    while (window.pollEvent(event)) {
      sc.handleEvent(event);
      if (event.type == sf::Event::Closed)
        window.close();
    }
    sc.update(dt);
    if (sc.empty())
      window.close();
    window.clear();
    sc.draw();
    window.display();
  }

  return 0;
}
