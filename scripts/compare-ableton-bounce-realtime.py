#!/usr/bin/env python3
"""Compare Ableton offline bounce audio against realtime resampling evidence."""

from __future__ import annotations

import argparse
import json
import math
import random
import struct
import sys
import wave
from pathlib import Path
from typing import Any


DEFAULT_THRESHOLDS = {
    "min_alignment_envelope_correlation": 0.94,
    "min_content_envelope_correlation": 0.94,
    "min_filtered_band_correlation_mean": 0.90,
    "min_filtered_band_correlation_floor": 0.85,
    "min_frame_band_cosine_mean": 0.80,
    "max_rms_delta_db_abs": 1.0,
    "max_peak_delta_db_abs": 3.0,
    "max_peak_abs": 0.99,
    "min_rms": 0.0001,
}

FILTER_BANDS_HZ = (120.0, 500.0, 2000.0, 6000.0)
DEFAULT_WINDOW_SIZE = 4096
DEFAULT_HOP_SIZE = 1024


def fail(message: str) -> None:
    print(message, file=sys.stderr)
    raise SystemExit(2)


def read_audio(path: Path) -> tuple[int, list[list[float]]]:
    suffix = path.suffix.lower()
    if suffix == ".wav":
        return read_wav(path)
    if suffix in {".aif", ".aiff"}:
        return read_aiff(path)
    fail(f"Unsupported audio file extension for {path}")


def read_wav(path: Path) -> tuple[int, list[list[float]]]:
    with wave.open(str(path), "rb") as reader:
        channels = reader.getnchannels()
        sample_width = reader.getsampwidth()
        sample_rate = reader.getframerate()
        raw = reader.readframes(reader.getnframes())
    return sample_rate, decode_pcm(raw, sample_width, channels, byte_order="little")


def read_aiff(path: Path) -> tuple[int, list[list[float]]]:
    data = path.read_bytes()
    if len(data) < 12 or data[:4] != b"FORM" or data[8:12] not in {b"AIFF", b"AIFC"}:
        fail(f"{path} is not an AIFF/AIFC file")

    channels = sample_frames = sample_width_bits = sample_rate = None
    sound_data = None
    offset = 12

    while offset + 8 <= len(data):
        chunk_id = data[offset : offset + 4]
        chunk_size = struct.unpack(">I", data[offset + 4 : offset + 8])[0]
        chunk = data[offset + 8 : offset + 8 + chunk_size]

        if chunk_id == b"COMM":
            if len(chunk) < 18:
                fail(f"{path} has a malformed COMM chunk")
            channels = struct.unpack(">H", chunk[0:2])[0]
            sample_frames = struct.unpack(">I", chunk[2:6])[0]
            sample_width_bits = struct.unpack(">H", chunk[6:8])[0]
            sample_rate = round(read_extended_80(chunk[8:18]))
            if data[8:12] == b"AIFC":
                compression = chunk[18:22]
                if compression not in {b"NONE", b"twos"}:
                    fail(f"{path} uses unsupported AIFC compression {compression!r}")

        elif chunk_id == b"SSND":
            if len(chunk) < 8:
                fail(f"{path} has a malformed SSND chunk")
            sound_offset = struct.unpack(">I", chunk[0:4])[0]
            sound_data = chunk[8 + sound_offset :]

        offset = offset + 8 + chunk_size + (chunk_size % 2)

    if None in {channels, sample_frames, sample_width_bits, sample_rate} or sound_data is None:
        fail(f"{path} is missing COMM or SSND audio data")

    decoded = decode_pcm(sound_data, math.ceil(sample_width_bits / 8), channels, byte_order="big")
    if any(len(channel) < sample_frames for channel in decoded):
        fail(f"{path} ended before the declared AIFF sample count")
    return int(sample_rate), [channel[:sample_frames] for channel in decoded]


def read_extended_80(data: bytes) -> float:
    if len(data) != 10:
        fail("Invalid AIFF 80-bit sample-rate value")
    sign = -1 if (data[0] & 0x80) else 1
    exponent = ((data[0] & 0x7F) << 8) | data[1]
    mantissa_hi, mantissa_lo = struct.unpack(">II", data[2:10])
    if exponent == 0 and mantissa_hi == 0 and mantissa_lo == 0:
        return 0.0
    if exponent == 0x7FFF:
        return math.inf
    power = exponent - 16383
    return sign * ((mantissa_hi * (2.0 ** (power - 31))) + (mantissa_lo * (2.0 ** (power - 63))))


