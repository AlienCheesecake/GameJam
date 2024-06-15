#include "mmedia/Animator.hh"
#include "mmedia/draw.hh"
#include "scdc/scene_compose.hh"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <functional>
#include <iostream>
#include <stdexcept>

template <typename T>
concept mouse_handler = requires(T t) {
  { t() } -> std::same_as<sf::Vector2i>;
};

namespace GUI {
struct Component : sf::Transformable, sf::Drawable {
  using ptr = std::shared_ptr<Component>;
  virtual bool handleEvent(const sf::Event &) = 0;
  virtual bool update(sf::Time) = 0;
  virtual void draw(sf::RenderTarget &target,
                    sf::RenderStates states) const = 0;
  virtual ~Component() = default;
};
template <typename T>
concept component_child = std::derived_from<T, Component>;
struct Container : public Component {
  std::vector<Component::ptr> cld_;
  bool update(sf::Time dt) override {
    return std::any_of(cld_.begin(), cld_.end(),
                       [dt](auto &&i) { return i->update(dt); });
  }
  bool handleEvent(const sf::Event &ev) override {
    return std::any_of(cld_.begin(), cld_.end(),
                       [&ev](auto &&i) { return i->handleEvent(ev); });
  }
  void draw(sf::RenderTarget &rt, sf::RenderStates states) const override {
    states.transform *= getTransform();
    for (auto &&i : cld_)
      ::draw(rt, *i, states);
  }
};

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
  std::function<sf::Vector2i()> mh_;
  std::function<void()> rls_;
  mmed::CharacterAnimation anim_;
  sf::RectangleShape rect_;
  Button(std::function<sf::Vector2i()> mh, std::function<void()> rls,
         const mmed::CharacterAnimation &anim, const sf::RectangleShape &rect)
      : mh_(mh), rls_(rls), anim_(anim), rect_(rect) {
    anim_.select_anim("default");
    anim_.restart();
  }
  inline bool contains() {
    auto [px, py] = mh_();
    auto transform = getInverseTransform();
    auto [x, y] = transform.transformPoint(px, py);
    return rect_.getGlobalBounds().contains(x, y);
  }
  bool update(sf::Time dt) override {
    anim_.update(dt);
    bool b = cur_state;

    switch (cur_state.get()) {
    case D:
      if (contains())
        cur_state = H;
      break;
    case H:
      if (!contains())
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
  bool handleEvent(const sf::Event &ev) override {
    bool b = cur_state;
    if (ev.type == sf::Event::MouseButtonPressed)
      switch (cur_state.get()) {
      case D:
      case H:
        if (contains())
          cur_state = P;
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
//
struct MenuScene : scdc::Scene {
  std::vector<mmed::Animation> button_anim = {
      mmed::AnimationManager::getAnimation("b1_0"),
      mmed::AnimationManager::getAnimation("b1_1"),
      mmed::AnimationManager::getAnimation("b1_2"),
      mmed::AnimationManager::getAnimation("b1_d"),
  };
  sf::RenderWindow &win_;

  std::function<sf::Vector2i()> get_pos() {
    return [this]() { return sf::Mouse::getPosition(win_); };
  }

  GUI::Button btn;
  MenuScene(scdc::SceneCompose &sc, sf::RenderWindow &win)
      : scdc::Scene(sc), win_(win),
        btn{get_pos(),
            [] { std::cout << "Foo" << std::endl; },
            mmed::CharacterAnimation(button_anim, {}),
            sf::RectangleShape{{100, 50}}} {
    btn.rect_.setPosition(-100, -50);
    btn.move(100, 50);
  }

  void draw() override { ::draw(win_, btn); }
  bool update(sf::Time dt) override {
    btn.update(dt);
    return true;
  }
  bool handleEvent(const sf::Event &event) override {
    btn.handleEvent(event);
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
