#include "mmedia/Animator.hh"
#include "mmedia/AssetManager.hh"
#include "mmedia/MusicPlayer.hh"
#include "mmedia/draw.hh"
#include "scdc/scene_compose.hh"
#include <SFML/Audio.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Context.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <future>
#include <iostream>
#include <latch>
#include <mutex>
#include <random>
#include <thread>

struct Foo {
  Foo() { std::cout << "Foo()\n"; }
  Foo(const Foo &) { std::cout << "Foo(c&)\n"; }
  Foo(Foo &&) { std::cout << "Foo(&&)\n"; }
  Foo &operator=(const Foo &) {
    std::cout << "Foo=(c&)\n";
    return *this;
  }
  Foo &operator=(Foo &&) {
    std::cout << "Foo=(&&)\n";
    return *this;
  }
  ~Foo() { std::cout << "~Foo()\n"; }
};

struct A : scdc::Scene {
  int x = 0;
  Foo y;
  // mmed::MusicField mf{"audio/sleep.ogg"};
  A(scdc::SceneCompose &cmp) : Scene(cmp) { std::cout << "A\n"; }
  void draw(sf::RenderTarget &) override {}
  bool update(sf::Time dt) override { return false; }

  bool handleEvent(const sf::Event &event) override;
  ~A() { std::cout << "~A:" << x << "\n"; }
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
  int &x;
  Foo &foo;
  M(scdc::SceneCompose &cmp, int &k, Foo &f)
      : Scene(cmp), x(k), foo(f),
        sound(mmed::AssetManager::getSoundBuffer("audio/quack.wav")) {
    x = 8;
    std::cout << "M: " << x << "\n";
  }
  void draw(sf::RenderTarget &) override {}
  bool update(sf::Time dt) override { return true; }
  bool handleEvent(const sf::Event &event) override {
    if (event.type == sf::Event::MouseButtonPressed)
      std::cout << "Quack! " << x++ << "\n", sound.play();
    return true;
  }
  ~M() { std::cout << "~M\n"; }
};

struct LoadScene : scdc::Scene {
  sf::CircleShape circle{50};
  mutable std::mutex mt_;
  float x = 0;
#define FUT_PROMISE 0
#define CONDITION_VARIABLE 1
#define LATCH 1
#if FUT_PROMISE
  std::promise<void> msg;
  std::shared_future<void> fut;
  std::jthread thr_, end_;
  void foo(std::promise<void> &&rdy) {
    for (size_t i = 0; i < 10000000; ++i) {
      std::lock_guard lk{mt_};
      x = x + 5e-7;
    }
    { rdy.set_value(); }
  }
  void bar(std::shared_future<void> bl) {
    bl.wait();
    cmp_.pending_pop();
  }
  LoadScene(scdc::SceneCompose &sc)
      : scdc::Scene(sc), msg(), fut(msg.get_future()),
        thr_([this](std::promise<void> &&pr) { foo(std::move(pr)); },
             std::move(msg)),
        end_([this](auto ftr) { bar(ftr); }, fut) {}
#elif CONDITION_VARIABLE
  std::jthread thr_, end_;
  std::condition_variable var;
  void foo() {
    for (size_t i = 0; i < 10000000; ++i) {
      std::lock_guard lk{mt_};
      x = x + 5e-7;
    }
    var.notify_all();
  }
  void bar() {
    {
      std::unique_lock lk{mt_};
      var.wait(lk);
      cmp_.pending_pop();
    }
  }
  LoadScene(scdc::SceneCompose &sc)
      : scdc::Scene(sc), thr_([this] { foo(); }), end_([this] { bar(); }) {}
#elif LATCH
  std::latch lt;
  std::jthread thr_, end_;

  void foo() {
    for (size_t i = 0; i < 10000000; ++i) {
      std::lock_guard lk{mt_};
      x = x + 5e-7;
    }
    lt.count_down();
  }
  
  void bar() {
    lt.wait();
    cmp_.pending_pop();
  }
#endif

  bool update(sf::Time dt) override {
    {
      std::lock_guard lk{mt_};
      circle.setRadius(100 + 50 * sin(x));
    }
    return false;
  }
  void draw(sf::RenderTarget &rt) override { ::draw(rt, circle); }
  bool handleEvent(const sf::Event &event) override { return true; }
  ~LoadScene() {}
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
      cmp_.pending_push<M>(x, y);
      break;
    case sf::Keyboard::L:
      cmp_.pending_push<LoadScene>();
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