#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

class AssetManager final {
  std::map<std::string, sf::Texture> m_Textures;
  std::map<std::string, sf::SoundBuffer> m_SoundBuffers;
  std::map<std::string, sf::Font> m_Fonts;

private:
  AssetManager() = default;

public:
  AssetManager(const AssetManager &other) = delete;
  void operator=(const AssetManager &) = delete;

  static AssetManager &getInstance();

  static sf::Texture &getTexture(std::string const &filename,
                                 const sf::Color &clr = {0, 255, 0});
  static sf::SoundBuffer &getSoundBuffer(std::string const &filename);
  static sf::Font &getFont(std::string const &filename);
};