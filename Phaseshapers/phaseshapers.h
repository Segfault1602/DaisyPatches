// =============================================================================
// phaseshapers.h -- Phaseshaper implementations
//
// Original implementation by:
// J.Kleimola, V.Lazzarini, J.Timoney, V.Valimaki,
// "PHASESHAPING OSCILLATOR ALGORITHMS FOR MUSICAL SOUND SYNTHESIS",
// SMC 2010, Barcelona, Spain, July 21-24, 2010.
//
// Adapted from python by:
// Alex St-Onge
// =============================================================================

#include "daisysp.h"

namespace PhaseShapers
{

enum class Waveform : uint8_t
{
    WAVESLICE = 0,
    HARDSYNC,
    SOFTSYNC,
    TRIANGLE_MOD,
    SUPERSAW,
    VARIABLE_SLOPE,
    NUM_WAVES,
};

class Oscillator
{
  public:
    Oscillator() = default;
    ~Oscillator() = default;

    void Init(float sampleRate)
    {
        m_sampleRate = sampleRate;
        m_phase = 0.f;
        m_freq = 220.f;
        m_phaseIncrement = m_freq / m_sampleRate;
        m_period = m_sampleRate / m_freq;

        m_lfo.Init(sampleRate);
        m_lfo.SetFreq(1.f);
        m_lfo.SetAmp(0.25f);
    }

    void SetWaveform(const Waveform wave)
    {
        m_waveform = wave;
    }

    void SetFreq(const float freq)
    {
        m_freq = freq;
        m_phaseIncrement = m_freq / m_sampleRate;
        m_period = m_sampleRate / m_freq;
    }

    float Process();

  private:
    float ProcessWaveSlice();
    float ProcessHardSync();
    float ProcessSoftSync();
    float ProcessTriMod();
    float ProcessSupersaw();
    float ProcessVarSlope();
    float ProcessVarTri();

  private:
    float m_sampleRate = 0.f;
    float m_freq = 220.f;
    float m_phaseIncrement = 0.f;
    float m_phase = 0.f;
    float m_period = 0.f;
    Waveform m_waveform = Waveform::WAVESLICE;

    daisysp::Oscillator m_lfo;
};
} // namespace PhaseShapers