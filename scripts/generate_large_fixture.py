#!/usr/bin/env python3
"""Generate a large CSV fixture (100,000 rows, 5 columns) for performance testing.

Output: tests/fixtures/tiny/large_100k.csv
"""

import csv
import math
import os
import random
import sys

ROWS = 100_000
COLS = ["col_a", "col_b", "col_c", "col_d", "col_e"]
NAN_PROBABILITY = 0.02  # ~2% of cells will be NaN

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
OUTPUT_PATH = os.path.join(PROJECT_ROOT, "tests", "fixtures", "tiny", "large_100k.csv")


def main() -> None:
    random.seed(42)  # reproducible output

    os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)

    with open(OUTPUT_PATH, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(COLS)

        for _ in range(ROWS):
            row = []
            for _ in COLS:
                if random.random() < NAN_PROBABILITY:
                    row.append("NaN")
                else:
                    row.append(f"{random.gauss(0, 100):.6f}")
            writer.writerow(row)

    # Verify
    with open(OUTPUT_PATH) as f:
        line_count = sum(1 for _ in f)

    expected = ROWS + 1  # header + data rows
    if line_count != expected:
        print(f"ERROR: expected {expected} lines, got {line_count}", file=sys.stderr)
        sys.exit(1)

    size_mb = os.path.getsize(OUTPUT_PATH) / (1024 * 1024)
    print(f"Generated {OUTPUT_PATH}")
    print(f"  {ROWS} data rows, {len(COLS)} columns, ~{size_mb:.1f} MB")
    print(f"  NaN probability: {NAN_PROBABILITY * 100:.0f}%")


if __name__ == "__main__":
    main()
