#include "mmedia/Animator.hh"
#include "mmedia/AssetManager.hh"
#include "mmedia/MusicPlayer.hh"
#include "mmedia/draw.hh"
#include "scdc/scene_compose.hh"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <concepts>
#include <memory>
#include <nlohmann/json.hpp>

using namespace scdc;
using namespace mmed;

template <typename T>
concept mouse_handler = requires(T t) {
  { t() } -> std::same_as<sf::Vector2i>;
};

template <typename T>
concept void_func = requires(T t) {
  { t() } -> std::same_as<void>;
};

namespace GUI {
struct Component {
  using ptr = std::shared_ptr<Component>;
  virtual bool handleEvent(const sf::Event &) = 0;
  virtual bool update(sf::Time) = 0;
  virtual void hover() = 0;
  virtual void press() = 0;
  virtual void release() = 0;
  virtual ~Component() = default;
};
template <typename T>
concept component_child = std::derived_from<T, Component>;
struct Container : public Component {
  std::vector<Component::ptr> cld_;
  void hover() override {
    std::for_each(cld_.begin(), cld_.end(), [](auto &&i) { i->hover(); });
  }
  void press() override {
    std::for_each(cld_.begin(), cld_.end(), [](auto &&i) { i->press(); });
  }
  void release() override {
    std::for_each(cld_.begin(), cld_.end(), [](auto &&i) { i->release(); });
  }
  bool update(sf::Time dt) override {
    return std::any_of(cld_.begin(), cld_.end(),
                       [dt](auto &&i) { return i->update(dt); });
  }
  bool handleEvent(const sf::Event &ev) override {
    return std::any_of(cld_.begin(), cld_.end(),
                       [&ev](auto &&i) { return i->handleEvent(ev); });
  }
  // template<GUI::component_child T, typename... Args>
  // void create_component() {
  //
  // }
};

template <mouse_handler MH, void_func Rls> struct Button : Component {
  sf::RenderTarget &rndr_;
  MH mh_;
  Rls rls_;
  CharacterAnimation anim_;
  sf::IntRect rect_;
  void hover() override { anim_.select_anim("hover"); }
  void press() override { anim_.select_anim("press"); }
  void release() override {
    anim_.select_anim("release");
    rls_();
  }
  bool update(sf::Time dt) override {
    anim_.update(dt);
    return true;
  }
  inline bool contains() { return rect_.contains(mh_()); }
  bool handleEvent(const sf::Event &ev) override {}
};

} // namespace GUI

mmed::Animation one_frame_anim(const std::string &name, const std::string &path,
                               sf::Color mask_clr = {0, 255, 0, 255}) {
  auto [w, h] = mmed::AssetManager::getTexture(std::string(path)).getSize();
  return {name, path, 1, 1, sf::Vector2i(w, h), {}, mask_clr, false};
}

struct Mehehenu : Scene {
  sf::View view_{};
  sf::RenderWindow &window;

  sf::Texture background1 = AssetManager::getTexture("images/fon1.png");
  sf::Texture background2 = AssetManager::getTexture("images/fon2.png");
  sf::Sprite bg;

  mmed::CharacterAnimation start_ca_ = {
      {one_frame_anim("release", "images/start.png"),
       one_frame_anim("hover", "images/start_select.png"),
       one_frame_anim("press", "images/start_final.png")},
      {}};

  mmed::CharacterAnimation exit_ca_ = {
      {one_frame_anim("release", "images/exit.png"),
       one_frame_anim("hover", "images/exit_select.png"),
       one_frame_anim("press", "images/exit_final.png")},
      {}};

