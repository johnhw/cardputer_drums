#include <math.h>
#include "adsr.h"

// Helper function: Convert decibels to a linear amplitude.
//   dB -> amp = 10^(dB/20)
static inline float dbToLinear(float dB)
{
    return powf(10.0f, dB / 20.0f);
}

// Precompute a coefficient so that envelope transitions from
// startAmp to endAmp exponentially over numSamples samples.
//   coef = (endAmp / startAmp)^(1 / numSamples)
// If numSamples <= 0, we avoid division by zero and return 1.0f (no change).

static float computeCoef(float startAmp, float endAmp, float numSamples)
{
    if (numSamples <= 1.0f || startAmp <= 0.0f)
    {
        return 1.0f;
    }
    return powf(endAmp / startAmp, 1.0f / numSamples);
}

// Initialize the ADSR struct.
// Times are in milliseconds, sustain in dB, samplerate in Hz.
void initADSR(adsr_t *adsr,
              float a_ms, float d_ms, float s_db, float r_ms,
              int samplerate)
{
    adsr->samplerate = samplerate;

    
    adsr->attackMs = a_ms;
    adsr->decayMs = d_ms;
    adsr->releaseMs = r_ms;
    adsr->sustainDb = s_db;

    

    // Convert sustain from dB to linear
    adsr->sustainLevel = dbToLinear(s_db);

    // Precompute the attack coefficient
    {
        float attackSamples = a_ms * samplerate * 0.001f;        
        float startAmp = MIN_ADSR_AMPLITUDE; // -90 dB
        float endAmp = 1.0f;                 // 0 dB
        adsr->attackCoef = computeCoef(startAmp, endAmp, attackSamples);
    }

    // Precompute the decay coefficient
    // (from 1.0 to sustainLevel)
    {
        float decaySamples = d_ms * samplerate * 0.001f;
        float startAmp = 1.0f;
        float endAmp = adsr->sustainLevel;
        adsr->decayCoef = computeCoef(startAmp, endAmp, decaySamples);
    }

    // We will set the releaseCoef dynamically inside releaseADSR(...)
    // because the release starts from the *current* envelope level.
    adsr->releaseCoef = 1.0f; // placeholder; see releaseADSR()

    // Start envelope at -90 dB, in idle state
    adsr->env = MIN_ADSR_AMPLITUDE;
    adsr->state = ADSR_STATE_IDLE;
}

// Trigger the envelope (i.e., begin Attack).
void triggerADSR(adsr_t *adsr)
{
    adsr->env = MIN_ADSR_AMPLITUDE; // -90 dB
    adsr->state = ADSR_STATE_ATTACK;
    adsr->enabled = true;
}

// Begin the Release phase (envelope goes from current level down to -90 dB).
// We recompute releaseCoef based on the *current* envelope value.
void releaseADSR(adsr_t *adsr)
{
    // If already idle or near zero, do nothing special
    if (adsr->state == ADSR_STATE_IDLE)
    {
        return;
    }
    float currentAmp = (adsr->env < MIN_ADSR_AMPLITUDE)
                           ? MIN_ADSR_AMPLITUDE
                           : adsr->env;
    float releaseSamples = adsr->releaseMs * adsr->samplerate * 0.001f;
    float startAmp = currentAmp;
    float endAmp = MIN_ADSR_AMPLITUDE; // -90 dB
    adsr->releaseCoef = computeCoef(startAmp, endAmp, releaseSamples);
    adsr->state = ADSR_STATE_RELEASE;
}

// Generate the next envelope value (in linear amplitude).
// Call this once per sample in your audio callback.
float nextADSR(adsr_t *adsr)
{
    switch (adsr->state)
    {
    case ADSR_STATE_IDLE:
        // Envelope is off or done
        break;

    case ADSR_STATE_ATTACK:
        // Attack: multiply envelope by precomputed coefficient
        adsr->env *= adsr->attackCoef;
        if (adsr->env >= 1.0f)
        {
            adsr->env = 1.0f;
            adsr->state = ADSR_STATE_DECAY;
        }
        break;

    case ADSR_STATE_DECAY:
        // Decay: from 1.0 down to sustainLevel
        adsr->env *= adsr->decayCoef;
        if (adsr->env <= adsr->sustainLevel)
        {
            adsr->env = adsr->sustainLevel;
            adsr->state = ADSR_STATE_SUSTAIN;
        }
        break;

    case ADSR_STATE_SUSTAIN:
        // Sustain: hold at sustainLevel
        // (Do nothing; just return sustainLevel below)
        break;

    case ADSR_STATE_RELEASE:
        // Release: from current envelope down to -90 dB
        adsr->env *= adsr->releaseCoef;
        if (adsr->env <= MIN_ADSR_AMPLITUDE)
        {
            adsr->env = 0.0f;
            adsr->state = ADSR_STATE_IDLE;
        }
        break;

    default:
        break;
    }
    return adsr->env;
}