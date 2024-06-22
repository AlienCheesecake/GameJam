#include "gmui/button.hh"
#include "gmui/dd.hh"
#include "mmedia/Animator.hh"
#include "mmedia/MusicPlayer.hh"
#include "mmedia/MusicStackOfQueues.hh"
#include "mmedia/draw.hh"
#include "scdc/scene_compose.hh"
#include <SFML/Audio/Music.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <vcruntime_typeinfo.h>
#include <vector>
#include "gmui/ListNode.hh"

using namespace mmed::gmui;
using namespace mmed;

struct Foo {
  Foo() { std::cout << "Foo()\n"; }
  ~Foo() { std::cout << "~Foo()\n"; }
  Foo(Foo &&) { std::cout << "Foo(&&)\n"; }
  Foo(const Foo &) { std::cout << "Foo(c&)\n"; }
  Foo &operator=(Foo &&) {
    std::cout << "=Foo(&&)\n";
    return *this;
  }
  Foo &operator=(const Foo &) {
    std::cout << "=Foo(c&)\n";
    return *this;
  }
  void update(sf::Time time, const sf::Vector2f &pos) {
    // std::cout << "Foo.update(dt, pos)\n";
  }
  bool handleEvent(const sf::Event &event, const sf::Vector2f &pos) {
    // std::cout << "Foo.handleEvent(event, pos)\n";
    return true;
  }
};

void draw(sf::RenderTarget &rt, const Foo &foo,
          sf::RenderStates states = sf::RenderStates::Default) {
  // std::cout << "::draw(target, foo, states)\n";
}

struct Menu3 : scdc::Scene {
  MusicPauseField mpf;
  sf::RenderWindow &win;

  mmed::CharacterAnimation button_anim = {
      {
          mmed::AnimationManager::getAnimation("b1_0"),
          mmed::AnimationManager::getAnimation("b1_1"),
          mmed::AnimationManager::getAnimation("b1_2"),
          mmed::AnimationManager::getAnimation("b1_d"),
      },
      {}};

  BoolDrawable<mmed::CharacterAnimation> follow = {
      {{mmed::Animation::one_frame_anim("start", "images/start.png"),
        mmed::Animation::one_frame_anim("exit", "images/exit.png")},
       {}},
      false};

  gm::ListNode components;
  bool is_move = false;

  Menu3(scdc::SceneCompose &cmp, sf::RenderWindow &window)
      : scdc::Scene(cmp), win(window) {
    auto dd_start = DD(button_anim, sf::RectangleShape{{100, 50}}, follow, "start");
    auto dd_exit = DD(button_anim, sf::RectangleShape{{100, 50}}, follow, "exit");
    dd_exit.move(0, 50);
    auto move_btn = Button([this]{is_move = true;}, [this]{is_move = false;}, button_anim, sf::RectangleShape{{100, 50}});
    move_btn.move(0, 100);
    components.cld = {dd_start, dd_exit, move_btn};
  }
  void draw() override {
    ::draw(win, components);
    ::draw(win, follow);
  }
  sf::Vector2f world_pos() {
    auto &&pixelPos = sf::Mouse::getPosition(win);
    return win.mapPixelToCoords(pixelPos);
  }
  bool update(sf::Time dt) override {
    if (is_move)
      components.move(0, 0.01);
    components.update(dt, world_pos());
    follow.update(dt);
    follow.t.sp_.setPosition(world_pos());
    return false;
  }
  bool handleEvent(const sf::Event &event) override {
    components.handleEvent(event, world_pos());
    return false;
  }
};

struct MenuScene : scdc::Scene {
#if 0
  BeginLoopMusicField mf{"audio/main_menu_begin.ogg",
                         "audio/main_menu_loop.ogg"};
#endif
  sf::View view;
  std::vector<mmed::Animation> button_anim = {
      mmed::AnimationManager::getAnimation("b1_0"),
      mmed::AnimationManager::getAnimation("b1_1"),
      mmed::AnimationManager::getAnimation("b1_2"),
      mmed::AnimationManager::getAnimation("b1_d"),
  };

  BoolDrawable<mmed::CharacterAnimation> follow = {
      {{mmed::Animation::one_frame_anim("start", "images/start.png"),
        mmed::Animation::one_frame_anim("exit", "images/exit.png")},
       {}},
      false};

  sf::RenderWindow &win_;
  MusicStackOfQueues &ms_;
  NMusicField mf{ms_,
                 {
                     {"audio/main_menu_begin.ogg", false},
                     {"audio/main_menu_loop.ogg", true},
                 }};

  Button btn;
  DD dd;
  MenuScene(scdc::SceneCompose &sc, sf::RenderWindow &win,
            MusicStackOfQueues &ms)
      : scdc::Scene(sc), win_(win), ms_(ms),
        btn{[] { std::cout << "Foo" << std::endl; },
            [this] {
              cmp_.pending_pop();
              cmp_.pending_push<Menu3>(win_);
            },
            mmed::CharacterAnimation(button_anim, {}),
            sf::RectangleShape{{100, 50}}},
        dd(
            mmed::CharacterAnimation{button_anim, {}},
            sf::RectangleShape{{100, 50}}, follow, "start",
            [] { std::cout << "Foo\n"; }, [] { std::cout << "Bar\n"; }) {
    follow.t.sp_.setScale(.5, .5);
    view.setViewport({.25, .25, .5, .5});
    btn.move(120, 100);
    btn.rotate(-30);
    btn.scale({2, 2});
  }

  void draw() override {
    {
      scdc::tmp_view tv(win_, view);
      ::draw(win_, btn);
      ::draw(win_, dd);
    }
    ::draw(win_, follow);
  }
  sf::Vector2f world_pos() {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(win_);
    return win_.mapPixelToCoords(pixelPos);
  }
  bool update(sf::Time dt) override {
    {
      scdc::tmp_view tv(win_, view);
      btn.update(dt, world_pos());
      dd.update(dt, world_pos());
    }
    follow.update(dt);
    follow.t.sp_.setPosition(world_pos());
    return true;
  }
  bool handleEvent(const sf::Event &event) override {
    scdc::tmp_view tv(win_, view);
    btn.handleEvent(event, world_pos());
    dd.handleEvent(event, world_pos());
    return true;
  }
};

int main() {
  MusicStackOfQueues ms;

  scdc::SceneCompose sc;
  auto &&aniM = mmed::AnimationManager::getInstance();
  aniM.loadFile("animations.json");
  auto window = sf::RenderWindow{sf::VideoMode(500, 300), "Test Manager",
                                 sf::Style::Titlebar | sf::Style::Close};
  // MusicStack ms;
  sc.pending_push<MenuScene>(window, ms);
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
    ms.update();
    if (sc.empty())
      window.close();
    window.clear();
    sc.draw();
    window.display();
  }

  return 0;
}
