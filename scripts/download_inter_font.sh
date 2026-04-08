#!/bin/bash
# Download Inter font files for Lumen.
# Run from the repository root: ./scripts/download_inter_font.sh

set -euo pipefail

FONT_DIR="resources/fonts"
ZIP_URL="https://github.com/rsms/inter/releases/download/v4.1/Inter-4.1.zip"
TMPDIR=$(mktemp -d)

echo "Downloading Inter font v4.1..."
wget -q -O "$TMPDIR/Inter-4.1.zip" "$ZIP_URL" \
    || curl -sL -o "$TMPDIR/Inter-4.1.zip" "$ZIP_URL"

echo "Extracting needed files..."
mkdir -p "$FONT_DIR"

unzip -o -j "$TMPDIR/Inter-4.1.zip" \
    "Inter-4.1/InterDesktop/Inter-Regular.ttf" \
    "Inter-4.1/InterDesktop/Inter-Medium.ttf" \
    "Inter-4.1/InterDesktop/Inter-SemiBold.ttf" \
    "Inter-4.1/InterDesktop/Inter-Bold.ttf" \
    "Inter-4.1/LICENSE.txt" \
    -d "$FONT_DIR" 2>/dev/null \
|| unzip -o -j "$TMPDIR/Inter-4.1.zip" \
    "InterDesktop/Inter-Regular.ttf" \
    "InterDesktop/Inter-Medium.ttf" \
    "InterDesktop/Inter-SemiBold.ttf" \
    "InterDesktop/Inter-Bold.ttf" \
    "LICENSE.txt" \
    -d "$FONT_DIR" 2>/dev/null \
|| {
    # Try to find the files in any location within the zip
    unzip -l "$TMPDIR/Inter-4.1.zip" | grep -E '(Inter-(Regular|Medium|SemiBold|Bold)\.ttf|LICENSE\.txt)' | awk '{print $NF}' | while read -r f; do
        unzip -o -j "$TMPDIR/Inter-4.1.zip" "$f" -d "$FONT_DIR"
    done
}

# Rename LICENSE.txt to OFL.txt if needed
if [ -f "$FONT_DIR/LICENSE.txt" ] && [ ! -f "$FONT_DIR/OFL.txt" ]; then
    mv "$FONT_DIR/LICENSE.txt" "$FONT_DIR/OFL.txt"
fi

rm -rf "$TMPDIR"

echo ""
echo "Fonts installed to $FONT_DIR:"
ls -la "$FONT_DIR"
echo ""
echo "Done."
