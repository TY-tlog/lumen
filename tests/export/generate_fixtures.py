#!/usr/bin/env python3
"""Generate 12 deterministic source fixtures for vector consistency CI.

Each fixture is a .lumen.json file that can be rendered by FigureExporter.
All data is mathematically generated (no RNG or external files).

Usage: python3 tests/export/generate_fixtures.py
Output: tests/export/fixtures/source/*.lumen.json
"""

import json
import math
import os

OUT_DIR = os.path.join(os.path.dirname(__file__), "fixtures", "source")


def write_fixture(name: str, data: dict) -> None:
    os.makedirs(OUT_DIR, exist_ok=True)
    path = os.path.join(OUT_DIR, f"{name}.lumen.json")
    with open(path, "w") as f:
        json.dump(data, f, indent=2)
    print(f"  wrote {path}")


def linspace(start, stop, n):
    return [start + (stop - start) * i / (n - 1) for i in range(n)]


def make_line_simple():
    N = 200
    x = linspace(0, 4 * math.pi, N)
    return {
        "version": 1, "fixture": True,
        "plot": {
            "viewport": {"xmin": 0, "xmax": 4 * math.pi, "ymin": -1.2, "ymax": 1.2},
            "title": {"text": "Three Sine Waves", "fontPx": 17, "weight": 63},
            "xAxis": {"label": "x (rad)", "rangeMode": "manual"},
            "yAxis": {"label": "Amplitude"},
            "legend": {"position": "top_right", "visible": True},
            "series": [
                {"type": "line", "name": "sin(x)", "color": "#0a84ff", "lineWidth": 1.5,
                 "xColumn": "x", "yColumn": "y1", "visible": True},
                {"type": "line", "name": "sin(x+π/3)", "color": "#30d158", "lineWidth": 1.5,
                 "xColumn": "x", "yColumn": "y2", "visible": True},
                {"type": "line", "name": "sin(x+2π/3)", "color": "#ff453a", "lineWidth": 1.5,
                 "xColumn": "x", "yColumn": "y3", "visible": True},
            ],
            "_data": {
                "x": x,
                "y1": [math.sin(v) for v in x],
                "y2": [math.sin(v + math.pi / 3) for v in x],
                "y3": [math.sin(v + 2 * math.pi / 3) for v in x],
            }
        }
    }


def make_line_log():
    N = 100
    x = linspace(0, 20, N)
    return {
        "version": 1, "fixture": True,
        "plot": {
            "viewport": {"xmin": 0, "xmax": 20, "ymin": 0.001, "ymax": 1.5},
            "title": {"text": "Exponential Decay (log scale)", "fontPx": 17, "weight": 63},
            "xAxis": {"label": "Time (s)"},
            "yAxis": {"label": "Amplitude (log)"},
            "legend": {"position": "top_right", "visible": True},
            "series": [
                {"type": "line", "name": "τ=1", "color": "#0a84ff", "lineWidth": 1.5,
                 "xColumn": "x", "yColumn": "y1", "visible": True},
                {"type": "line", "name": "τ=2", "color": "#30d158", "lineWidth": 1.5,
                 "xColumn": "x", "yColumn": "y2", "visible": True},
                {"type": "line", "name": "τ=5", "color": "#ff453a", "lineWidth": 1.5,
                 "xColumn": "x", "yColumn": "y3", "visible": True},
            ],
            "_data": {
                "x": x,
                "y1": [math.exp(-v / 1) for v in x],
                "y2": [math.exp(-v / 2) for v in x],
                "y3": [math.exp(-v / 5) for v in x],
            }
        }
    }


def make_scatter_color():
    N = 200
    # Deterministic "random" via sin/cos combination
    xs = [math.sin(i * 0.1) * 2 + math.cos(i * 0.07) for i in range(N)]
    ys = [math.cos(i * 0.1) * 2 + math.sin(i * 0.07) for i in range(N)]
    return {
        "version": 1, "fixture": True,
        "plot": {
            "viewport": {"xmin": -4, "xmax": 4, "ymin": -4, "ymax": 4},
            "title": {"text": "Scatter (colored by radius)", "fontPx": 17, "weight": 63},
            "xAxis": {"label": "X"}, "yAxis": {"label": "Y"},
            "legend": {"visible": False},
            "series": [
                {"type": "scatter", "name": "points", "color": "#5e5ce6",
                 "xColumn": "x", "yColumn": "y", "visible": True}
            ],
            "_data": {"x": xs, "y": ys}
        }
    }


