#include "Animator.hh"
#include "AssetManager.hh"
#include <SFML/Graphics.hpp>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace mmed;

sf::IntRect Animation::get_box(uint64_t cur_frame) const {
  if (total_frame <= cur_frame)
    throw std::out_of_range("total_frame <= cur_frame");
  return {sf::Vector2i(static_cast<int>(cur_frame % columns) * frame_size.x,
                       static_cast<int>(cur_frame / columns) * frame_size.y),
          frame_size};
}

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

Animator::Animator(const sf::Sprite &sp, const Animation &anim)
    : sp_(sp), anim_(anim) {
  if (!anim.texture_path.empty())
    sp_.setTexture(AssetManager::getTexture(anim.texture_path));
}

void Animator::switchAnimation(const Animation &anim) {
  anim_ = anim;
  if (!anim.texture_path.empty())
    sp_.setTexture(AssetManager::getTexture(anim.texture_path));
}
void Animator::update(sf::Time dt) {
  if (anim_.texture_path.empty())
    return;
  if (!end_anim)
    time += dt;
  float scaledTime = (time.asSeconds() / anim_.duration_.asSeconds());
  uint64_t cur_frame = scaledTime * anim_.total_frame;
  if (anim_.loop)
    cur_frame %= anim_.total_frame;
  else if (cur_frame >= anim_.total_frame) {
    cur_frame = anim_.total_frame - 1;
    end_anim = true;
  }
  sp_.setTextureRect(anim_.get_box(cur_frame));
}
void Animator::restart() {
  time = sf::Time::Zero;
  end_anim = false;
}
void Animator::stop() { end_anim = true; }

CharacterAnimation::CharacterAnimation(const std::vector<Animation> &anims,
                                       const sf::Sprite &sp)
    : Animator(sp, {}), anims_(anims) {}

void CharacterAnimation::select_anim(std::string_view name) {
  auto fnd = std::find_if(anims_.begin(), anims_.end(),
                          [name](auto &&i) { return name == i.name; });
  if (fnd == anims_.end())
    throw std::invalid_argument("Animation not found");
  switchAnimation(*fnd);
}