#include "mmedia/Animator.hh"
#include "mmedia/draw.hh"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <nlohmann/json.hpp>
#include "scdc/scene_compose.hh"
#include "mmedia/AssetManager.hh"
#include "mmedia/MusicPlayer.hh"

using namespace scdc;
using namespace mmed;

struct Mehehenu:Scene
{
  sf::RenderWindow &window;

  sf::Texture background1 = AssetManager::getTexture("images/fon1.png");
  sf::Texture background2 = AssetManager::getTexture("images/fon2.png");
  sf::Sprite bg;

  sf::Sprite button_start;
  sf::Texture start = AssetManager::getTexture("images/start.png");
  sf::Texture start_select = AssetManager::getTexture("images/start_select.png");
  sf::Texture start_final = AssetManager::getTexture("images/start_final.png");

  sf::Sprite button_exit;
  sf::Texture exit = AssetManager::getTexture("images/exit.png");
  sf::Texture exit_select = AssetManager::getTexture("images/exit_select.png");
  sf::Texture exit_final = AssetManager::getTexture("images/exit_final.png");

  MusicField mf{"audio/goofy.ogg"};
  Mehehenu(SceneCompose &cmp, sf::RenderWindow &win) : Scene(cmp), window(win)
  {
    button_start.setPosition(750, 200);
    button_exit.setPosition(790, 400);
    bg.setTexture(background2);
    button_start.setTexture(start);
    button_exit.setTexture(exit);
  }
  void draw() override 
  { 
    ::draw(window, bg); 
    ::draw(window, button_start);
    ::draw(window, button_exit);
  }
  bool update(sf::Time dt) override 
  {
    //ca_.update(dt);
    return false;
  }
  bool handleEvent(const sf::Event &event) override 
  { 
    sf::Vector2i pos = sf::Mouse::getPosition(window);
    sf::Vector2f mousepos = window.mapPixelToCoords(pos);
    if(button_start.getGlobalBounds().contains(mousepos.x, mousepos.y))
    {
      button_start.setTexture(start_select);
      if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
      {
        button_start.setTexture(start_final);
      }
    }
    else if(button_exit.getGlobalBounds().contains(mousepos.x, mousepos.y))
    {
      button_exit.setTexture(exit_select);
      if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
      {
        button_exit.setTexture(exit_final);
        cmp_.pending_pop();
      }
    }
    else
    {
      button_start.setTexture(start);
      button_exit.setTexture(exit);
    }
    return true; 
  }
};

struct Menu:Scene
{
  sf::RenderWindow &window;

  sf::Texture background1 = AssetManager::getTexture("images/fon1.png");
  sf::Texture background2 = AssetManager::getTexture("images/fon2.png");
  sf::Sprite bg;

  sf::Sprite button_start;
  sf::Texture start = AssetManager::getTexture("images/start.png");
  sf::Texture start_select = AssetManager::getTexture("images/start_select.png");
  sf::Texture start_final = AssetManager::getTexture("images/start_final.png");

  sf::Sprite button_exit;
  sf::Texture exit = AssetManager::getTexture("images/exit.png");
  sf::Texture exit_select = AssetManager::getTexture("images/exit_select.png");
  sf::Texture exit_final = AssetManager::getTexture("images/exit_final.png");

  MusicField mf{"audio/sleep.ogg"};
  Menu(SceneCompose &cmp, sf::RenderWindow &win) : Scene(cmp), window(win)
  {
    button_start.setPosition(750, 200);
    button_exit.setPosition(790, 400);
    bg.setTexture(background1);
    button_start.setTexture(start);
    button_exit.setTexture(exit);
  }
  void draw() override 
  { 
    ::draw(window, bg); 
    ::draw(window, button_start);
    ::draw(window, button_exit);
  }
  bool update(sf::Time dt) override 
  {
    //ca_.update(dt);
    return false;
  }
  bool handleEvent(const sf::Event &event) override 
  { 
    sf::Vector2i pos = sf::Mouse::getPosition(window);
    sf::Vector2f mousepos = window.mapPixelToCoords(pos);
    if(button_start.getGlobalBounds().contains(mousepos.x, mousepos.y))
    {
      button_start.setTexture(start_select);
      if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
      {
        bg.setTexture(background2);
        button_start.setTexture(start_final);
        cmp_.pending_pop();
        cmp_.pending_push<Mehehenu>(window);
      }
    }
    else if(button_exit.getGlobalBounds().contains(mousepos.x, mousepos.y))
    {
      button_exit.setTexture(exit_select);
      if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
      {
        button_exit.setTexture(exit_final);
        cmp_.pending_pop();
      }
    }
    else
    {
      button_start.setTexture(start);
      button_exit.setTexture(exit);
    }
    return true; 
  }
};

int main() {
  auto window = sf::RenderWindow{sf::VideoMode(1920, 1080), "Test Manager",
                                 sf::Style::Titlebar | sf::Style::Close};
  sf::Clock clock;
  SceneCompose scmp = SceneCompose();
  scmp.pending_push<Menu>(window);

  while (window.isOpen()) {
    auto dt = clock.restart();

    sf::Event event;
    while (window.pollEvent(event)) 
    {
      scmp.handleEvent(event);
      if (event.type == sf::Event::Closed)
        window.close();
    }
    scmp.update(dt);
    if(scmp.empty())
    {window.close();}
    window.clear();
    scmp.draw();
    window.display();
  }
  return 0;
}