#include "mmedia/Animator.hh"
#include "mmedia/draw.hh"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <iterator>
#include <nlohmann/json.hpp>


int main() {
  auto &&aniM = mmed::AnimationManager::getInstance();
  aniM.loadFile("animations.json");
  mmed::CharacterAnimation character{
      {mmed::AnimationManager::getAnimation("na_l"),
       mmed::AnimationManager::getAnimation("na_2"),
       mmed::AnimationManager::getAnimation("na_3")},
      {}};
  character.select_anim("break_dance");
  character.restart();
  {
    auto [x, y] = character.getAnimation().frame_size / 2;
    character.sp_.setOrigin(x, 0);
  }
  // character.sp_.setPosition(100, 100);

  auto window = sf::RenderWindow{sf::VideoMode(204, 204), "Test Manager",
                                 sf::Style::Titlebar | sf::Style::Close};
  sf::Clock clock;

  while (window.isOpen()) {
    auto dt = clock.restart();

    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
        case sf::Keyboard::B:
          character.select_anim("break_dance");
          break;
        case sf::Keyboard::D:
          character.select_anim("dance");
          break;
        case sf::Keyboard::E:
          character.select_anim("dissapear");
          break;
        case sf::Keyboard::F:
          character.sp_.setScale(-1, 1);
          break;
        case sf::Keyboard::U:
          character.sp_.setScale(1, 1);
          break;
        case sf::Keyboard::M:
          character.sp_.move(2, 0);
          break;
        case sf::Keyboard::N:
          character.sp_.move(-2, 0);
        default:
          break;
        }
        character.restart();
      }
      if (event.type == sf::Event::Closed)
        window.close();
    }
    character.update(dt);
    window.clear();
    ::draw(window, character);
    window.display();
  }
  return 0;
}