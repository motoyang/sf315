#include <Magnum/Ui/Anchor.h>
#include <Magnum/Ui/Button.h>
#include <Magnum/Ui/Label.h>
#include <Magnum/Ui/Plane.h>
#include <Magnum/Ui/UserInterface.h>
#include <Magnum/Ui/ValidatedInput.h>
#include <Magnum/Text/Alignment.h>

#include "baseuiplane.h"

// --

constexpr Vector2 RollButtonSize{32, 16};
const std::regex FloatValidator{R"(\d+)"};

// --

BaseUiPlane::BaseUiPlane(Ui::UserInterface& ui)
  : Ui::Plane{ui, Ui::Snap::Top|Ui::Snap::Bottom|Ui::Snap::Left|Ui::Snap::Right, 0, 32, 256}
  , _fps{*this, {Ui::Snap::Top | Ui::Snap::Left, Vector2{64, 24}}, "FPS: 0    ", Text::Alignment::MiddleLeft}
  , _degrees{*this, {Ui::Snap::Top | Ui::Snap::Right, Vector2{64, 24}}, FloatValidator, "360", 3}
  , _D{*this, {Ui::Snap::Bottom | Ui::Snap::Right, RollButtonSize}, "D", Ui::Style::Default}
  , _d{*this, {Ui::Snap::Top, _D, RollButtonSize}, "d", Ui::Style::Default}
  , _U{*this, {Ui::Snap::Left, _D, RollButtonSize}, "U", Ui::Style::Default}
  , _u{*this, {Ui::Snap::Top, _U, RollButtonSize}, "u", Ui::Style::Default}
  , _R{*this, {Ui::Snap::Left, _U, RollButtonSize}, "R", Ui::Style::Default}
  , _r{*this, {Ui::Snap::Top, _R, RollButtonSize}, "r", Ui::Style::Default}
  , _L{*this, {Ui::Snap::Left, _R, RollButtonSize}, "L", Ui::Style::Default}
  , _l{*this, {Ui::Snap::Top, _L, RollButtonSize}, "l", Ui::Style::Default}
  , _B{*this, {Ui::Snap::Left, _L, RollButtonSize}, "B", Ui::Style::Default}
  , _b{*this, {Ui::Snap::Top, _B, RollButtonSize}, "b", Ui::Style::Default}
  , _F{*this, {Ui::Snap::Left, _B, RollButtonSize}, "F", Ui::Style::Default}
  , _f{*this, {Ui::Snap::Top, _F, RollButtonSize}, "f", Ui::Style::Default}
  , _apply{*this, {Ui::Snap::Bottom, _degrees, Vector2{64, 24}}, "Apply", Ui::Style::Primary}
{
  Ui::Label{*this, {Ui::Snap::Bottom|Ui::Snap::Left, Vector2(256, 96)},
            "Keys f/F, b/B, u/U, d/D to roll.\nSpace key for redirection.\n"
            "Enter key to origin viewport.\nPress mouse to move.\n"
            "Alt+Enter keys to full screen.",
            Text::Alignment::MiddleLeft};
  Ui::Label{*this, {Ui::Snap::Left, _degrees, Vector2{64, 24}}, "Degrees/Sec: ", Text::Alignment::MiddleRight};
}
