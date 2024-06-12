#include "draw.hh"
#include "Animator.hh"

template <> void draw<mmed::Animator &>(sf::RenderTarget &trgt, mmed::Animator &anim) {
  draw(trgt, anim.sp_);
}

template <>
void draw<mmed::CharacterAnimation &>(sf::RenderTarget &rd,
                                mmed::CharacterAnimation &char_anim) {
  rd.draw(char_anim.sp_);
}