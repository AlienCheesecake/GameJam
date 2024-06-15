#include "scene_compose.hh"

using namespace scdc;

SceneCompose::task_t SceneCompose::pop_task() {
  return [](auto &&sk) {
    sk.pop_back();
    return 0;
  };
}

SceneCompose::task_t SceneCompose::clear_task() {
  return [](std::vector<Scene::ptr> &sk) {
    sk.clear();
    return 0;
  };
}

void SceneCompose::consume_tasks() {
  while (!pending_task.empty()) {
    std::move(pending_task.front())(scenes_);
    pending_task.pop();
  }
}

void SceneCompose::pending_pop() { pending_task.push(pop_task()); }

void SceneCompose::pending_clear() { pending_task.push(clear_task()); }

void SceneCompose::handleEvent(sf::Event event) {
  for (auto i = scenes_.rbegin(), ei = scenes_.rend(); i != ei; ++i)
    if (!(*i)->handleEvent(event))
      break;
  consume_tasks();
}

void SceneCompose::draw() {
  for (auto &&i : scenes_)
    i->draw();
}

void SceneCompose::update(sf::Time dt) {
  for (auto i = scenes_.rbegin(), ei = scenes_.rend(); i != ei; ++i)
    if (!(*i)->update(dt))
      break;
  consume_tasks();
}

bool SceneCompose::empty() const noexcept { return scenes_.empty(); }


tmp_view::tmp_view(sf::RenderWindow &win, const sf::View &view) : win_(win) {
  win.setView(view);
}
tmp_view::~tmp_view() { win_.setView(win_.getDefaultView()); }