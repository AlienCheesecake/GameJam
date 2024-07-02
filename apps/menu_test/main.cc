#include "mmedia/Animator.hh"
#include "mmedia/draw.hh"

#include "scdc/scene_compose.hh"

#include <SFML/Graphics/Glsl.hpp>
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <thread>

struct Foo {
  int x = 0;
  Foo() { std::cout << "Foo()" << std::endl; }

  Foo(const Foo &f) : x(f.x) { std::cout << "Foo(c&)" << std::endl; }

  Foo(Foo &&f) : x(f.x) { std::cout << "Foo(&&)" << std::endl; }

  Foo &operator=(const Foo &f) {
    x = f.x;
    std::cout << "Foo=(c&)" << std::endl;
    return *this;
  }

  Foo &operator=(Foo &&f) {
    x = f.x;
    std::cout << "Foo=(&&)" << std::endl;
    return *this;
  }

  ~Foo() { std::cout << "~Foo()" << std::endl; }
};

struct LoadScene : scdc::Scene {
  LoadScene(scdc::SceneCompose &compositor, sf::RenderTarget &render_window,
            mmed::AnimationManager anim_manager,
            mmed::AnimationManager &load_target, const std::string &file_name)
      : scdc::Scene(compositor), render_window_(render_window),
        anim_manager_(std::move(anim_manager)), load_target_(load_target),
        animation_bar_({anim_manager_.getAnimation("na_l")}, {}) {
    animation_bar_.select_anim("load");
    animation_bar_.restart();

    std::packaged_task<mmed::AnimationManager(const std::string &)>
        load_am_task{[](const std::string &file_name) {
          mmed::AnimationManager am_result;
          am_result.loadFile(file_name);
          std::this_thread::sleep_for(std::chrono::seconds{2});
          return am_result;
        }};
    loaded_am_future = load_am_task.get_future();
    std::thread am_loader{std::move(load_am_task), file_name};
    am_loader.detach();
  }

  bool update(sf::Time dt) override {
    auto status = loaded_am_future.wait_for(std::chrono::milliseconds(0));;
    if (status == std::future_status::ready) {
      auto &&tmp = loaded_am_future.get();
      std::swap(load_target_, tmp);
      compositor_.pending_pop();
    }
    animation_bar_.update(dt);
    return false;
  }

  void draw() override { ::draw(render_window_, animation_bar_); }

  bool handleEvent(const sf::Event &event) override { return false; }

private:
  sf::RenderTarget &render_window_;
  mmed::AnimationManager anim_manager_, &load_target_;
  std::future<mmed::AnimationManager> loaded_am_future;
  mmed::CharacterAnimation animation_bar_;
};

struct Scene1 : scdc::Scene {};

struct StartScene : scdc::Scene {
  sf::RenderTarget &render_window_;
  mmed::AnimationManager anim_manager_;

  StartScene(scdc::SceneCompose &compositor, sf::RenderTarget &render_window)
      : scdc::Scene(compositor), render_window_(render_window) {
    mmed::AnimationManager tmp_am;
    tmp_am.loadFile("basic_anims.json");
    compositor_.pending_push<LoadScene>(
        std::ref(render_window), std::move(tmp_am), std::ref(anim_manager_),
        "animations.json");
    std::cout << "Foo" << std::endl;
  }

  void draw() override {}

  bool update(sf::Time dt) override {
    anim_manager_.getAnimation("na_l");
    return false;
  }

  bool handleEvent(const sf::Event &event) override { return false; }
};

int main() {
  auto window = sf::RenderWindow{sf::VideoMode(200, 300), "Test Manager",
                                 sf::Style::Titlebar | sf::Style::Close};
  sf::Clock clock;
  scdc::SceneCompose compositor;

  compositor.pending_push<StartScene>(std::ref(window));

  while (window.isOpen()) {
    auto dt = clock.restart();

    sf::Event event;
    while (window.pollEvent(event)) {
      compositor.handleEvent(event);
      if (event.type == sf::Event::Closed)
        window.close();
    }

    compositor.update(dt);
    if (compositor.empty())
      window.close();
    window.clear();
    compositor.draw();
    window.display();
  }
  return 0;
}