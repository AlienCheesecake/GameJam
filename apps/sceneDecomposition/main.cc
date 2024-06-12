#include "mmedia/Animator.hh"
#include "mmedia/AssetManager.hh"
#include "mmedia/MusicPlayer.hh"
#include "mmedia/draw.hh"
#include "scdc/scene_compose.hh"
#include <SFML/Audio.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Context.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <random>


struct A : scdc::Scene {
  mmed::MusicField mf{"audio/sleep.ogg"};
  A(scdc::SceneCompose &cmp) : Scene(cmp) { std::cout << "A\n"; }
  void draw(sf::RenderTarget &) override {}
  bool update(sf::Time dt) override { return false; }

  bool handleEvent(const sf::Event &event) override;
  ~A() { std::cout << "~A\n"; }
};

std::default_random_engine dre{std::random_device{}()};
std::uniform_real_distribution<float> uid{0, 200};

struct B : scdc::Scene {
  mmed::CharacterAnimation ca_;
  sf::CircleShape shape{50};
  mmed::MusicField mf{"audio/goofy.ogg"};
  B(scdc::SceneCompose &cmp)
      : Scene(cmp), ca_({mmed::AnimationManager::getAnimation("na_l")}, {}) {
    ca_.sp_.setPosition({uid(dre), uid(dre)});
    ca_.select_anim("dance");
    ca_.restart();
    std::cout << "B\n";
  }
  void draw(sf::RenderTarget &rt) override { ::draw(rt, ca_); }
  bool update(sf::Time dt) override {
    ca_.update(dt);
    return false;
  }
  bool handleEvent(const sf::Event &event) override { return true; }
  ~B() { std::cout << "~B\n"; }
};

struct M : scdc::Scene {
  mmed::MusicPauseField mpf;
  sf::Sound sound;
  M(scdc::SceneCompose &cmp)
      : Scene(cmp),
        sound(mmed::AssetManager::getSoundBuffer("audio/quack.wav")) {
    std::cout << "M\n";
  }
  void draw(sf::RenderTarget &) override {}
  bool update(sf::Time dt) override { return true; }
  bool handleEvent(const sf::Event &event) override { 
    if (event.type == sf::Event::MouseButtonPressed) 
      std::cout << "Quack!\n", sound.play();
    return true;
  }
  ~M() { std::cout << "~M\n"; }
};

bool A::handleEvent(const sf::Event &event) {
  if (event.type == sf::Event::KeyPressed)
    switch (event.key.code) {
    case sf::Keyboard::A:
      cmp_.pending_push<A>();
      break;
    case sf::Keyboard::B:
      cmp_.pending_push<B>();
      break;
    case sf::Keyboard::P:
      cmp_.pending_pop();
      break;
    case sf::Keyboard::C:
      cmp_.pending_clear();
      cmp_.pending_push<A>();
      break;
    case sf::Keyboard::M:
      cmp_.pending_push<M>();
      break;
    default:
      break;
    }
  return false;
}

int main() try {
  auto &&aniM = mmed::AnimationManager::getInstance();
  aniM.loadFile("animations.json");
  auto scmp = scdc::SceneCompose{};
  scmp.pending_push<A>();

  auto window = sf::RenderWindow{sf::VideoMode(400, 400), "Test Manager",
                                 sf::Style::Titlebar | sf::Style::Close};
  sf::Clock clock;

  while (window.isOpen()) {
    auto dt = clock.restart();

    sf::Event event;
    while (window.pollEvent(event)) {
      scmp.handleEvent(event);
      if (event.type == sf::Event::Closed)
        window.close();
    }
    scmp.update(dt);
    if (scmp.empty())
      window.close();
    window.clear();
    draw(window, scmp);
    window.display();
  }
  return 0;
} catch (std::exception &err) {
  std::cerr << "Catched exception: " << err.what() << std::endl;
} catch (...) {
  std::cerr << "Catched unknown exception" << std::endl;
}