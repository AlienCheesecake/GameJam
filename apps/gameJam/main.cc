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

class Menu
{
public:
sf::Sprite backgraund;
float menu_X;
float menu_Y;
int menu_step;
//int menu_selected;

CharacterAnimation button_start{{AnimationManager::getAnimation("na_l"), AnimationManager::getAnimation("na_2"),
                                  AnimationManager::getAnimation("na_3")},
                                  {}};
CharacterAnimation button_level{{AnimationManager::getAnimation("na_l"), AnimationManager::getAnimation("na_2"),
                                  AnimationManager::getAnimation("na_3")},
                                  {}};
CharacterAnimation button_exit{{AnimationManager::getAnimation("na_l"), AnimationManager::getAnimation("na_2"),
                                  AnimationManager::getAnimation("na_3")},
                                  {}};
public:
Menu(float x, float y, int step)
{
    menu_X = x;
    menu_Y = y;
    menu_step = step;
    button_start.sp_.setPosition(sf::Vector2f(menu_X, menu_Y));
    button_level.sp_.setPosition(sf::Vector2f(menu_X, menu_Y+menu_step));
    button_exit.sp_.setPosition(sf::Vector2f(menu_X, menu_Y+2*menu_step));
}
};

int main() 
{
    auto &&aniM = AnimationManager::getInstance();
    aniM.loadFile("animations.json");
    CharacterAnimation character{{AnimationManager::getAnimation("na_l"), AnimationManager::getAnimation("na_2"),
                                  AnimationManager::getAnimation("na_3")},
                                  {}};
    

    sf::RenderWindow window = sf::RenderWindow{sf::VideoMode(1920, 1080), "Test Manager",
                                 sf::Style::Titlebar | sf::Style::Close};
    sf::Clock clock;
    Menu menu{700, 150, 300};
    menu.button_start.select_anim("dance");
    menu.button_level.select_anim("dance");
    menu.button_exit.select_anim("dance");
    while (window.isOpen()) 
    {
        auto dt = clock.restart();
        sf::Event event;
        sf::Vector2i pos = sf::Mouse::getPosition(window);
        sf::Vector2f mousepos = window.mapPixelToCoords(pos);
        while (window.pollEvent(event))
        {
            if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                if(menu.button_start.sp_.getGlobalBounds().contains(mousepos.x, mousepos.y))
                {
                    menu.button_start.select_anim("break_dance");
                    menu.button_start.restart();
                }
                if(menu.button_level.sp_.getGlobalBounds().contains(mousepos.x, mousepos.y))
                {
                    menu.button_level.select_anim("break_dance");
                    menu.button_level.restart();
                }
                if(menu.button_exit.sp_.getGlobalBounds().contains(mousepos.x, mousepos.y))
                {
                    menu.button_exit.select_anim("break_dance");
                    menu.button_exit.restart();
                }
            }
            if (event.type == sf::Event::Closed)
                            window.close();
        }
            menu.button_exit.update(dt);
            menu.button_start.update(dt);
            menu.button_level.update(dt);
            window.clear();
            draw(window, menu.button_start);
            draw(window, menu.button_level);
            draw(window, menu.button_exit);
            window.display();
    }

}