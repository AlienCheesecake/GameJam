#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <iostream>

template <typename T> void draw(sf::RenderTarget &trgt, T &&drawable) {
  trgt.draw(std::forward<T>(drawable));
}

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

struct Animation {
  std::string name, texture_path;
  uint64_t total_frame, columns;
  sf::Vector2i frame_size;
  sf::Time duration_;
  sf::Color mask_color;
  sf::IntRect get_box(uint64_t cur_frame) const;
  bool loop;
};

sf::IntRect Animation::get_box(uint64_t cur_frame) const {
  if (total_frame <= cur_frame)
    throw std::out_of_range("total_frame <= cur_frame");
  return {sf::Vector2i(static_cast<int>(cur_frame % columns) * frame_size.x,
                       static_cast<int>(cur_frame / columns) * frame_size.y),
          frame_size};
}

class AnimationManager final {
  std::map<std::string, Animation> m_Animations;
  AnimationManager() = default;

public:
  AnimationManager(AnimationManager &) = delete;
  void operator=(const AnimationManager &) = delete;

  static AnimationManager &getInstance();
  void loadFile(std::string_view file_name = "media/animations.json");
  static Animation &getAnimation(const std::string &id_name);
};

AnimationManager &AnimationManager::getInstance() {
  static AnimationManager am;
  return am;
}

void AnimationManager::loadFile(std::string_view file_name) {
  nlohmann::json j;
  auto fin = std::ifstream("animations.json");
  fin >> j;
  for (auto &&i : j.at("objects")) {
    auto id_name = i.at("id_name").get<std::string>();
    auto [w, h] = i.at("frame_size").get<std::array<int, 2>>();
    auto [r, g, b, a] = i.at("mask_color").get<std::array<uint8_t, 4>>();
    m_Animations.emplace(
        id_name, Animation{i.at("name").get<std::string>(),
                           i.at("texture_path").get<std::string>(),
                           i.at("total_frame").get<uint64_t>(),
                           i.at("columns").get<uint64_t>(), sf::Vector2i{w, h},
                           sf::seconds(i.at("duration").get<float>()),
                           sf::Color(r, g, b, a), i.at("isLoop").get<bool>()});
  }
}

Animation &AnimationManager::getAnimation(const std::string &id_name) {
  auto &&am = AnimationManager::getInstance();
  return am.m_Animations.at(id_name);
}

class Animator {
  Animation anim_;
  bool end_anim = true;
  sf::Time time;

public:
  sf::Sprite &sp_;
  Animator(sf::Sprite &sp, Animation anim = {}) : sp_(sp), anim_(anim) {
    if (!anim.texture_path.empty())
      sp_.setTexture(AssetManager::getTexture(anim.texture_path));
  }

  void switchAnimation(Animation anim) {
    anim_ = anim;
    if (!anim.texture_path.empty())
      sp_.setTexture(AssetManager::getTexture(anim.texture_path));
  }
  void update(sf::Time dt) {
    if (anim_.texture_path.empty()) return;
    if (!end_anim) time += dt;
    float scaledTime = (time.asSeconds() / anim_.duration_.asSeconds());
    uint64_t cur_frame = scaledTime * anim_.total_frame;
    if (anim_.loop) cur_frame %= anim_.total_frame;
    else if (cur_frame >= anim_.total_frame) {
        cur_frame = anim_.total_frame - 1;
        end_anim = true;
    }
    sp_.setTextureRect(anim_.get_box(cur_frame));

  }
  void restart() {
    time = sf::Time::Zero;
    end_anim = false;
  }
  void stop() {
    end_anim = true;
  }
};

template <> void draw<Animator &>(sf::RenderTarget &trgt, Animator &anim) {
  draw(trgt, anim.sp_);
}

int main() {
  auto &&aniM = AnimationManager::getInstance();
  aniM.loadFile("animations.json");
  sf::Sprite sprite;
  // sprite.setTexture(AssetManager::getTexture("images/na.png"));
  // sprite.setTextureRect({0, 0, 204, 168});
  Animator ar{sprite, AnimationManager::getAnimation("na_l")};

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
    window.draw(sprite);
    window.display();
  }
  return 0;
}