"""
Plot the base Fed Funds futures curve and the two zero-rate spread curves.

Run SpreadCurve first to generate the CSV files, then:
    python plot_curves.py
"""

import csv
import subprocess
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
import numpy as np

# ---------------------------------------------------------------------------
# 1.  Locate / run the binary so the CSVs are always fresh
# ---------------------------------------------------------------------------
SCRIPT_DIR = Path(__file__).parent
BUILD_DIR = SCRIPT_DIR.parents[1] / "build"
BINARY = BUILD_DIR / "Examples" / "SpreadCurve" / "SpreadCurve"

# CSVs are written to cwd of the binary — run from build dir
result = subprocess.run([str(BINARY)], cwd=str(BUILD_DIR), capture_output=True, text=True)
if result.returncode != 0:
    print("SpreadCurve binary failed:\n", result.stderr)
    sys.exit(1)

# ---------------------------------------------------------------------------
# 2.  Load data
# ---------------------------------------------------------------------------
def load_csv(path):
    with open(path) as f:
        reader = csv.DictReader(f)
        return list(reader)

curves   = load_csv(BUILD_DIR / "spread_curves.csv")
pillars  = load_csv(BUILD_DIR / "spread_pillars.csv")
fomc_raw = load_csv(BUILD_DIR / "fomc_dates.csv")

t_grid  = np.array([float(r["time"])             for r in curves])
base_z  = np.array([float(r["base_zero"])         for r in curves])
lin_z   = np.array([float(r["linear_zero"])       for r in curves])
cub_z   = np.array([float(r["cubic_zero"])        for r in curves])
lin_s   = np.array([float(r["linear_spread_bp"])  for r in curves])
cub_s   = np.array([float(r["cubic_spread_bp"])   for r in curves])

t_pil   = np.array([float(r["time"])              for r in pillars])
ls_pil  = np.array([float(r["linear_spread_bp"])  for r in pillars])
cs_pil  = np.array([float(r["cubic_spread_bp"])   for r in pillars])
bz_pil  = np.array([float(r["base_zero"])         for r in pillars])
lz_pil  = np.array([float(r["linear_zero"])       for r in pillars])

t_fomc  = np.array([float(r["time"]) for r in fomc_raw])

# ---------------------------------------------------------------------------
# 3.  Plot
# ---------------------------------------------------------------------------
fig, axes = plt.subplots(
    2, 1,
    figsize=(11, 8),
    gridspec_kw={"height_ratios": [3, 2]},
    sharex=True,
)

BLUE   = "#1f77b4"
RED    = "#d62728"
GRAY   = "#555555"
LIGHT  = "#aaaaaa"

# ── Top panel: zero rates ──────────────────────────────────────────────────
ax = axes[0]

ax.plot(t_grid, base_z, color=GRAY,  lw=1.8, ls="--", label="Base (Fed Funds futures)")
ax.plot(t_grid, lin_z,  color=BLUE,  lw=1.8,          label="Spread – Linear")
ax.plot(t_grid, cub_z,  color=RED,   lw=1.8, ls="-.", label="Spread – Cubic spline")

# Pillar markers on both spread curves
ax.scatter(t_pil, lz_pil, color=BLUE, s=40, zorder=5)
ax.scatter(t_pil, bz_pil, color=GRAY, s=30, marker="D", zorder=5,
           label="Spread pillar dates")

# FOMC verticals (only first 2Y to avoid clutter)
for t in t_fomc[t_fomc <= 2.05]:
    ax.axvline(t, color=LIGHT, lw=0.8, ls=":")

ax.set_ylabel("Continuous zero rate (%)", fontsize=11)
ax.yaxis.set_major_formatter(mticker.FormatStrFormatter("%.2f"))
ax.legend(fontsize=9, loc="upper right")
ax.set_title(
    "Fed Funds Futures Base Curve + Zero-Rate Spread Curves\n"
    "(PiecewiseSpreadYieldCurve<ZeroYield, Linear/Cubic>)",
    fontsize=12,
)
ax.grid(True, alpha=0.3)

# ── Bottom panel: spread in bp ─────────────────────────────────────────────
ax2 = axes[1]

ax2.plot(t_grid, lin_s, color=BLUE,  lw=1.8,          label="Linear spread")
ax2.plot(t_grid, cub_s, color=RED,   lw=1.8, ls="-.", label="Cubic spread")
ax2.fill_between(t_grid, lin_s, cub_s, alpha=0.12, color=RED,
                 label="|Linear − Cubic|")

# Pillar markers on spread values
ax2.scatter(t_pil, ls_pil, color=BLUE, s=40, zorder=5)
ax2.scatter(t_pil, cs_pil, color=RED,  s=30, marker="s", zorder=5)

for t in t_fomc[t_fomc <= 2.05]:
    ax2.axvline(t, color=LIGHT, lw=0.8, ls=":")
    ax2.text(t + 0.02, 5, "FOMC", fontsize=6, color=LIGHT,
             rotation=90, va="bottom")

ax2.set_xlabel("Time (years)", fontsize=11)
ax2.set_ylabel("Zero-rate spread (bp)", fontsize=11)
ax2.legend(fontsize=9, loc="lower left")
ax2.grid(True, alpha=0.3)
ax2.set_xlim(0, t_grid[-1] * 1.01)

plt.tight_layout()

out = SCRIPT_DIR / "spread_curves.png"
plt.savefig(out, dpi=150, bbox_inches="tight")
print(f"Saved: {out}")
plt.show()
