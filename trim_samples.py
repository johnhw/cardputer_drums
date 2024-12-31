#!/usr/bin/env python3
import click
import numpy as np
from pathlib import Path

from scipy.io import wavfile
from scipy.signal import resample_poly

def find_power_exceeding(
    snd: np.ndarray, 
    thresh_db: float, 
    window_samples: int = 1024, 
    from_start: bool = True
) -> int:
    """
    Scan through the signal in 'window_samples'-sized chunks and return
    the first (or last) sample index where the RMS amplitude exceeds the
    threshold in dB. If no window is above threshold, return -1.

    :param snd: 1D float array of audio samples.
    :param thresh_db: Threshold in dB (e.g. -50.0) for RMS amplitude.
    :param window_samples: Size of the RMS window in samples.
    :param from_start: If True, find the first window from the left that exceeds threshold;
                       If False, find the last window from the right that exceeds threshold.
    :return: Sample index in [0..len(snd)-1], or -1 if no window is above threshold.
    """

    # Convert threshold in dB to linear amplitude.
    # For amplitude-based thresholds, we can do: threshold_lin = 10^(thresh_db / 20)
    threshold_lin = 10 ** (thresh_db / 20.0)

    # Edge case: if signal is shorter than one window, just compute once
    if len(snd) < window_samples:
        rms_val = np.sqrt(np.mean(snd**2))
        if rms_val > threshold_lin:
            return 0 if from_start else len(snd) - 1
        else:
            return -1

    if from_start:
        # Search from the left
        for start_idx in range(0, len(snd) - window_samples + 1):
            window = snd[start_idx : start_idx + window_samples]
            rms_val = np.sqrt(np.mean(window**2))
            if rms_val > threshold_lin:
                return start_idx
        return -1
    else:
        # Search from the right
        for start_idx in range(len(snd) - window_samples, -1, -1):
            window = snd[start_idx : start_idx + window_samples]
            rms_val = np.sqrt(np.mean(window**2))
            if rms_val > threshold_lin:
                # Return the end of this window or the start index?
                # Usually, for "end" trim, you'd want the 'end' of the window
                # but we can just return start_idx + window_samples - 1
                return start_idx + window_samples - 1
        return -1