  MusicField mf{"audio/goofy.ogg"};
  Mehehenu(SceneCompose &cmp, sf::RenderWindow &win) : Scene(cmp), window(win) {
    view_.setViewport({0.25, 0.25, 0.5, .5});

    start_ca_.sp_.setPosition(750, 200);
    exit_ca_.sp_.setPosition(790, 400);
    start_ca_.select_anim("release");
    exit_ca_.select_anim("release");
    bg.setTexture(background2);
  }
  void draw() override {
    // ::draw(window, bg);
    // window.setView(view_);
    // ::draw(window, start_ca_);
    // ::draw(window, exit_ca_);
    window << bg << start_ca_ << exit_ca_;
    // window.setView(window.getDefaultView());
  }
  bool update(sf::Time dt) override { return false; }
  bool handleEvent(const sf::Event &event) override {
    sf::Vector2i pos = sf::Mouse::getPosition(window);
    sf::Vector2f mousepos = window.mapPixelToCoords(pos);
    if (start_ca_.sp_.getGlobalBounds().contains(mousepos.x, mousepos.y)) {
      start_ca_.select_anim("hover");
      if (event.type == sf::Event::MouseButtonPressed &&
          event.mouseButton.button == sf::Mouse::Left) {
        start_ca_.select_anim("press");
      }
    } else if (exit_ca_.sp_.getGlobalBounds().contains(mousepos.x,
                                                       mousepos.y)) {
      exit_ca_.select_anim("hover");
      if (event.type == sf::Event::MouseButtonPressed &&
          event.mouseButton.button == sf::Mouse::Left) {
        exit_ca_.select_anim("press");
        cmp_.pending_pop();
      }
    } else {
      start_ca_.select_anim("release");
      exit_ca_.select_anim("release");
    }
    return true;
  }
};

struct Menu : Scene {
  sf::RenderWindow &window;

  sf::Texture background1 = AssetManager::getTexture("images/fon1.png");
  sf::Texture background2 = AssetManager::getTexture("images/fon2.png");
  sf::Sprite bg;

  sf::Sprite button_start;
  sf::Texture start = AssetManager::getTexture("images/start.png");
  sf::Texture start_select =
      AssetManager::getTexture("images/start_select.png");
  sf::Texture start_final = AssetManager::getTexture("images/start_final.png");

  sf::Sprite button_exit;
  sf::Texture exit = AssetManager::getTexture("images/exit.png");
  sf::Texture exit_select = AssetManager::getTexture("images/exit_select.png");
  sf::Texture exit_final = AssetManager::getTexture("images/exit_final.png");

  MusicField mf{"audio/sleep.ogg"};
  Menu(SceneCompose &cmp, sf::RenderWindow &win) : Scene(cmp), window(win) {
    button_start.setPosition(750, 200);
    button_exit.setPosition(790, 400);
    bg.setTexture(background1);
    button_start.setTexture(start);
    button_exit.setTexture(exit);
  }
  void draw() override {
    // ::draw(window, bg);
    // ::draw(window, button_start);
    // ::draw(window, button_exit);
    window << bg << button_start << button_exit;
  }
  bool update(sf::Time dt) override {
    // ca_.update(dt);
    return false;
  }
  bool handleEvent(const sf::Event &event) override {
    sf::Vector2i pos = sf::Mouse::getPosition(window);
    sf::Vector2f mousepos = window.mapPixelToCoords(pos);
    if (button_start.getGlobalBounds().contains(mousepos.x, mousepos.y)) {
      button_start.setTexture(start_select);
      if (event.type == sf::Event::MouseButtonPressed &&
          event.mouseButton.button == sf::Mouse::Left) {
        bg.setTexture(background2);
        button_start.setTexture(start_final);
        cmp_.pending_pop();
        cmp_.pending_push<Mehehenu>(window);
      }
    } else if (button_exit.getGlobalBounds().contains(mousepos.x, mousepos.y)) {
      button_exit.setTexture(exit_select);
      if (event.type == sf::Event::MouseButtonPressed &&
          event.mouseButton.button == sf::Mouse::Left) {
        button_exit.setTexture(exit_final);
        cmp_.pending_pop();
      }
    } else {
      button_start.setTexture(start);
      button_exit.setTexture(exit);
    }
    return true;
  }
};

int main() {
  auto window = sf::RenderWindow{sf::VideoMode(1920, 1080), "Test Manager",
                                 sf::Style::Titlebar | sf::Style::Close};
  sf::Clock clock;
  SceneCompose scmp = SceneCompose();
  scmp.pending_push<Menu>(window);

  while (window.isOpen()) {
    auto dt = clock.restart();

    sf::Event event;
    while (window.pollEvent(event)) {
      scmp.handleEvent(event);
      if (event.type == sf::Event::Closed)
        window.close();
    }
    scmp.update(dt);
    if (scmp.empty()) {
      window.close();
    }
    window.clear();
    scmp.draw();
    window.display();
  }
  return 0;
}