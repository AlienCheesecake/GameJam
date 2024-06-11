#include "draw.hh"
#include "Animator.hh"

template <> void draw<Animator &>(sf::RenderTarget &trgt, Animator &anim) {
  draw(trgt, anim.sp_);
}

template <>
void draw<CharacterAnimation &>(sf::RenderTarget &rd,
                                CharacterAnimation &char_anim) {
  rd.draw(char_anim.sp_);
}