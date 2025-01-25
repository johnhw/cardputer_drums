#include "fx.h"


void dynamicCompressFX(
    int16_t *samples,
    int32_t numSamples,
    compressor_t *compressor)
{
    float threshold_dB = compressor->threshold_dB;
    float ratio = compressor->ratio;
    float makeup_dB = compressor->makeup_dB;
    float attack_ms = compressor->attack_ms;
    float release_ms = compressor->release_ms;
    float sampleRate = compressor->sampleRate;
    float *rms = &compressor->rms;

    // --- Parameter validation / clamping (optional) ---
    if (numSamples <= 0 || sampleRate <= 0 || ratio < 1.0f)
    {
        return; // Nothing to do or invalid usage
    }

    // --- Convert times from ms to "per sample" exponential factors ---
    // Typically, alpha = exp(-1 / (timeConstant * sampleRate)),
    // but you can tweak the formula’s constant to taste.
    const float attackTime = attack_ms / 1000.0f;   // seconds
    const float releaseTime = release_ms / 1000.0f; // seconds

    // Attack and release coefficients for the RMS filter
    float alphaAttack = expf(-1.0f / (attackTime * sampleRate));
    float alphaRelease = expf(-1.0f / (releaseTime * sampleRate));

    // --- Pre-calculate linear makeup gain ---
    float makeupGainLinear = powf(10.0f, makeup_dB / 20.0f);

    // --- Store threshold in dB for reference ---
    float thresholdLinear = powf(10.0f, threshold_dB / 20.0f);

    // We'll keep track of the RMS in *linear* domain.
    // You could track RMS^2, or do it directly in dB,
    // but this is a straightforward approach.

    for (int32_t i = 0; i < numSamples; i++)
    {
        // 1) Convert sample from int16 to float in [-1.0, 1.0].
        float in = (float)samples[i] / 32768.0f;
        float mag = fabsf(in);

        // 2) Update RMS (one-pole smoothing of the square, then sqrt).
        //    We detect whether we need attack or release coefficient
        //    based on whether the new magnitude is larger or smaller
        //    than our current RMS.
        float currentRMSsq = (*rms) * (*rms);
        float sampleSq = mag * mag;

        // If sampleSq is higher than current RMS^2, we use attack time,
        // otherwise release time.
        if (sampleSq > currentRMSsq)
        {
            currentRMSsq = alphaAttack * currentRMSsq + (1.0f - alphaAttack) * sampleSq;
        }
        else
        {
            currentRMSsq = alphaRelease * currentRMSsq + (1.0f - alphaRelease) * sampleSq;
        }
        *rms = sqrtf(currentRMSsq + 1e-24f); // add tiny offset for numerical safety

        // 3) Convert RMS to dB
        float rms_dB = 20.0f * log10f(*rms);

        // 4) Compute the amount (if any) by which we exceed threshold
        float dB_above_threshold = rms_dB - threshold_dB;
        if (dB_above_threshold < 0.0f)
        {
            dB_above_threshold = 0.0f; // No compression below threshold
        }

        // 5) Calculate the gain reduction in dB:
        //    If we are 'above threshold' by X dB,
        //    the output is threshold_dB + X/ratio,
        //    so the gain in dB is [threshold_dB + X/ratio - (threshold_dB + X)]
        //    = - X * (1 - 1/ratio).
        float gainReduction_dB = -dB_above_threshold * ((ratio - 1.0f) / ratio);

        // 6) Add makeup gain in dB
        float totalGain_dB = gainReduction_dB + makeup_dB;

        // 7) Convert total gain back to linear
        float totalGain_linear = powf(10.0f, totalGain_dB / 20.0f);

        // 8) Apply gain to current sample
        float out = in * totalGain_linear;

        // 9) Prevent clipping at int16 range
        if (out > 1.0f)
            out = 1.0f;
        if (out < -1.0f)
            out = -1.0f;

        // 10) Convert back to int16
        samples[i] = (int16_t)(out * 32767.0f);
    }
}