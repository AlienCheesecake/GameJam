#include "mmedia/Animator.hh"
#include "mmedia/draw.hh"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Context.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <concepts>
#include <iostream>
#include <memory>
#include <queue>
#include <type_traits>
#include <random>

template <typename T> class fire_once;

template <typename R, typename... Args> class fire_once<R(Args...)> {
  std::unique_ptr<void, void (*)(void *)> ptr{nullptr, +[](void *) {}};
  R (*invoke)(void *, Args...) = nullptr;

public:
  fire_once() = default;
  fire_once(fire_once &&) = default;
  fire_once &operator=(fire_once &&) = default;

  template <typename F> fire_once(F &&f) {
    auto pf = std::make_unique<F>(std::move(f));
    invoke = +[](void *pf, Args &&...args) -> R {
      F *f = reinterpret_cast<F *>(pf);
      return (*f)(std::forward<Args>(args)...);
    };
    ptr = {pf.release(), [](void *pf) {
             F *f = reinterpret_cast<F *>(pf);
             delete f;
           }};
  }

  R operator()(Args &&...args) && {
    R ret = invoke(ptr.get(), std::forward<Args>(args)...);
    clear();
    return std::move(ret);
  }

  void clear() {
    invoke = nullptr;
    ptr.reset();
  }

  explicit operator bool() const { return static_cast<bool>(ptr); }
};

struct SceneCompose;

struct Scene {
  using ptr = std::unique_ptr<Scene>;

  SceneCompose &cmp_;
  Scene(SceneCompose &cmp) : cmp_(cmp) {}

  virtual void draw(sf::RenderTarget &) = 0;
  virtual bool update(sf::Time dt) = 0;
  virtual bool handleEvent(const sf::Event &event) = 0;

  virtual ~Scene() = default;
};

template <typename T>
concept scene_child = std::derived_from<T, Scene>;

struct SceneCompose {
  std::vector<Scene::ptr> scenes_;
  using task_t = fire_once<int(std::vector<Scene::ptr> &)>;
  template <scene_child T> task_t push_task() {
    return [this](std::vector<Scene::ptr> &sk) {
      sk.push_back(create_scene<T>());
      return 0;
    };
  }
  task_t pop_task() {
    return [](auto &&sk) {
      sk.pop_back();
      return 0;
    };
  }
  task_t clear_task() {
    return [](std::vector<Scene::ptr> &sk) {
      sk.clear();
      return 0;
    };
  }
  std::queue<task_t> pending_task;

  void consume_tasks() {
    while (!pending_task.empty()) {
      std::move(pending_task.front())(scenes_);
      pending_task.pop();
    }
  }

  template <scene_child T> inline void pending_push() {
    pending_task.push(push_task<T>());
  }

  inline void pending_pop() { pending_task.push(pop_task()); }

  inline void pending_clear() { pending_task.push(clear_task()); }

  template <scene_child T> Scene::ptr create_scene() {
    return Scene::ptr{new T(*this)};
  }

  void handleEvent(sf::Event event) {
    for (auto i = scenes_.rbegin(), ei = scenes_.rend(); i != ei; ++i)
      if (!(*i)->handleEvent(event))
        break;
    consume_tasks();
  }

  void draw(sf::RenderTarget &rnd) {
    for (auto &&i : scenes_)
      i->draw(rnd);
  }

  void update(sf::Time dt) {
    for (auto i = scenes_.rbegin(), ei = scenes_.rend(); i != ei; ++i)
      if (!(*i)->update(dt))
        break;
    consume_tasks();
  }
};

struct A : Scene {
  A(SceneCompose &cmp) : Scene(cmp) { std::cout << "A\n"; }
  void draw(sf::RenderTarget &) override {}
  bool update(sf::Time dt) override { return false; }

  bool handleEvent(const sf::Event &event) override;
  ~A() { std::cout << "~A\n"; }
};

std::default_random_engine dre{std::random_device{}()};
std::uniform_real_distribution<float> uid{0, 200};

struct B : Scene {
  CharacterAnimation ca_;
  sf::CircleShape shape{50};
  B(SceneCompose &cmp)
      : Scene(cmp), ca_({AnimationManager::getAnimation("na_l")}, {}) {
    ca_.sp_.setPosition({uid(dre), uid(dre)});
    ca_.select_anim("dance");
    ca_.restart();
    std::cout << "B\n";
  }
  void draw(sf::RenderTarget &rt) override {
    // ::draw(rt, shape);
    ::draw(rt, ca_);
    // rt.draw(ca_.sp_);
  }
  bool update(sf::Time dt) override { 
    ca_.update(dt);
    return false;
  }
  bool handleEvent(const sf::Event &event) override { return true; }
  ~B() { std::cout << "~B\n"; }
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
    default:
      break;
    }
  return false;
}

int main() try {
  auto &&aniM = AnimationManager::getInstance();
  aniM.loadFile("animations.json");
  auto scmp = SceneCompose{};
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
    if (scmp.scenes_.empty())
      window.close();
    window.clear();
    scmp.draw(window);
    window.display();
  }
  return 0;
} catch (std::exception &err) {
  std::cerr << "Catched exception: " << err.what() << std::endl; 
} catch (...) {
  std::cerr << "Catched unknown exception" << std::endl;
}