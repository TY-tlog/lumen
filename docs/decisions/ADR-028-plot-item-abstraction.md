# ADR-028: PlotItem abstraction with virtual paint and hit-test

## Status
Accepted (Phase 5)

## Context
Phases 2-4 support only line plots via LineSeries. Phase 5 adds
scatter and bar plots. The rendering, hit-testing, and editing
pipelines currently assume LineSeries everywhere. We need an
abstraction that allows multiple plot types to coexist in one
PlotScene and participate in all existing flows (rendering,
hit-testing, editing, saving, exporting) without type-specific
dispatch in every call site.

## Decision
Introduce PlotItem as an abstract base class in src/lumen/plot/:

```cpp
class PlotItem {
public:
    enum class Type { Line, Scatter, Bar };
    virtual ~PlotItem() = default;
    virtual Type type() const = 0;
    virtual QRectF dataBounds() const = 0;
    virtual void paint(QPainter*, const CoordinateMapper&,
                       const QRectF& plotArea) const = 0;
    virtual bool isVisible() const = 0;
    virtual QString name() const = 0;
    virtual QColor primaryColor() const = 0;
};
```

LineSeries inherits from PlotItem and implements all virtuals.
ScatterSeries and BarSeries do the same. PlotScene holds
`vector<unique_ptr<PlotItem>>`. PlotRenderer iterates items and
calls `item->paint()` — one code path for all types, preserving
ADR-026's single-rendering-path principle.

Type-specific properties (line width, marker shape, bar width)
stay on the concrete classes, not on PlotItem. The dialog dispatch
uses `PlotItem::type()` to pick the right dialog.

## Consequences
- + Polymorphic rendering: no type switch in PlotRenderer
- + New types added by creating a new subclass, not modifying
  existing code (open/closed principle)
- + All existing flows (rendering, hit-test, save, export) work
  through PlotItem without knowing the concrete type
- + Legend, auto-range, and visibility all work through the base
- - Virtual dispatch overhead (~1 ns per item per frame). Negligible.
- - LineSeries refactor in Phase 5.1 is a large mechanical change
  that must preserve bit-identical behavior.

## Alternatives considered
- **std::variant<LineSeries, ScatterSeries, BarSeries>**: type-safe
  sum type with std::visit for dispatch. Rejected: adding a new
  type requires modifying the variant definition and every visit
  site. Virtual dispatch is cleaner for an open set of types.
- **PlotItem as pure data + separate Renderer per type**: PlotItem
  holds data only, each type has a dedicated renderer. Rejected:
  scatters rendering logic across files, makes it harder to
  maintain the single-code-path invariant from ADR-026. Self-
  rendering via paint() is simpler.