@click.command()
@click.option(
    "--dir", "-d", 
    default=".",
    help="Directory containing .wav files (default: current directory)."
)
@click.option(
    "--rate", "-r", 
    default=16000, 
    type=int,
    help="Sampling rate for output files (default: 16000)."
)
@click.option(
    "--silence", "-s", 
    default=-50.0,
    type=float,
    help="Silence threshold in dB for trimming (default: -50 dB)."
)
@click.option(
    "--time", "-t", 
    default=0.1,
    type=float,
    help="Maximum duration (seconds) to keep; files longer than this will be trimmed (default: 0.1)."
)
@click.option(
    "--fade-time", "-f",
    default=0.003,
    type=float,
    help="Fade-out duration (seconds), applied if anything is trimmed (default: 0.003)."
)
@click.option(
    "--window-samples", "-w",
    default=60,
    type=int,
    help="Window size (samples) for RMS-based silence detection (default: 60)."
)
def main(dir, rate, silence, time, fade_time, window_samples):
    """
    Script to process .wav files in a directory:
    - Convert to mono and specified sampling rate
    - Trim silence at start/end
    - Further trim if longer than specified time
    - Apply fade-out to the last fade_time seconds if any trimming occurred
    - Save as 16-bit mono .wav to export/ directory
    - Print stats per file and total samples in output
    """

    input_dir = Path(dir)
    output_dir = input_dir / "export"
    output_dir.mkdir(exist_ok=True)  # Create the export directory if needed

    # Convert the silence threshold in dB to linear amplitude
    # threshold dB => threshold_lin = 10^(threshold_dB / 20)
    silence_threshold_lin = 10 ** (silence / 20.0)

    total_samples_written = 0  # Accumulate total samples of all processed files
    
    wav_files = list(input_dir.glob("*.wav"))
    if not wav_files:
        click.echo(f"No .wav files found in '{input_dir.resolve()}'")
        return

    for wav_path in sorted(wav_files):
        # Read file
        orig_rate, orig_data = wavfile.read(wav_path)
        orig_data = orig_data.astype(np.float32)  # Convert to float for processing

        # If stereo, convert to mono by averaging channels
        if orig_data.ndim > 1:
            orig_data = np.mean(orig_data, axis=1)

        # Store original length info (before any resampling or trimming)
        orig_num_samples = len(orig_data)
        orig_duration_sec = orig_num_samples / orig_rate

        # Resample to desired rate using resample_poly
        # (resample_poly is often preferred over resample for audio)
        # We want: new_data = resample_poly(orig_data, up=rate, down=orig_rate)
        # but to avoid potential large memory, we do an integer ratio approach or a direct factor:
        # If rate/orig_rate is not integer, resample_poly can handle that.
        new_data = resample_poly(orig_data, rate, orig_rate)

        # We'll do RMS-based trimming on new_data
        # 1) Find the first chunk from left that exceeds threshold
        start_idx = find_power_exceeding(
            snd=new_data, 
            thresh_db=silence, 
            window_samples=window_samples, 
            from_start=True
        )
        # 2) Find from right
        end_idx = find_power_exceeding(
            snd=new_data, 
            thresh_db=silence, 
            window_samples=window_samples, 
            from_start=False
        )

        # If the entire file is below threshold, we can skip or produce empty
        if start_idx == -1 or end_idx == -1 or start_idx > end_idx:
            trimmed_data = np.array([0], dtype=np.float32)
        else:
            # Include end_idx in the slice => end_idx+1
            trimmed_data = new_data[start_idx : end_idx + 1]

        # Now handle "Further trim the end if longer than `time` seconds"
        new_sr = rate
        max_samples = int(round(time * new_sr))
        trim_happened = False

        if len(trimmed_data) > max_samples:
            trimmed_data = trimmed_data[:max_samples]
            trim_happened = True

        # We'll do a fade-out if "the end of the file has been trimmed by either silence or time"
        # That is, if the actual start or end changed from the original or if we trimmed for time.
        # We should check if start_idx != 0 or end_idx != len(new_data) => that means silence-based trim,
        # or if we trimmed to max_samples => time-based trim
        if len(trimmed_data) == 0:
            # If it's fully trimmed out, there's nothing to fade, so skip
            final_data = trimmed_data
        else:
            silence_trim_happened = (above_thresh[0] != 0 or above_thresh[-1] != len(new_data) - 1)
            if silence_trim_happened or trim_happened:
                # Apply exponential decay fade over the last fade_time seconds
                fade_samples = int(round(fade_time * new_sr))
                if fade_samples > len(trimmed_data):
                    fade_samples = len(trimmed_data)

                # We want to go from 1.0 to ~10^(-120/20)=10^(-6)=0.000001 over fade_samples
                # That implies amplitude(t) = exp(-alpha * t), alpha>0
                # amplitude(0)=1, amplitude(fade_samples-1)=1e-6 => solve for alpha
                alpha = (6 * np.log(10)) / (fade_samples - 1) if fade_samples > 1 else 0.0
                # Build fade envelope
                fade_envelope = np.exp(-alpha * np.arange(fade_samples))
                fade_envelope = fade_envelope.astype(np.float32)[::-1]  # optional reverse, or keep as is
                # We want the fade on the *end* => multiply the last fade_samples
                final_data = trimmed_data.copy()
                final_data[-fade_samples:] *= fade_envelope[:fade_samples]
            else:
                # No trimming, so no fade
                final_data = trimmed_data

        # Convert to 16-bit integers for writing
        # Typical approach: ensure the data is clipped to -1..1 range first
        final_data = np.clip(final_data, -1.0, 1.0)
        # Then scale to int16 range
        final_data_16 = (final_data * 32767.0).astype(np.int16)

        # Write out to export folder
        out_path = output_dir / wav_path.name  # same filename, but in export/
        wavfile.write(str(out_path), new_sr, final_data_16)

        # Print per-file stats
        new_num_samples = len(final_data_16)
        new_duration_sec = new_num_samples / new_sr

        click.echo(
            f"File: {wav_path.name}\n"
            f"  Original length: {orig_duration_sec:.4f}s ({orig_num_samples} samples) @ {orig_rate} Hz\n"
            f"  Final length:    {new_duration_sec:.4f}s ({new_num_samples} samples) @ {new_sr} Hz\n"
        )

        total_samples_written += new_num_samples

    # Print total samples in the exported wav files
    click.echo(
        f"Total samples in all processed files (in '{output_dir.name}/'): {total_samples_written}"
    )

if __name__ == "__main__":
    main()