def make_bar_grouped():
    return {
        "version": 1, "fixture": True,
        "plot": {
            "viewport": {"xmin": -0.5, "xmax": 3.5, "ymin": 0, "ymax": 10},
            "title": {"text": "Grouped Bar Chart", "fontPx": 17, "weight": 63},
            "xAxis": {"label": "Category"}, "yAxis": {"label": "Value"},
            "legend": {"position": "top_right", "visible": True},
            "series": [
                {"type": "bar", "name": "Group A", "color": "#0a84ff",
                 "xColumn": "x", "yColumn": "a", "visible": True},
                {"type": "bar", "name": "Group B", "color": "#30d158",
                 "xColumn": "x", "yColumn": "b", "visible": True},
                {"type": "bar", "name": "Group C", "color": "#ff453a",
                 "xColumn": "x", "yColumn": "c", "visible": True},
            ],
            "_data": {
                "x": [0, 1, 2, 3],
                "a": [3, 7, 5, 2], "b": [4, 6, 8, 3], "c": [5, 4, 3, 6]
            }
        }
    }


def make_heatmap_viridis():
    N = 64
    sigma = 20
    center = 32
    vals = []
    for y in range(N):
        for x in range(N):
            dx, dy = x - center, y - center
            vals.append(math.exp(-(dx*dx + dy*dy) / (2*sigma*sigma)))
    return {
        "version": 1, "fixture": True,
        "plot": {
            "viewport": {"xmin": 0, "xmax": 63, "ymin": 0, "ymax": 63},
            "title": {"text": "Gaussian Heatmap (Viridis)", "fontPx": 17, "weight": 63},
            "xAxis": {"label": "X"}, "yAxis": {"label": "Y"},
            "legend": {"visible": False},
            "series": [{"type": "heatmap", "name": "gaussian", "colormap": "viridis",
                        "width": N, "height": N, "visible": True}],
            "_data": {"values": vals}
        }
    }


def make_heatmap_diverging():
    N = 64
    vals = []
    for y in range(N):
        for x in range(N):
            xn = (x - 32) / 16.0
            yn = (y - 32) / 16.0
            vals.append(xn*xn - yn*yn)
    return {
        "version": 1, "fixture": True,
        "plot": {
            "viewport": {"xmin": 0, "xmax": 63, "ymin": 0, "ymax": 63},
            "title": {"text": "Saddle Function (diverging)", "fontPx": 17, "weight": 63},
            "xAxis": {"label": "X"}, "yAxis": {"label": "Y"},
            "legend": {"visible": False},
            "series": [{"type": "heatmap", "name": "saddle", "colormap": "coolwarm",
                        "width": N, "height": N, "visible": True}],
            "_data": {"values": vals}
        }
    }


def make_contour_filled():
    N = 32
    vals = []
    for y in range(N):
        for x in range(N):
            xn = (x - 16) / 5.0
            yn = (y - 16) / 5.0
            # Peaks-like function
            v = (3*(1-xn)**2 * math.exp(-(xn**2) - (yn+1)**2)
                 - 10*(xn/5 - xn**3 - yn**5) * math.exp(-xn**2 - yn**2)
                 - 1/3 * math.exp(-(xn+1)**2 - yn**2))
            vals.append(v)
    return {
        "version": 1, "fixture": True,
        "plot": {
            "viewport": {"xmin": 0, "xmax": 31, "ymin": 0, "ymax": 31},
            "title": {"text": "Filled Contour (peaks)", "fontPx": 17, "weight": 63},
            "xAxis": {"label": "X"}, "yAxis": {"label": "Y"},
            "series": [{"type": "contour", "name": "peaks", "levels": 10,
                        "filled": True, "width": N, "height": N, "visible": True}],
            "_data": {"values": vals}
        }
    }


def make_contour_lines():
    N = 32
    vals = []
    for y in range(N):
        for x in range(N):
            xn = (x - 16) / 8.0
            yn = (y - 16) / 8.0
            v = (1-xn)**2 + 100*(yn - xn**2)**2  # Rosenbrock
            vals.append(min(v, 500))  # clamp
    return {
        "version": 1, "fixture": True,
        "plot": {
            "viewport": {"xmin": 0, "xmax": 31, "ymin": 0, "ymax": 31},
            "title": {"text": "Line Contour (Rosenbrock)", "fontPx": 17, "weight": 63},
            "xAxis": {"label": "X"}, "yAxis": {"label": "Y"},
            "series": [{"type": "contour", "name": "rosenbrock", "levels": 8,
                        "filled": False, "width": N, "height": N, "visible": True}],
            "_data": {"values": vals}
        }
    }


