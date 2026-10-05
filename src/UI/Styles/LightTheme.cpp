// project headers
#include "Lettuce/UI/types.hpp"
#include "Lettuce/UI/styles.hpp"

using namespace Lettuce::Quimera;
using namespace Lettuce::UI;

LightTheme::LightTheme(Surface& surf)
{
    m_brushes.resize(3);
    m_brushes[0] = surf.CreateBrush(SolidColorBrushDesc{ Colors::White });
    m_brushes[1] = surf.CreateBrush(SolidColorBrushDesc{ Colors::Gray });
    m_brushes[2] = surf.CreateBrush(SolidColorBrushDesc{ Colors::Black });
    // m_font = 
}

LightTheme::~LightTheme()
{

}

auto LightTheme::GetBackground(State state) -> Lettuce::Quimera::Brush
{
    switch (state)
    {
    case State::Default:
    default:
        return m_brushes[0];
    }
}

auto LightTheme::GetForeground(State state) -> Lettuce::Quimera::Brush
{
    switch (state)
    {
    case State::Default:
    default:
        return m_brushes[1];
    }
}

auto LightTheme::GetThickness(State state) -> Lettuce::Quimera::Brush
{
    switch (state)
    {
    case State::Default:
    default:
        return m_brushes[2];
    }
}

auto LightTheme::GetFontFamily() -> Lettuce::Quimera::Font
{
    return m_font;
}