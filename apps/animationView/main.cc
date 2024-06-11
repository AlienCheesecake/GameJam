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
#include <nlohmann/json.hpp>
#include <stdexcept>

struct CharacterAnimation : private Animator {
  std::vector<Animation> anims_;
  CharacterAnimation(const std::vector<Animation> &anims, const sf::Sprite &sp)
      : Animator(sp, {}), anims_(anims) {}

  void select_anim(std::string_view name) {
    auto fnd = std::find_if(anims_.begin(), anims_.end(),
                            [name](auto &&i) { return name == i.name; });
    if (fnd == anims_.end())
      throw std::invalid_argument("Animation not found");
    switchAnimation(*fnd);
  }
  using Animator::restart;
  using Animator::sp_;
  using Animator::stop;
  using Animator::update;
};

template <>
void draw<CharacterAnimation &>(sf::RenderTarget &rd,
                                CharacterAnimation &char_anim) {
  rd.draw(char_anim.sp_);
}

int main() {
  auto &&aniM = AnimationManager::getInstance();
  aniM.loadFile("animations.json");
  CharacterAnimation character{{AnimationManager::getAnimation("na_l"),
                                AnimationManager::getAnimation("na_2")},
                               {}};
  character.select_anim("break_dance");

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
    draw(window, character);
    window.display();
  }
  return 0;
}