def make_errorbar():
    N = 10
    x = list(range(N))
    y = [2.0 + 0.5 * i + 0.3 * math.sin(i) for i in range(N)]
    err = [0.3 + 0.1 * abs(math.sin(i * 0.7)) for i in range(N)]
    return {
        "version": 1, "fixture": True,
        "plot": {
            "viewport": {"xmin": -0.5, "xmax": 9.5, "ymin": 0, "ymax": 8},
            "title": {"text": "Error Bar Plot", "fontPx": 17, "weight": 63},
            "xAxis": {"label": "Sample"}, "yAxis": {"label": "Measurement"},
            "series": [{"type": "scatter", "name": "data", "color": "#0a84ff",
                        "xColumn": "x", "yColumn": "y", "visible": True}],
            "_data": {"x": x, "y": y, "error": err}
        }
    }


def make_stat_violin():
    # 3 groups of deterministic "distributions"
    N = 100
    g1 = [math.sin(i * 0.1) * 2 + 5 for i in range(N)]
    g2 = [math.cos(i * 0.07) * 3 + 8 for i in range(N)]
    g3 = [math.sin(i * 0.13) * 1.5 + 3 for i in range(N)]
    return {
        "version": 1, "fixture": True,
        "plot": {
            "viewport": {"xmin": -0.5, "xmax": 2.5, "ymin": 0, "ymax": 12},
            "title": {"text": "Violin Plot", "fontPx": 17, "weight": 63},
            "xAxis": {"label": "Group"}, "yAxis": {"label": "Value"},
            "series": [
                {"type": "violin", "name": "Group 1", "color": "#0a84ff", "visible": True},
                {"type": "violin", "name": "Group 2", "color": "#30d158", "visible": True},
                {"type": "violin", "name": "Group 3", "color": "#ff453a", "visible": True},
            ],
            "_data": {"g1": g1, "g2": g2, "g3": g3}
        }
    }


def make_plot3d_surface():
    # Rendered as 2D heatmap fallback for vector consistency
    N = 32
    vals = []
    for y in range(N):
        for x in range(N):
            xn = (x - 16) / 8.0
            yn = (y - 16) / 8.0
            r = math.sqrt(xn*xn + yn*yn) + 1e-10
            vals.append(math.sin(r * math.pi) / r)
    return {
        "version": 1, "fixture": True,
        "plot": {
            "viewport": {"xmin": 0, "xmax": 31, "ymin": 0, "ymax": 31},
            "title": {"text": "Surface3D (sinc, 2D fallback)", "fontPx": 17, "weight": 63},
            "xAxis": {"label": "X"}, "yAxis": {"label": "Y"},
            "series": [{"type": "heatmap", "name": "sinc", "colormap": "viridis",
                        "width": N, "height": N, "visible": True}],
            "_data": {"values": vals}
        }
    }


def make_plot3d_scatter():
    # Rendered as 2D XY scatter fallback
    N = 100
    xs = [math.sin(i * 0.1) * math.cos(i * 0.07) for i in range(N)]
    ys = [math.sin(i * 0.1) * math.sin(i * 0.07) for i in range(N)]
    return {
        "version": 1, "fixture": True,
        "plot": {
            "viewport": {"xmin": -1.5, "xmax": 1.5, "ymin": -1.5, "ymax": 1.5},
            "title": {"text": "Scatter3D (sphere, XY projection)", "fontPx": 17, "weight": 63},
            "xAxis": {"label": "X"}, "yAxis": {"label": "Y"},
            "series": [{"type": "scatter", "name": "sphere", "color": "#5e5ce6",
                        "xColumn": "x", "yColumn": "y", "visible": True}],
            "_data": {"x": xs, "y": ys}
        }
    }


FIXTURES = {
    "line_simple": make_line_simple,
    "line_log": make_line_log,
    "scatter_color": make_scatter_color,
    "bar_grouped": make_bar_grouped,
    "heatmap_viridis": make_heatmap_viridis,
    "heatmap_diverging": make_heatmap_diverging,
    "contour_filled": make_contour_filled,
    "contour_lines": make_contour_lines,
    "errorbar": make_errorbar,
    "stat_violin": make_stat_violin,
    "plot3d_surface": make_plot3d_surface,
    "plot3d_scatter": make_plot3d_scatter,
}

if __name__ == "__main__":
    print(f"Generating {len(FIXTURES)} fixtures...")
    for name, fn in FIXTURES.items():
        write_fixture(name, fn())
    print("Done.")
