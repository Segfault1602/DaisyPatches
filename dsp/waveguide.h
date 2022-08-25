// =============================================================================
// waveguide.h -- Trivial digital waveguide implementation
//
// Heavily based on daisysp/Delayline.h
// =============================================================================
#pragma once

#include "delayline2.h"
#include <stdlib.h>

namespace marguerite
{
template <size_t MAX_DELAY> class Waveguide
{
  public:
    Waveguide() = default;
    ~Waveguide() = default;

    void Reset()
    {
        m_rightDelay.Reset();
        m_leftDelay.Reset();
    }

    void SetDelay(size_t delay)
    {
        m_delayLength = std::min(delay, MAX_DELAY);

        m_rightDelay.SetDelay(m_delayLength);
        m_leftDelay.SetDelay(m_delayLength);

        m_pickup = std::min(m_pickup, m_delayLength);
    }

    void SetPickup(size_t pickup)
    {
        m_pickup = std::min(pickup, m_delayLength);
    }

    void AddIn(float sample, size_t addLocation)
    {
        float right = m_rightDelay.Read(addLocation);
        m_rightDelay.TapIn(right + sample, addLocation);

        float left = m_leftDelay.Read(addLocation);
        m_leftDelay.TapIn(left + sample, addLocation);
    }

    void TapIn(float sample, size_t tapDelay)
    {
        m_rightDelay.TapIn(sample, tapDelay);
        m_leftDelay.TapIn(sample, m_delayLength - tapDelay - 1);
    }

    void Pluck(size_t pluckLocation)
    {
        const float amp = 0.8f;
        float inc = amp / pluckLocation;
        float pluck = 0;
        for (size_t i = 0; i < pluckLocation; ++i)
        {
            TapIn(pluck, i);
            pluck += inc;
        }

        inc = amp / (m_delayLength - pluckLocation);
        for (size_t i = pluckLocation; i < m_delayLength; ++i)
        {
            TapIn(pluck, i);
            pluck -= inc;
        }
    }

    float Process()
    {
        float rightSample = m_rightDelay.Read();
        float leftSample = m_leftDelay.Read();

        m_rightDelay.Write(leftSample * -1 * m_g);
        m_leftDelay.Write(rightSample * -1 * m_g);

        size_t rightPickup = m_pickup;
        size_t leftPickup = m_delayLength - m_pickup - 1;

        return m_rightDelay.Read(rightPickup) + m_leftDelay.Read(leftPickup);
    }

  private:
    size_t m_delayLength;
    float m_g = 0.99;
    size_t m_pickup;
    DelayLine2<float, MAX_DELAY> m_rightDelay;
    DelayLine2<float, MAX_DELAY> m_leftDelay;
};
} // namespace marguerite