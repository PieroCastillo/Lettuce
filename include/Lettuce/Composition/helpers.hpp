/*
Created by @PieroCastillo on 2026-01-25
*/
#ifndef LETTUCE_COMPOSITION_HELPERS_HPP
#define LETTUCE_COMPOSITION_HELPERS_HPP

#include <array>
#include <chrono>
#include <cstdint>
#include <span>

#include "../Core/api.hpp"
#include "api.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Composition;

namespace Lettuce::Composition::Helpers
{
    static constexpr Color FromRGBA(uint32_t rgba) {
        return {
            ((rgba >> 24) & 0xFF) / 255.0f,
            ((rgba >> 16) & 0xFF) / 255.0f,
            ((rgba >> 8) & 0xFF) / 255.0f,
            (rgba & 0xFF) / 255.0f
        };
    }

    constexpr Color White = { 1.0f, 1.0f, 1.0f, 1.0f };
    constexpr Color Black = { 0.0f, 0.0f, 0.0f, 1.0f };
    constexpr Color Transparent = { 0.0f, 0.0f, 0.0f, 0.0f };
    constexpr Color Red = { 1.0f, 0.0f, 0.0f, 1.0f };
    constexpr Color Green = { 0.0f, 1.0f, 0.0f, 1.0f };
    constexpr Color Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
    constexpr Color MaterialRed = FromRGBA(0xF44336FF);
    constexpr Color MaterialPink = FromRGBA(0xE91E63FF);
    constexpr Color MaterialPurple = FromRGBA(0x9C27B0FF);
    constexpr Color MaterialDeepPurple = FromRGBA(0x673AB7FF);
    constexpr Color MaterialIndigo = FromRGBA(0x3F51B5FF);
    constexpr Color MaterialBlue = FromRGBA(0x2196F3FF);
    constexpr Color MaterialLightBlue = FromRGBA(0x03A9F4FF);
    constexpr Color MaterialCyan = FromRGBA(0x00BCD4FF);
    constexpr Color MaterialTeal = FromRGBA(0x009688FF);
    constexpr Color MaterialGreen = FromRGBA(0x4CAF50FF);
    constexpr Color MaterialLightGreen = FromRGBA(0x8BC34AFF);
    constexpr Color MaterialLime = FromRGBA(0xCDDC39FF);
    constexpr Color MaterialYellow = FromRGBA(0xFFEB3BFF);
    constexpr Color MaterialAmber = FromRGBA(0xFFC107FF);
    constexpr Color MaterialOrange = FromRGBA(0xFF9800FF);
    constexpr Color MaterialDeepOrange = FromRGBA(0xFF5722FF);
    constexpr Color MaterialBrown = FromRGBA(0x795548FF);
    constexpr Color MaterialGrey = FromRGBA(0x9E9E9EFF);
    constexpr Color MaterialBlueGrey = FromRGBA(0x607D8BFF);

    constexpr Vec2 Zero2 = { 0.0f, 0.0f };
    constexpr Vec2 One = { 1.0f, 1.0f };
    constexpr Vec3 Zero3 = { 0.0f, 0.0f, 0.0f };

    constexpr AnimationDesc DefaultAnimation() {
        return { 300,0, 1, EasingMode::EaseInOut, true, 0.25f, 0.1f, 0.25f, 1.0f };
    }
    constexpr NaturalMotionAnimationDesc DefaultNaturalMotionAnimation() {
        return { 300.0f, 20.0f, 1.0f };
    }
    constexpr AnimationDesc Quick(uint32_t ms = 150) {
        return { ms, 0, 1, EasingMode::EaseInOut };
    }
    constexpr AnimationDesc Smooth(uint32_t ms = 300) {
        return { ms, 0, 1, EasingMode::EaseInOut };
    }
    constexpr AnimationDesc Bounce() {
        return { 400, 0, 1, EasingMode::EaseOutBounce };
    }
    constexpr AnimationDesc FadeIn(uint32_t durationMs = 300) {
        return { durationMs, 0, 1, EasingMode::EaseOut };
    }
    constexpr AnimationDesc FadeOut(uint32_t durationMs = 300) {
        return { durationMs, 0, 1, EasingMode::EaseIn };
    }
    constexpr AnimationDesc ScaleIn(uint32_t durationMs = 300) {
        return { durationMs, 0, 1, EasingMode::EaseOut };
    }
    constexpr AnimationDesc ScaleOut(uint32_t durationMs = 300) {
        return { durationMs, 0, 1, EasingMode::EaseIn };
    }
    constexpr AnimationDesc SlideIn(uint32_t durationMs = 300) {
        return { durationMs, 0, 1, EasingMode::EaseOut };
    }
    constexpr AnimationDesc Pulse(uint32_t durationMs = 200) {
        return { durationMs, 0, 2, EasingMode::EaseInOut };
    }
    constexpr NaturalMotionAnimationDesc SoftMotion() {
        return { 200.0f, 15.0f, 1.0f };
    }
    constexpr NaturalMotionAnimationDesc StiffMotion() {
        return { 400.0f, 25.0f, 1.0f };
    }

    constexpr BrushDesc SolidColorBrush(Color color) {
        return { BrushType::SolidColor, color };
    }
    constexpr BrushDesc LinearGradientBrush(Color start, Color end) {
        return { BrushType::LinearGradient, start, end };
    }
    constexpr BrushDesc RadialGradientBrush(Color center, Color edge) {
        return { BrushType::RadialGradient, center, edge };
    }
    constexpr BrushDesc AcrylicBrush(Color tint, float blurAmount = 30.0f) {
        return { BrushType::Acrylic, tint, Black, blurAmount };
    }
    constexpr BrushDesc NoiseBrush(Color baseColor = White) {
        return { BrushType::Noise, baseColor };
    }
    inline BrushDesc TextureBrush(Texture texture) {
        return { BrushType::Texture, White, Black, 0.0f, texture };
    }

    constexpr LightDesc DefaultLight() {
        return { LightType::Point, White, 1.0f, {0.0f, 0.0f, 100.0f}, {0.0f, 0.0f, -1.0f} };
    }
    constexpr LightDesc AmbientLight(Color color = White, float intensity = 0.5f) {
        return { LightType::Ambient, color, intensity };
    }
    constexpr LightDesc PointLight(Vec3 position, Color color = White, float intensity = 1.0f) {
        return { LightType::Point, color, intensity, position };
    }

    constexpr EffectDesc DropShadow(Vec2 offset = { 0, 4 }, float radius = 8.0f, Color color = { 0, 0, 0, 0.3f }) {
        return { EffectType::DropShadow, 1.0f, color, offset, radius };
    }
    constexpr EffectDesc BlurEffect(float radius = 5.0f) {
        return { EffectType::Blur, 1.0f,  Black, Zero2, radius };
    }
    constexpr EffectDesc GlowEffect(Color color, float radius = 10.0f) {
        return { EffectType::Glow, 1.0f, color, Zero2, radius };
    }
}

#endif // LETTUCE_COMPOSITION_HELPERS_HPP