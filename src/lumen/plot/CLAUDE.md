# plot/

Owner: Frontend.
Responsibilities: self-built plot engine. Coordinate transforms,
axes, line/scatter/marker rendering on QPainter, hit-testing,
interaction state machines, plot object model exposed to the
property inspector.
Forbidden: external plot libraries (QCustomPlot, Qt Charts, Qwt).
QPainter and Qt graphics primitives only.
