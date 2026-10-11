/*
Created by @PieroCastillo on 2026-10-01
*/
#ifndef LETTUCE_UI_STYLES_HPP
#define LETTUCE_UI_STYLES_HPP

// standard headers
#include <vector>

// project headers
#include "../Quimera/api.hpp"
#include "input.hpp"
#include "types.hpp"

namespace Lettuce::UI
{
    class LightTheme : public Style
    {
    private:
        Lettuce::Quimera::Surface* m_surface;
        std::vector<Lettuce::Quimera::Brush> m_brushes;
        Lettuce::Quimera::Font m_font;
    public:
        LightTheme() = default;
        explicit LightTheme(Lettuce::Quimera::Surface&);
        ~LightTheme() override;
        auto GetBackground(State) -> Lettuce::Quimera::Brush override;
        auto GetForeground(State) -> Lettuce::Quimera::Brush override;
        auto GetThickness(State) -> Lettuce::Quimera::Brush override;
        auto GetFontFamily() -> Lettuce::Quimera::Font override;
    };
};
#endif // LETTUCE_UI_STYLES_HPP