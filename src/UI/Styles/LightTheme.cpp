// project headers
#include "Lettuce/UI/types.hpp"
#include "Lettuce/UI/styles.hpp"

using namespace Lettuce::Quimera;
using namespace Lettuce::UI;

LightTheme::LightTheme(Surface&)
{

}

LightTheme::~LightTheme()
{
    
}

auto LightTheme::GetBackground(State) -> Lettuce::Quimera::Brush
{
    return {};
}

auto LightTheme::GetForeground(State) -> Lettuce::Quimera::Brush
{
    return {};
}

auto LightTheme::GetThickness(State) -> Lettuce::Quimera::Brush
{
    return {};
}