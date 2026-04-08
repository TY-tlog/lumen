# Lumen Design System v0

Apple-mood, restrained, scientific. This document is the source of
truth; all `src/lumen/style/` code derives its values from here.

## Color tokens

### Light palette
- background.primary    : #FFFFFF
- background.secondary  : #F5F5F7
- surface.elevated      : #FFFFFF (with shadow.md)
- surface.sunken        : #ECECEE
- border.subtle         : #E5E5EA
- border.strong         : #D1D1D6
- text.primary          : #1D1D1F
- text.secondary        : #636366
- text.tertiary         : #8E8E93
- accent.primary        : #0A84FF
- accent.muted          : #E5F0FF
- success               : #30D158
- warning               : #FF9F0A
- error                 : #FF3B30

### Dark palette
- background.primary    : #1C1C1E
- background.secondary  : #2C2C2E
- surface.elevated      : #2C2C2E
- surface.sunken        : #1C1C1E
- border.subtle         : #38383A
- border.strong         : #48484A
- text.primary          : #F2F2F7
- text.secondary        : #AEAEB2
- text.tertiary         : #8E8E93
- accent.primary        : #0A84FF
- accent.muted          : #1F3A5F
- success               : #30D158
- warning               : #FF9F0A
- error                 : #FF453A

## Typography

Font stack:
- macOS: -apple-system, "SF Pro Text", "SF Pro Display"
- Linux / fallback: "Inter", "Helvetica Neue", "Helvetica", sans-serif

Scale (px):
- caption     : 11, regular, line-height 14
- footnote    : 12, regular, line-height 16
- body        : 13, regular, line-height 18
- body-strong : 13, medium,  line-height 18
- subhead     : 15, medium,  line-height 20
- title-3     : 17, semibold, line-height 22
- title-2     : 22, semibold, line-height 28
- title-1     : 28, bold,    line-height 34

Letter spacing: -0.01em on titles, 0 on body.

## Spacing

4-unit baseline grid: 4, 8, 12, 16, 24, 32, 48, 64.
Default container padding: 16. Default gap between sections: 24.

## Radii

- xs: 4   (chips, small badges)
- sm: 8   (buttons, inputs)
- md: 12  (cards, dialogs)
- lg: 16  (panels)

## Shadows

- sm: 0 1px 2px rgba(0,0,0,0.04), 0 1px 1px rgba(0,0,0,0.06)
- md: 0 4px 12px rgba(0,0,0,0.08), 0 2px 4px rgba(0,0,0,0.06)
- lg: 0 12px 32px rgba(0,0,0,0.12), 0 4px 8px rgba(0,0,0,0.08)
- xl: 0 24px 64px rgba(0,0,0,0.16), 0 8px 16px rgba(0,0,0,0.10)

## Motion

- duration.fast    : 120 ms
- duration.normal  : 200 ms
- duration.slow    : 320 ms
- easing.standard  : cubic-bezier(0.2, 0.0, 0.0, 1.0)
- easing.emphasized: cubic-bezier(0.3, 0.0, 0.0, 1.0)

## Plot palette (categorical, color-blind-friendly)

1. #0A84FF  blue
2. #FF9F0A  orange
3. #30D158  green
4. #FF375F  red
5. #BF5AF2  purple
6. #5E5CE6  indigo
7. #64D2FF  cyan
8. #FFD60A  yellow

Default line width: 1.5 px. Marker size: 5 px.
Axis line: 1 px, border.strong. Grid: 1 px, border.subtle, dashed.

## Application shell

- Window background: background.secondary
- Dock title bar: surface.elevated, text.primary, body-strong
- Dock body: background.primary
- Toolbar: surface.elevated, 44 px tall
- Status bar: surface.sunken, footnote text
