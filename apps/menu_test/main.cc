#include "gmui/button.hh"
#include "mmedia/Animator.hh"
#include "mmedia/draw.hh"
#include "scdc/scene_compose.hh"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Vector2.hpp>
#include <functional>
#include <iostream>
#include "gmui/dd.hh"

using namespace mmed::gmui;

struct MenuScene : scdc::Scene {
  sf::View view;
  std::vector<mmed::Animation> button_anim = {
      mmed::AnimationManager::getAnimation("b1_0"),
      mmed::AnimationManager::getAnimation("b1_1"),
      mmed::AnimationManager::getAnimation("b1_2"),
      mmed::AnimationManager::getAnimation("b1_d"),
  };
  sf::RenderWindow &win_;
  struct tmp_view {
    sf::RenderWindow &win_;
    tmp_view(sf::RenderWindow &win, const sf::View &view) : win_(win) {
      win.setView(view);
    }
    ~tmp_view() { win_.setView(win_.getDefaultView()); }
  };

  Button btn;
  DD dd;
  MenuScene(scdc::SceneCompose &sc, sf::RenderWindow &win)
      : scdc::Scene(sc), win_(win),
        btn{[] { std::cout << "Foo" << std::endl; },
            [] { std::cout << "Bar" << std::endl; },
            mmed::CharacterAnimation(button_anim, {}),
            sf::RectangleShape{{100, 50}}},
        dd(
            mmed::CharacterAnimation{button_anim, {}},
            sf::RectangleShape{{100, 50}},
            mmed::CharacterAnimation{
                {mmed::Animation::one_frame_anim("follow", "images/start.png")},
                {}},
            [] { std::cout << "Foo\n"; }, [] { std::cout << "Bar\n"; }) {
    dd.follow_animation().sp_.setScale(.5, .5);
    dd.btn_.setScale(3, 3);
    view.setViewport({.25, .25, .5, .5});
    btn.move(120, 100);
    btn.rotate(-30);
    btn.scale({2, 2});
  }

  void draw() override {
    {
      tmp_view tv(win_, view);
      ::draw(win_, btn);
      ::draw(win_, dd);
    }
    dd.follow_draw(win_, sf::RenderStates::Default);
  }
  sf::Vector2f world_pos() {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(win_);
    return win_.mapPixelToCoords(pixelPos);
  }
  bool update(sf::Time dt) override {
    {
      tmp_view tv(win_, view);
      btn.update(dt, world_pos());
      dd.update(dt, world_pos());
    }
    dd.follow_update(dt, world_pos());
    return true;
  }
  bool handleEvent(const sf::Event &event) override {
    tmp_view tv(win_, view);
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
