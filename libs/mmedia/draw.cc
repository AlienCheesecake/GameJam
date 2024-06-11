#include "draw.hh"
#include "Animator.hh"

template <> void draw<Animator &>(sf::RenderTarget &trgt, Animator &anim) {
  draw(trgt, anim.sp_);
}