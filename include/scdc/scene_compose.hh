#pragma once
#include "fire_once.hh"
#include <SFML/Graphics.hpp>
#include <queue>
#include <vector>

namespace scdc {
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

class SceneCompose {
  using task_t = fire_once<int(std::vector<Scene::ptr> &)>;
  std::vector<Scene::ptr> scenes_;

  template <scene_child T> task_t push_task() {
    return [this](std::vector<Scene::ptr> &sk) {
      sk.push_back(create_scene<T>());
      return 0;
    };
  }
  task_t pop_task();

  task_t clear_task();
  std::queue<task_t> pending_task;

  template <scene_child T> Scene::ptr create_scene() {
    return Scene::ptr{new T(*this)};
  }

public:
  bool empty() const noexcept;
  void consume_tasks();

  template <scene_child T> inline void pending_push() {
    pending_task.push(push_task<T>());
  }
  void pending_pop();
  void pending_clear();

  void handleEvent(sf::Event event);
  void draw(sf::RenderTarget &rnd);
  void update(sf::Time dt);
};
} // namespace scdc