#!/usr/bin/env python3
"""3-tier metric comparison for vector consistency CI (ADR-056).

Computes MS-SSIM, PSNR, and CIEDE2000 between two PNG images.
Returns JSON report with gate pass/fail decision.

Usage: python3 compare.py reference.png actual.png [--json]

Requires: pip install scikit-image numpy colour-science Pillow
"""

import json
import sys

import numpy as np
from PIL import Image


# Gate thresholds (spec §3.3, Decision D1)
MSSSIM_THRESHOLD = 0.97
PSNR_THRESHOLD = 35.0
CIEDE2000_MEAN_THRESHOLD = 2.0
CIEDE2000_MAX_THRESHOLD = 5.0


def load_image(path: str) -> np.ndarray:
    """Load PNG as RGB uint8 numpy array."""
    img = Image.open(path).convert("RGB")
    return np.array(img, dtype=np.uint8)


def compute_psnr(ref: np.ndarray, actual: np.ndarray) -> float:
    """Channel-averaged PSNR in dB."""
    mse = np.mean((ref.astype(np.float64) - actual.astype(np.float64)) ** 2)
    if mse < 1e-10:
        return 100.0  # identical
    return 10.0 * np.log10(255.0 ** 2 / mse)


def compute_msssim(ref: np.ndarray, actual: np.ndarray) -> float:
    """Multi-scale SSIM via scikit-image."""
    from skimage.metrics import structural_similarity
    # Convert to grayscale for SSIM (channel_axis for color would work too).
    ref_gray = np.mean(ref.astype(np.float64), axis=2)
    act_gray = np.mean(actual.astype(np.float64), axis=2)
    # Use data_range for uint8
    return structural_similarity(ref_gray, act_gray, data_range=255.0)


def compute_ciede2000(ref: np.ndarray, actual: np.ndarray) -> tuple:
    """CIEDE2000 mean and max over all pixels."""
    try:
        import colour
    except ImportError:
        return 0.0, 0.0  # Skip if colour-science not installed

    # Convert sRGB [0,255] to [0,1] float
    ref_f = ref.astype(np.float64) / 255.0
    act_f = actual.astype(np.float64) / 255.0

    # Reshape to (N, 3) for batch conversion
    shape = ref_f.shape[:2]
    ref_flat = ref_f.reshape(-1, 3)
    act_flat = act_f.reshape(-1, 3)

    # sRGB → XYZ → Lab
    ref_xyz = colour.sRGB_to_XYZ(ref_flat)
    act_xyz = colour.sRGB_to_XYZ(act_flat)
    ref_lab = colour.XYZ_to_Lab(ref_xyz)
    act_lab = colour.XYZ_to_Lab(act_xyz)

    # CIEDE2000
    delta_e = colour.delta_E(ref_lab, act_lab, method="CIE 2000")

    return float(np.mean(delta_e)), float(np.max(delta_e))


def compare(ref_path: str, actual_path: str) -> dict:
    """Compare two images and return metric report."""
    ref = load_image(ref_path)
    actual = load_image(actual_path)

    # Resize actual to match reference if dimensions differ.
    if ref.shape != actual.shape:
        actual_img = Image.fromarray(actual).resize(
            (ref.shape[1], ref.shape[0]), Image.LANCZOS)
        actual = np.array(actual_img, dtype=np.uint8)

    psnr = compute_psnr(ref, actual)
    msssim = compute_msssim(ref, actual)
    ciede_mean, ciede_max = compute_ciede2000(ref, actual)

    gate_pass = (
        msssim >= MSSSIM_THRESHOLD
        and psnr >= PSNR_THRESHOLD
        and ciede_mean <= CIEDE2000_MEAN_THRESHOLD
        and ciede_max <= CIEDE2000_MAX_THRESHOLD
    )

    return {
        "ms_ssim": round(msssim, 6),
        "psnr_db": round(psnr, 2),
        "ciede2000_mean": round(ciede_mean, 4),
        "ciede2000_max": round(ciede_max, 4),
        "gate_pass": gate_pass,
        "thresholds": {
            "ms_ssim": MSSSIM_THRESHOLD,
            "psnr_db": PSNR_THRESHOLD,
            "ciede2000_mean": CIEDE2000_MEAN_THRESHOLD,
            "ciede2000_max": CIEDE2000_MAX_THRESHOLD,
        }
    }


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} reference.png actual.png [--json]")
        sys.exit(1)

    result = compare(sys.argv[1], sys.argv[2])

    if "--json" in sys.argv:
        print(json.dumps(result, indent=2))
    else:
        status = "PASS" if result["gate_pass"] else "FAIL"
        print(f"[{status}] MS-SSIM={result['ms_ssim']:.4f} "
              f"PSNR={result['psnr_db']:.1f}dB "
              f"CIEDE2000 mean={result['ciede2000_mean']:.3f} "
              f"max={result['ciede2000_max']:.3f}")

    sys.exit(0 if result["gate_pass"] else 1)
