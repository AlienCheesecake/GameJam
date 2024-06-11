#include "AssetManager.hh"

AssetManager &AssetManager::getInstance() {
  static AssetManager mng;
  return mng;
}

sf::Texture &AssetManager::getTexture(std::string const &filename,
                                      const sf::Color &clr) {
  auto &&am = AssetManager::getInstance();
  auto pairFound = am.m_Textures.find(filename);
  if (pairFound != am.m_Textures.end())
    return pairFound->second;
  else {
    auto &texture = am.m_Textures[filename];
    sf::Image img;
    img.loadFromFile(filename);
    img.createMaskFromColor(clr);
    texture.loadFromImage(img);
    return texture;
  }
}

sf::SoundBuffer &AssetManager::getSoundBuffer(const std::string &filename) {
  auto &&am = AssetManager::getInstance();
  auto pairFound = am.m_SoundBuffers.find(filename);
  if (pairFound != am.m_SoundBuffers.end())
    return pairFound->second;
  else {
    auto &sb = am.m_SoundBuffers[filename];
    sb.loadFromFile(filename);
    return sb;
  }
}

sf::Font &AssetManager::getFont(const std::string &filename) {
  auto &&am = AssetManager::getInstance();
  auto pairFound = am.m_Fonts.find(filename);
  if (pairFound != am.m_Fonts.end())
    return pairFound->second;
  else {
    auto &font = am.m_Fonts[filename];
    font.loadFromFile(filename);
    return font;
  }
}