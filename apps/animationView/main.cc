#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <nlohmann/json.hpp>
#include "mmedia/Animator.hh"
#include "mmedia/draw.hh"

int main() {
  auto &&aniM = AnimationManager::getInstance();
  aniM.loadFile("animations.json");
  Animator ar{{}, AnimationManager::getAnimation("na_l")};

  auto window = sf::RenderWindow{sf::VideoMode(204, 204), "Test Manager",
                                 sf::Style::Titlebar | sf::Style::Close};
  sf::Clock clock;

  while (window.isOpen()) {
    auto dt = clock.restart();

    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::KeyPressed)
        ar.restart();
      if (event.type == sf::Event::Closed)
        window.close();
    }
    ar.update(dt);
    window.clear();
    draw(window, ar);
    window.display();
  }
  return 0;
}