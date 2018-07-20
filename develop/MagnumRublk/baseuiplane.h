#ifndef BASEUIPLANE_H
#define BASEUIPLANE_H

// --

using namespace Magnum;

struct BaseUiPlane: Ui::Plane
{
  explicit BaseUiPlane(Ui::UserInterface& ui);

  Ui::Label _fps;
};

#endif // BASEUIPLANE_H
