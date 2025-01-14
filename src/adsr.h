#include <math.h>
#include <stdio.h>

#define MIN_ADSR_AMPLITUDE 0.00005f

typedef enum
{
    ADSR_STATE_IDLE = 0,
    ADSR_STATE_ATTACK,
    ADSR_STATE_DECAY,
    ADSR_STATE_SUSTAIN,
    ADSR_STATE_RELEASE
} adsr_state_t;

/*
 * This struct holds all precomputed values for an exponential ADSR.
 *  a_ms, d_ms, s_db, r_ms: Attack/Decay/Release in milliseconds, Sustain in dB.
 *  samplerate: sample rate in Hz.
 */
typedef struct
{
    float samplerate;

    // User-specified times and sustain-level (in dB).
    float attackMs;
    float decayMs;
    float releaseMs;
    float sustainDb;

    // Sustain in linear scale.
    float sustainLevel; // = 10^(sustainDb/20)

    // Exponential coefficients for each phase.
    float attackCoef;
    float decayCoef;
    float releaseCoef;

    // Current envelope value in linear amplitude.
    float env;

    // Current state of the envelope.
    adsr_state_t state;
} adsr_t;
