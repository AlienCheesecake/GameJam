#pragma once
#include "fire_once.hh"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <queue>
#include <tuple>
#include <vector>

namespace scdc {
struct SceneCompose;

struct Scene {
  using ptr = std::unique_ptr<Scene>;

  SceneCompose &cmp_;
  Scene(SceneCompose &cmp) : cmp_(cmp) {}

  virtual void draw() = 0;
  virtual bool update(sf::Time dt) = 0;
  virtual bool handleEvent(const sf::Event &event) = 0;

  virtual ~Scene() = default;
};

template <typename T>
concept scene_child = std::derived_from<T, Scene>;

class SceneCompose {
  using task_t = fire_once<int(std::vector<Scene::ptr> &)>;
  std::vector<Scene::ptr> scenes_;

  template <scene_child T, typename... Args> task_t push_task(Args &&...args) {
    return [this, args = std::forward_as_tuple(args...)](
               std::vector<Scene::ptr> &sk) mutable {
      sk.push_back(std::apply(
          [this](auto &&...args) {
            return create_scene<T>(std::forward<Args>(args)...);
          },
          args));
      return 0;
    };
  }
  task_t pop_task();

  task_t clear_task();
  std::queue<task_t> pending_task;

  template <scene_child T, typename... Args>
  Scene::ptr create_scene(Args &&...args) {
    return Scene::ptr{new T(*this, std::forward<Args>(args)...)};
  }

public:
  bool empty() const noexcept;
  void consume_tasks();

  template <scene_child T, typename... Args>
  inline void pending_push(Args &&...args) {
    pending_task.push(push_task<T>(std::forward<Args>(args)...));
  }
  void pending_pop();
  void pending_clear();

  void handleEvent(sf::Event event);
  void draw();
  void update(sf::Time dt);
};
struct tmp_view final {
  sf::RenderWindow &win_;
  tmp_view(sf::RenderWindow &win, const sf::View &view);
  ~tmp_view();
};
} // namespace scdc