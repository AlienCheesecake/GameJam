#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Context.hpp>
#include <concepts>
#include <iostream>
#include <memory>
#include <queue>
#include <type_traits>

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
  sf::RenderWindow win_;
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
    for (auto i = scenes_.rbegin(), ei = scenes_.rend(); i != ei; ++i)
      (*i)->draw(rnd);
  }

  void update(sf::Time dt) {
    for (auto i = scenes_.rbegin(), ei = scenes_.rend(); i != ei; ++i)
      if (!(*i)->update(dt))
        break;
    consume_tasks();
  }
};

template <> void draw<Scene &>(sf::RenderTarget &rt, Scene &sc) { sc.draw(rt); }

struct A : Scene {
  A(SceneCompose &cmp) : Scene(cmp) { std::cout << "A\n"; }
  void draw(sf::RenderTarget &) override {}
  bool update(sf::Time dt) override { return false; }
  bool handleEvent(const sf::Event &event) override { return false; }
  ~A() {std::cout << "~A\n";}
};

struct B : Scene {
  B(SceneCompose &cmp) : Scene(cmp) { std::cout << "B\n"; }
  void draw(sf::RenderTarget &) override {}
  bool update(sf::Time dt) override { return false; }
  bool handleEvent(const sf::Event &event) override { return false; }
  ~B() {std::cout << "~B\n";}
};

int main() {
  SceneCompose cmp;
  cmp.pending_task.push(cmp.push_task<A>());
  cmp.pending_task.push(cmp.push_task<B>());
  cmp.pending_task.push(cmp.pop_task());
  cmp.pending_task.push(cmp.push_task<B>());
  cmp.pending_task.push(cmp.clear_task());
  cmp.pending_task.push(cmp.push_task<A>());
  cmp.update({});
  return 0;
}