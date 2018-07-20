#include <Magnum/Ui/Anchor.h>
#include <Magnum/Ui/Button.h>
#include <Magnum/Ui/Label.h>
#include <Magnum/Ui/Plane.h>
#include <Magnum/Ui/UserInterface.h>
#include <Magnum/Ui/ValidatedInput.h>
#include <Magnum/Text/Alignment.h>

#include "baseuiplane.h"

// --

constexpr Vector2 WidgetSize{256, 64};
//const std::regex FloatValidator{R"(-?\d+(\.\d+)?)"};

// --

BaseUiPlane::BaseUiPlane(Ui::UserInterface& ui)
  : Ui::Plane{ui, Ui::Snap::Top|Ui::Snap::Bottom|Ui::Snap::Left|Ui::Snap::Right, 0, 16, 128}
  , _fps{*this, {Ui::Snap::Top | Ui::Snap::Left, WidgetSize}, "FPS: 59.8", Text::Alignment::TopLeft}
{
  Ui::Label{*this, {Ui::Snap::Bottom|Ui::Snap::Left, WidgetSize},
            "Keys f/F, b/B, u/U, d/D to roll.\nSpace key for redirection.\n"
            "Enter key to origin viewport.\nPress mouse to move.",
            Text::Alignment::MiddleLeft};
}
