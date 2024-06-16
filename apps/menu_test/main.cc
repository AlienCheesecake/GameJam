#include "gmui/button.hh"
#include "gmui/component.hh"
#include "gmui/dd.hh"
#include "mmedia/Animator.hh"
#include "mmedia/MusicPlayer.hh"
#include "mmedia/draw.hh"
#include "scdc/scene_compose.hh"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <functional>
#include <iostream>
#include <mutex>
#include <vector>
#include <thread>

using namespace mmed::gmui;
using namespace mmed;

struct BeginLoopMusicField {
  bool was_paused;
  std::string loop_path_, begin_path_, prev_path_;
  std::jthread thr_dj;
  // mutable std::mutex mtx;
  std::atomic_bool end_thread = false;

  void dj_thread() {
    auto &&mp = MusicPlayer::getInstance();
    {
      // std::lock_guard lk{mtx};
      std::cout << "Begin part\n";
      prev_path_ = mp.path();
      was_paused = mp.paused();
      mp.set_pause(false);
      mp.play(begin_path_);
      mp.setLoop(false);
    }
    for (;;) {
      // std::lock_guard lk{mtx};
      if (mp.getStatus() != sf::Music::Status::Playing || end_thread)
        break;
    }
    {
      // std::lock_guard lk{mtx};
      std::cout << "The loop part\n";
      mp.play(loop_path_);
      mp.setLoop(true);
    }
  }

  BeginLoopMusicField(const std::string_view begin_path,
                      const std::string_view loop_path)
      : begin_path_(begin_path), loop_path_(loop_path),
        thr_dj([this] { dj_thread(); }) {}
  ~BeginLoopMusicField() {
    end_thread = true;
    thr_dj.join();
    // std::lock_guard lk{mtx};
    auto &&mp = MusicPlayer::getInstance();
    mp.set_pause(was_paused);
    mp.play(prev_path_);
  }
};

namespace mmed::gmui {
struct Container : Component {
  std::vector<Component::ptr> cld;

  Container(const std::vector<Component::ptr> &components) : cld(components) {}

  sf::Vector2f transform_coords(const sf::Vector2f &pos) {
    auto [px, py] = pos;
    auto &&transform = getInverseTransform();
    return transform.transformPoint(px, py);
  }

  void update(sf::Time dt, const sf::Vector2f &pos) override {
    auto t_pos = transform_coords(pos);
    for (auto &&i : cld)
      i->update(dt, t_pos);
  }

  bool handleEvent(const sf::Event &event, const sf::Vector2f &pos) override {
    auto t_pos = transform_coords(pos);
    return !std::all_of(cld.begin(), cld.end(), [&event, &t_pos](auto &&i) {
      return i->handleEvent(event, t_pos);
    });
  }
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
    states.transform *= getTransform();
    for (auto &&i : cld)
      ::draw(target, *i, states);
  }
};
} // namespace mmed::gmui

struct Menu2 : scdc::Scene {
  // MusicPauseField mpf;
  BeginLoopMusicField mf {
    "audio/piano_main_menu_begin.ogg",
    "audio/piano_main_menu_loop.ogg"
  };
  sf::RenderWindow &win_;

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

  bool move = false;

#if 1
  Container menu = {
      {Component::ptr{
           new DD(button_anim, sf::RectangleShape({100, 50}), follow, "start")},
       Component::ptr{
           new DD(button_anim, sf::RectangleShape({100, 50}), follow, "exit")},
       Component::ptr(new Button([this] { move = true; },
                                 [this] {
                                   move = false;
                                   std::cout << "Foo" << std::endl;
                                 },
                                 button_anim, sf::RectangleShape{{100, 50}}))}};
#endif

  Menu2(scdc::SceneCompose &sc, sf::RenderWindow &win)
      : scdc::Scene(sc), win_(win) {
#if 1
    follow.t.sp_.setScale(.4, .4);
    for (size_t i = 0; i < 3; ++i) {
      menu.cld[i]->setPosition({0, (float)(i * 60)});
    }
#endif
  }
  sf::Vector2f world_pos() {
    auto &&pixelPos = sf::Mouse::getPosition(win_);
    return win_.mapPixelToCoords(pixelPos);
  }
  void draw() override {
    ::draw(win_, menu);
    ::draw(win_, follow);
  }
  bool update(sf::Time dt) override {
    if (move)
      menu.move({0, 0.005});
    menu.update(dt, world_pos());
    follow.update(dt);
    follow.t.sp_.setPosition(world_pos());
    return false;
  }
  bool handleEvent(const sf::Event &event) override {
    menu.handleEvent(event, world_pos());
    return false;
  }
};

struct MenuScene : scdc::Scene {
  BeginLoopMusicField mf{"audio/main_menu_begin.ogg",
                         "audio/main_menu_loop.ogg"};
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

  Button btn;
  DD dd;
  MenuScene(scdc::SceneCompose &sc, sf::RenderWindow &win)
      : scdc::Scene(sc), win_(win),
        btn{[] { std::cout << "Foo" << std::endl; },
            [this] {
              cmp_.pending_pop();
              cmp_.pending_push<Menu2>(win_);
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
