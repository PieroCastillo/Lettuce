/*
Created by @PieroCastillo on 2026-06-01
*/
#ifndef LETTUCE_UTILS_FRAME_TIMER_HPP
#define LETTUCE_UTILS_FRAME_TIMER_HPP

#include <chrono>

namespace Lettuce::Utils
{
    class FrameTimer
    {
    public:
        using clock = std::chrono::steady_clock;
        using time_point = clock::time_point;
        using seconds_f = std::chrono::duration<float>;

        void Start()
        {
            m_start = clock::now();
            m_last = m_start;
            m_delta = 0.0f;
            m_running = true;
        }

        void Tick()
        {
            if (!m_running) return;

            time_point now = clock::now();
            m_delta = std::chrono::duration_cast<seconds_f>(now - m_last).count();
            m_last = now;
        }

        float GetDeltaTime() const
        {
            return m_delta;
        }

        float GetTotalTime() const
        {
            if (!m_running) return 0.0f;
            return std::chrono::duration_cast<seconds_f>(clock::now() - m_start).count();
        }

    private:
        time_point m_start{};
        time_point m_last{};
        float m_delta{ 0.0f };
        bool m_running{ false };
    };
}
#endif // LETTUCE_UTILS_FRAME_TIMER_HPP