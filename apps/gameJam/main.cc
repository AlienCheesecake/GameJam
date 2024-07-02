#include "mmedia/Animator.hh"
#include "mmedia/AssetManager.hh"
#include "mmedia/MusicPlayer.hh"
#include "mmedia/draw.hh"
#include "scdc/scene_compose.hh"
#include <SFML/Audio.hpp>
#include <SFML/Audio/Music.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
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
#include <iterator>
#include <latch>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string_view>
#include <thread>
#include <vector>

using namespace scdc;
using namespace mmed;

struct BeginLoopMusicField {
  bool was_paused;
  std::string loop_path_, begin_path_, prev_path_;
  mutable std::mutex mtx_;
  std::jthread thr_begin, thr_loop;
  std::latch await_latch{1};

  void await_begin_end() {
    for (;;) {
      std::lock_guard lk(mtx_);
      auto &&mp = MusicPlayer::getInstance().getStatus();
      if (mp != sf::Music::Status::Playing)
        break;
    }
    {
      std::lock_guard lk{mtx_};
      await_latch.count_down();
    }
  }

  void begin_loop() {
    await_latch.wait();
    {
      std::lock_guard lk{mtx_};
      auto &&mp = MusicPlayer::getInstance();
      mp.play(loop_path_);
      mp.setLoop(true);
    }
  }

  BeginLoopMusicField(const std::string_view begin_path,
                      const std::string_view loop_path)
      : begin_path_(begin_path), loop_path_(loop_path),
        thr_begin([this] { await_begin_end(); }),
        thr_loop([this] { begin_loop(); }) {
    auto &&mp = MusicPlayer::getInstance();
    prev_path_ = mp.path();
    was_paused = mp.paused();
    mp.set_pause(false);
    mp.play(begin_path);
    mp.setLoop(false);
  }
  ~BeginLoopMusicField() {
    auto &&mp = MusicPlayer::getInstance();
    mp.set_pause(was_paused);
    mp.play(prev_path_);
  }
};

struct Mehehenu : Scene {
  sf::View view_{};
  sf::RenderWindow &window;

  sf::Texture background1 = AssetManager::getTexture("images/fon1.png");
  sf::Texture background2 = AssetManager::getTexture("images/fon2.png");
  sf::Sprite bg;

  mmed::CharacterAnimation start_ca_ = {
      {mmed::Animation::one_frame_anim("release", "images/start.png"),
       mmed::Animation::one_frame_anim("hover", "images/start_select.png"),
       mmed::Animation::one_frame_anim("press", "images/start_final.png")},
      {}};

  mmed::CharacterAnimation exit_ca_ = {
      {mmed::Animation::one_frame_anim("release", "images/exit.png"),
       mmed::Animation::one_frame_anim("hover", "images/exit_select.png"),
       mmed::Animation::one_frame_anim("press", "images/exit_final.png")},
      {}};

  inline static auto ml = []() -> sf::Vector2i {
    return sf::Mouse::getPosition();
  };

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
    ::draw(window, bg);
    ::draw(window, start_ca_);
    ::draw(window, exit_ca_);
    // window << bg << start_ca_ << exit_ca_;
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
        compositor_.pending_pop();
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
  std::vector<sf::CircleShape> circles;
  std::vector<sf::Drawable *> test;

  // MusicField mf{"audio/sleep.ogg"};
  BeginLoopMusicField mf{"audio/main_menu_begin.ogg",
                         "audio/main_menu_loop.ogg"};
  Menu(SceneCompose &cmp, sf::RenderWindow &win) : Scene(cmp), window(win) {
    for (size_t i = 0; i < 5; ++i) {
      circles.emplace_back(i * 50);
      circles.back().setPosition(i * 50, i * 10);
    }
    auto insrt = std::back_inserter(test);
    for (auto &&i : circles) {
      insrt = &i;
    }
    button_start.setPosition(750, 200);
    button_exit.setPosition(790, 400);
    bg.setTexture(background1);
    button_start.setTexture(start);
    button_exit.setTexture(exit);
  }
  void draw() override {
    ::draw(window, bg);
    ::draw(window, button_start);
    ::draw(window, button_exit);
    // window << bg << button_start << button_exit;
  }
  bool update(sf::Time dt) override { return false; }
  bool handleEvent(const sf::Event &event) override {
    sf::Vector2i pos = sf::Mouse::getPosition(window);
    sf::Vector2f mousepos = window.mapPixelToCoords(pos);
    if (button_start.getGlobalBounds().contains(mousepos.x, mousepos.y)) {
      button_start.setTexture(start_select);
      if (event.type == sf::Event::MouseButtonPressed &&
          event.mouseButton.button == sf::Mouse::Left) {
        bg.setTexture(background2);
        button_start.setTexture(start_final);
        compositor_.pending_pop();
        compositor_.pending_push<Mehehenu>(window);
      }
    } else if (button_exit.getGlobalBounds().contains(mousepos.x, mousepos.y)) {
      button_exit.setTexture(exit_select);
      if (event.type == sf::Event::MouseButtonPressed &&
          event.mouseButton.button == sf::Mouse::Left) {
        button_exit.setTexture(exit_final);
        compositor_.pending_pop();
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