def decode_pcm(raw: bytes, sample_width: int, channels: int, *, byte_order: str) -> list[list[float]]:
    if channels <= 0:
        fail("Audio file declares no channels")
    output = [[] for _ in range(channels)]

    if sample_width == 1:
        for sample_index, value in enumerate(raw):
            output[sample_index % channels].append((float(value) - 128.0) / 128.0)
        return output

    if sample_width == 2:
        fmt = ">h" if byte_order == "big" else "<h"
        for sample_index, (value,) in enumerate(struct.iter_unpack(fmt, raw)):
            output[sample_index % channels].append(float(value) / 32768.0)
        return output

    if sample_width == 3:
        if len(raw) % 3 != 0:
            fail("24-bit PCM data length is not sample-aligned")
        for byte_index in range(0, len(raw), 3):
            chunk = raw[byte_index : byte_index + 3]
            if byte_order == "big":
                value = (chunk[0] << 16) | (chunk[1] << 8) | chunk[2]
            else:
                value = chunk[0] | (chunk[1] << 8) | (chunk[2] << 16)
            if value & 0x800000:
                value |= ~0xFFFFFF
            output[(byte_index // 3) % channels].append(float(value) / 8388608.0)
        return output

    if sample_width == 4:
        fmt = ">i" if byte_order == "big" else "<i"
        for sample_index, (value,) in enumerate(struct.iter_unpack(fmt, raw)):
            output[sample_index % channels].append(float(value) / 2147483648.0)
        return output

    fail(f"Unsupported PCM sample width: {sample_width} bytes")


def mixed_channel(channels: list[list[float]]) -> list[float]:
    frame_count = min(len(channel) for channel in channels)
    return [sum(channel[index] for channel in channels) / len(channels) for index in range(frame_count)]


def prefix_squares(samples: list[float]) -> list[float]:
    total = 0.0
    prefix = [0.0]
    for sample in samples:
        total += sample * sample
        prefix.append(total)
    return prefix


def rms(samples: list[float]) -> float:
    if not samples:
        return 0.0
    return math.sqrt(sum(sample * sample for sample in samples) / len(samples))


def rms_from_prefix(prefix: list[float], start: int, window_size: int) -> float:
    return math.sqrt(max(0.0, prefix[start + window_size] - prefix[start]) / window_size)


def peak(samples: list[float]) -> float:
    return max((abs(sample) for sample in samples), default=0.0)


def db_ratio(numerator: float, denominator: float) -> float:
    if denominator <= 0.0:
        return 999.0 if numerator > 0.0 else 0.0
    if numerator <= 0.0:
        return -999.0
    return 20.0 * math.log10(numerator / denominator)


def correlation(left: list[float], right: list[float]) -> float:
    count = min(len(left), len(right))
    if count == 0:
        return 0.0
    left = left[:count]
    right = right[:count]
    left_mean = sum(left) / count
    right_mean = sum(right) / count
    numerator = 0.0
    left_energy = 0.0
    right_energy = 0.0
    for left_value, right_value in zip(left, right):
        left_delta = left_value - left_mean
        right_delta = right_value - right_mean
        numerator += left_delta * right_delta
        left_energy += left_delta * left_delta
        right_energy += right_delta * right_delta
    denominator = math.sqrt(left_energy * right_energy)
    return numerator / denominator if denominator > 1.0e-20 else 0.0


def envelope(samples: list[float], *, window_size: int = DEFAULT_WINDOW_SIZE, hop_size: int = DEFAULT_HOP_SIZE) -> list[float]:
    if len(samples) < window_size:
        return []
    prefix = prefix_squares(samples)
    return [
        rms_from_prefix(prefix, start, window_size)
        for start in range(0, len(samples) - window_size + 1, hop_size)
    ]


def align_by_envelope(offline: list[float], realtime: list[float]) -> dict[str, Any]:
    offline_envelope = envelope(offline)
    realtime_envelope = envelope(realtime)
    if not offline_envelope or len(realtime_envelope) < len(offline_envelope):
        fail("Realtime capture is shorter than the offline bounce after envelope analysis")

    best_hop = 0
    best_correlation = -math.inf
    for hop_index in range(0, len(realtime_envelope) - len(offline_envelope) + 1):
        candidate = realtime_envelope[hop_index : hop_index + len(offline_envelope)]
        value = correlation(offline_envelope, candidate)
        if value > best_correlation:
            best_correlation = value
            best_hop = hop_index

    return {
        "offset_frames": best_hop * DEFAULT_HOP_SIZE,
        "offset_hops": best_hop,
        "envelope_correlation": best_correlation,
        "window_size": DEFAULT_WINDOW_SIZE,
        "hop_size": DEFAULT_HOP_SIZE,
    }


def one_pole_lowpass(samples: list[float], sample_rate: int, cutoff_hz: float) -> list[float]:
    alpha = (1.0 / sample_rate) / ((1.0 / (2.0 * math.pi * cutoff_hz)) + (1.0 / sample_rate))
    state = 0.0
    output = []
    for sample in samples:
        state += alpha * (sample - state)
        output.append(state)
    return output


def filtered_bands(samples: list[float], sample_rate: int) -> list[list[float]]:
    low_120 = one_pole_lowpass(samples, sample_rate, FILTER_BANDS_HZ[0])
    low_500 = one_pole_lowpass(samples, sample_rate, FILTER_BANDS_HZ[1])
    low_2000 = one_pole_lowpass(samples, sample_rate, FILTER_BANDS_HZ[2])
    low_6000 = one_pole_lowpass(samples, sample_rate, FILTER_BANDS_HZ[3])
    return [
        low_120,
        [low_500[index] - low_120[index] for index in range(len(samples))],
        [low_2000[index] - low_500[index] for index in range(len(samples))],
        [low_6000[index] - low_2000[index] for index in range(len(samples))],
        [samples[index] - low_6000[index] for index in range(len(samples))],
    ]


def percentile(values: list[float], percent: float) -> float:
    if not values:
        return 0.0
    ordered = sorted(values)
    index = (len(ordered) - 1) * percent / 100.0
    low_index = math.floor(index)
    high_index = math.ceil(index)
    if low_index == high_index:
        return ordered[low_index]
    low = ordered[low_index] * (high_index - index)
    high = ordered[high_index] * (index - low_index)
    return low + high


def frame_band_cosine(left_frame: list[float], right_frame: list[float]) -> float:
    return correlation(left_frame, right_frame)


def channel_metrics(
    offline: list[float], realtime_aligned: list[float], sample_rate: int, thresholds: dict[str, float]
) -> dict[str, Any]:
    frame_count = min(len(offline), len(realtime_aligned))
    offline = offline[:frame_count]
    realtime_aligned = realtime_aligned[:frame_count]

    offline_envelope = envelope(offline)
    realtime_envelope = envelope(realtime_aligned)

    offline_band_envelopes = [envelope(band) for band in filtered_bands(offline, sample_rate)]
    realtime_band_envelopes = [envelope(band) for band in filtered_bands(realtime_aligned, sample_rate)]

    active_frames = active_frame_mask(offline_envelope, realtime_envelope)
    if sum(active_frames) < 8:
        metrics = basic_channel_metrics(offline, realtime_aligned, sample_rate)
        metrics.update(
            {
                "content_envelope_correlation": correlation(offline_envelope, realtime_envelope),
                "filtered_band_correlation_mean": 0.0,
                "filtered_band_correlation_min": 0.0,
                "frame_band_cosine_mean": 0.0,
                "active_filtered_band_frames": sum(active_frames),
                "filtered_band_correlations": [0.0 for _ in FILTER_BANDS_HZ] + [0.0],
                "passed": False,
            }
        )
        return metrics

    band_correlations = []
    for band_index in range(len(offline_band_envelopes)):
        left_band = [offline_band_envelopes[band_index][index] for index, active in enumerate(active_frames) if active]
        right_band = [realtime_band_envelopes[band_index][index] for index, active in enumerate(active_frames) if active]
        band_correlations.append(correlation(left_band, right_band))

    frame_cosines = []
    for frame_index, active in enumerate(active_frames):
        if not active:
            continue
        left_frame = [band[frame_index] for band in offline_band_envelopes]
        right_frame = [band[frame_index] for band in realtime_band_envelopes]
        frame_cosines.append(frame_band_cosine(left_frame, right_frame))

    metrics = basic_channel_metrics(offline, realtime_aligned, sample_rate)
    metrics.update(
        {
            "content_envelope_correlation": correlation(offline_envelope, realtime_envelope),
            "filtered_band_correlation_mean": sum(band_correlations) / len(band_correlations),
            "filtered_band_correlation_min": min(band_correlations),
            "frame_band_cosine_mean": sum(frame_cosines) / len(frame_cosines),
            "active_filtered_band_frames": sum(active_frames),
            "filtered_band_correlations": band_correlations,
        }
    )
    metrics["passed"] = channel_passed(metrics, thresholds)
    return metrics


def basic_channel_metrics(offline: list[float], realtime_aligned: list[float], sample_rate: int) -> dict[str, Any]:
    frame_count = min(len(offline), len(realtime_aligned))
    offline = offline[:frame_count]
    realtime_aligned = realtime_aligned[:frame_count]
    offline_rms = rms(offline)
    realtime_rms = rms(realtime_aligned)
    offline_peak = peak(offline)
    realtime_peak = peak(realtime_aligned)
    return {
        "frames_compared": frame_count,
        "duration_seconds": frame_count / sample_rate,
        "offline_peak": offline_peak,
        "realtime_aligned_peak": realtime_peak,
        "offline_rms": offline_rms,
        "realtime_aligned_rms": realtime_rms,
        "rms_delta_db": db_ratio(realtime_rms, offline_rms),
        "peak_delta_db": db_ratio(realtime_peak, offline_peak),
        "waveform_correlation": correlation(offline, realtime_aligned),
    }


def active_frame_mask(offline_envelope: list[float], realtime_envelope: list[float]) -> list[bool]:
    count = min(len(offline_envelope), len(realtime_envelope))
    offline_envelope = offline_envelope[:count]
    realtime_envelope = realtime_envelope[:count]
    offline_floor = max(1.0e-5, percentile(offline_envelope, 35.0))
    realtime_floor = max(1.0e-5, percentile(realtime_envelope, 35.0))
    return [
        offline_envelope[index] > offline_floor and realtime_envelope[index] > realtime_floor
        for index in range(count)
    ]


def channel_passed(metrics: dict[str, Any], thresholds: dict[str, float]) -> bool:
    return bool(
        metrics["offline_rms"] >= thresholds["min_rms"]
        and metrics["realtime_aligned_rms"] >= thresholds["min_rms"]
        and metrics["offline_peak"] <= thresholds["max_peak_abs"]
        and metrics["realtime_aligned_peak"] <= thresholds["max_peak_abs"]
        and abs(metrics["rms_delta_db"]) <= thresholds["max_rms_delta_db_abs"]
        and abs(metrics["peak_delta_db"]) <= thresholds["max_peak_delta_db_abs"]
        and metrics["content_envelope_correlation"] >= thresholds["min_content_envelope_correlation"]
        and metrics["filtered_band_correlation_mean"] >= thresholds["min_filtered_band_correlation_mean"]
        and metrics["filtered_band_correlation_min"] >= thresholds["min_filtered_band_correlation_floor"]
        and metrics["frame_band_cosine_mean"] >= thresholds["min_frame_band_cosine_mean"]
    )


def compare_aligned_channels(
    offline_channels: list[list[float]],
    aligned_channels: list[list[float]],
    sample_rate: int,
    thresholds: dict[str, float],
) -> dict[str, Any]:
    if len(offline_channels) != len(aligned_channels):
        fail("Aligned channel count mismatch")
    channel_reports = [
        channel_metrics(offline_channel, aligned_channel, sample_rate, thresholds)
        for offline_channel, aligned_channel in zip(offline_channels, aligned_channels)
    ]
    return {
        "channel_count": len(channel_reports),
        "channels": channel_reports,
        "content_envelope_correlation_min": min(report["content_envelope_correlation"] for report in channel_reports),
        "filtered_band_correlation_mean_min": min(report["filtered_band_correlation_mean"] for report in channel_reports),
        "filtered_band_correlation_floor_min": min(report["filtered_band_correlation_min"] for report in channel_reports),
        "frame_band_cosine_mean_min": min(report["frame_band_cosine_mean"] for report in channel_reports),
        "passed": all(report["passed"] for report in channel_reports),
    }


def compare_audio(
    offline_channels: list[list[float]],
    realtime_channels: list[list[float]],
    sample_rate: int,
    thresholds: dict[str, float],
) -> dict[str, Any]:
    if len(offline_channels) != len(realtime_channels):
        fail(f"Channel count mismatch: offline {len(offline_channels)}, realtime {len(realtime_channels)}")

    alignment = align_by_envelope(mixed_channel(offline_channels), mixed_channel(realtime_channels))
    offset = alignment["offset_frames"]
    aligned_channels = [channel[offset : offset + len(offline_channels[index])] for index, channel in enumerate(realtime_channels)]
    actual = compare_aligned_channels(offline_channels, aligned_channels, sample_rate, thresholds)

    negative_controls = build_negative_controls(offline_channels, aligned_channels, sample_rate, thresholds)
    negatives_failed = all(not control["passed"] for control in negative_controls.values())
    passed = bool(
        alignment["envelope_correlation"] >= thresholds["min_alignment_envelope_correlation"]
        and actual["passed"]
        and negatives_failed
    )

    return {
        "schema_version": 1,
        "suite": "ableton-strong-bounce-realtime-compare",
        "sample_rate": sample_rate,
        "thresholds": thresholds,
        "alignment": alignment,
        "actual": actual,
        "negative_controls": negative_controls,
        "negative_controls_failed": negatives_failed,
        "passed": passed,
        "comparison_mode": "stereo_envelope_alignment_plus_per_channel_filtered_band_content_match",
        "notes": (
            "This is not a strict waveform/null test. It rejects same-loudness and stereo-specific "
            "mismatches by requiring per-channel amplitude-envelope and filtered-band-envelope agreement."
        ),
    }


def build_negative_controls(
    offline_channels: list[list[float]],
    aligned_channels: list[list[float]],
    sample_rate: int,
    thresholds: dict[str, float],
) -> dict[str, dict[str, Any]]:
    controls: dict[str, list[list[float]]] = {
        "wrong_offset": wrong_offset_control(aligned_channels, sample_rate),
        "reversed_realtime": [list(reversed(channel)) for channel in aligned_channels],
        "same_rms_sine": sine_control(offline_channels, sample_rate),
        "same_rms_noise": noise_control(offline_channels),
    }

    if len(aligned_channels) > 1:
        controls["silent_last_channel"] = [list(channel) for channel in aligned_channels]
        controls["silent_last_channel"][-1] = [0.0 for _ in controls["silent_last_channel"][-1]]

    return {
        name: compare_aligned_channels(offline_channels, control_channels, sample_rate, thresholds)
        for name, control_channels in controls.items()
    }


def wrong_offset_control(channels: list[list[float]], sample_rate: int) -> list[list[float]]:
    leading_count = min(sample_rate, max(1, min(len(channel) for channel in channels) // 4))
    return [([0.0] * leading_count + channel)[: len(channel)] for channel in channels]


def sine_control(channels: list[list[float]], sample_rate: int) -> list[list[float]]:
    output = []
    for channel_index, offline_channel in enumerate(channels):
        timeline = range(len(offline_channel))
        frequency = 220.0 * (channel_index + 1)
        sine = [math.sin(2.0 * math.pi * frequency * frame / sample_rate) for frame in timeline]
        output.append(normalize_rms(sine, rms(offline_channel)))
    return output


def noise_control(channels: list[list[float]]) -> list[list[float]]:
    rng = random.Random(7)
    return [
        normalize_rms([rng.uniform(-1.0, 1.0) for _ in range(len(channel))], rms(channel))
        for channel in channels
    ]


def normalize_rms(samples: list[float], target_rms: float) -> list[float]:
    current = rms(samples)
    if current <= 0.0:
        return samples
    scale = target_rms / current
    return [sample * scale for sample in samples]


def run_file_compare(args: argparse.Namespace) -> int:
    thresholds = thresholds_from_args(args)
    offline_rate, offline_channels = read_audio(Path(args.offline))
    realtime_rate, realtime_channels = read_audio(Path(args.realtime))
    if offline_rate != realtime_rate:
        fail(f"Sample-rate mismatch: offline {offline_rate}, realtime {realtime_rate}")

    report = compare_audio(offline_channels, realtime_channels, offline_rate, thresholds)
    report["offline_file"] = args.offline
    report["realtime_file"] = args.realtime
    write_report(Path(args.output), report)
    print(f"wrote {args.output}")
    print(f"passed: {str(report['passed']).lower()}")
    return 0 if report["passed"] else 1


def run_self_test(args: argparse.Namespace) -> int:
    sample_rate = 44100
    thresholds = dict(DEFAULT_THRESHOLDS)
    offline_channels = make_self_test_signal(sample_rate)
    leading = [0.0] * (32 * DEFAULT_HOP_SIZE)
    rng = random.Random(3)
    realtime_channels = [
        leading
        + normalize_rms(
            [sample * 0.985 + rng.uniform(-0.0005, 0.0005) for sample in channel],
            rms(channel),
        )
        for channel in offline_channels
    ]
    report = compare_audio(offline_channels, realtime_channels, sample_rate, thresholds)
    report["self_test"] = True
    if args.output:
        write_report(Path(args.output), report)
    else:
        print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if report["passed"] else 1


def make_self_test_signal(sample_rate: int) -> list[list[float]]:
    duration_seconds = 4.0
    frame_count = int(sample_rate * duration_seconds)
    left = []
    right = []
    for frame in range(frame_count):
        time = frame / sample_rate
        phase = time % 0.5
        gate = 1.0 if phase < 0.32 else 0.0
        attack = min(1.0, phase / 0.025)
        release = min(1.0, (0.5 - phase) / 0.18)
        amp = 0.25 * gate * min(attack, release)
        left_value = amp * (
            0.55 * math.sin(2.0 * math.pi * 110.0 * time)
            + 0.35 * math.sin(2.0 * math.pi * 220.0 * time + 0.2)
            + 0.18 * math.sin(2.0 * math.pi * 440.0 * time + 0.5)
            + 0.08 * math.sin(2.0 * math.pi * 880.0 * time + 0.8)
        )
        right_value = amp * (
            0.48 * math.sin(2.0 * math.pi * 110.0 * time + 0.1)
            + 0.38 * math.sin(2.0 * math.pi * 220.0 * time + 0.35)
            + 0.20 * math.sin(2.0 * math.pi * 660.0 * time + 0.7)
            + 0.07 * math.sin(2.0 * math.pi * 1320.0 * time + 0.9)
        )
        left.append(left_value)
        right.append(right_value)
    return [left, right]


def thresholds_from_args(args: argparse.Namespace) -> dict[str, float]:
    thresholds = dict(DEFAULT_THRESHOLDS)
    thresholds.update(
        {
            "min_alignment_envelope_correlation": args.min_alignment_envelope_correlation,
            "min_content_envelope_correlation": args.min_content_envelope_correlation,
            "min_filtered_band_correlation_mean": args.min_filtered_band_correlation_mean,
            "min_filtered_band_correlation_floor": args.min_filtered_band_correlation_floor,
            "min_frame_band_cosine_mean": args.min_frame_band_cosine_mean,
            "max_rms_delta_db_abs": args.max_rms_delta_db_abs,
            "max_peak_delta_db_abs": args.max_peak_delta_db_abs,
        }
    )
    return thresholds


def write_report(path: Path, report: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--offline", help="Ableton offline bounce WAV path")
    parser.add_argument("--realtime", help="Ableton realtime resampling AIFF/WAV path")
    parser.add_argument("--output", help="JSON report output path")
    parser.add_argument("--self-test", action="store_true", help="Run synthetic positive/negative metric self-test")
    parser.add_argument(
        "--min-alignment-envelope-correlation",
        type=float,
        default=DEFAULT_THRESHOLDS["min_alignment_envelope_correlation"],
    )
    parser.add_argument(
        "--min-content-envelope-correlation",
        type=float,
        default=DEFAULT_THRESHOLDS["min_content_envelope_correlation"],
    )
    parser.add_argument(
        "--min-filtered-band-correlation-mean",
        type=float,
        default=DEFAULT_THRESHOLDS["min_filtered_band_correlation_mean"],
    )
    parser.add_argument(
        "--min-filtered-band-correlation-floor",
        type=float,
        default=DEFAULT_THRESHOLDS["min_filtered_band_correlation_floor"],
    )
    parser.add_argument(
        "--min-frame-band-cosine-mean",
        type=float,
        default=DEFAULT_THRESHOLDS["min_frame_band_cosine_mean"],
    )
    parser.add_argument("--max-rms-delta-db-abs", type=float, default=DEFAULT_THRESHOLDS["max_rms_delta_db_abs"])
    parser.add_argument("--max-peak-delta-db-abs", type=float, default=DEFAULT_THRESHOLDS["max_peak_delta_db_abs"])
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    if args.self_test:
        return run_self_test(args)
    missing = [name for name in ("offline", "realtime", "output") if not getattr(args, name)]
    if missing:
        parser.error("--offline, --realtime, and --output are required unless --self-test is used")
    return run_file_compare(args)


if __name__ == "__main__":
    raise SystemExit(main())
