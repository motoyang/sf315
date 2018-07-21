#ifndef BASEUIPLANE_H
#define BASEUIPLANE_H

// --

using namespace Magnum;

struct BaseUiPlane: Ui::Plane
{
  explicit BaseUiPlane(Ui::UserInterface& ui);

  Ui::Label _fps;
  Ui::ValidatedInput _degrees;
  Ui::Button _D, _d,  _U, _u, _R, _r, _L, _l, _B, _b, _F, _f, _apply;
};

#endif // BASEUIPLANE_H
