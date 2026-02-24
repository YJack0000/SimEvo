"""
River Ecosystem Experiment:
A main river flows across a large map with branching tributaries.
Food density is highest near the river and decreases with distance.

Hypothesis: Organisms near the river evolve larger bodies (abundant food),
while those far from the river stay small and fast (scarce food).

Outputs: frames saved to river_output/ and compiled into river_simulation.mp4
"""

import os
import math
import random

import imageio
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.colors import Normalize
from matplotlib.cm import ScalarMappable

from simevopy import Environment, Food, Genes, Organism
from utils.common import remove_all_foods, reproduce_organisms

OUTPUT_DIR = "river_output"
os.makedirs(OUTPUT_DIR, exist_ok=True)

# --- Config ---
MAP_W, MAP_H = 10000, 10000
NUM_GENERATIONS = 800
STEPS_PER_GEN = 100
FOOD_PER_GEN = 2000
INITIAL_ORGANISMS = 500
FRAME_EVERY = 5
VIDEO_FPS = 12


# --- River geometry ---
def build_river_segments():
    """
    Main river: flows from top-left to bottom-right with S-curve.
    Plus 4 tributaries branching off at different points.
    """
    segments = []

    # Main river - S-curve across the large map
    main_river = []
    for t in np.linspace(0, 1, 600):
        x = 1000 + 8000 * t + 800 * math.sin(t * math.pi * 2.5)
        y = MAP_H * t
        main_river.append((np.clip(x, 0, MAP_W - 1), np.clip(y, 0, MAP_H - 1)))
    segments.append(main_river)

    # Tributary 1: branches left from ~20%
    bp = main_river[120]
    trib = []
    for t in np.linspace(0, 1, 150):
        x = bp[0] - 3000 * t + 300 * math.sin(t * math.pi * 1.5)
        y = bp[1] + 3000 * t
        if 0 <= x < MAP_W and 0 <= y < MAP_H:
            trib.append((x, y))
    segments.append(trib)

    # Tributary 2: branches right from ~45%
    bp2 = main_river[270]
    trib2 = []
    for t in np.linspace(0, 1, 120):
        x = bp2[0] + 2500 * t + 200 * math.sin(t * math.pi)
        y = bp2[1] - 2800 * t
        if 0 <= x < MAP_W and 0 <= y < MAP_H:
            trib2.append((x, y))
    segments.append(trib2)

    # Tributary 3: branches left from ~70%
    bp3 = main_river[420]
    trib3 = []
    for t in np.linspace(0, 1, 100):
        x = bp3[0] - 2000 * t
        y = bp3[1] + 1500 * t + 200 * math.sin(t * math.pi * 2)
        if 0 <= x < MAP_W and 0 <= y < MAP_H:
            trib3.append((x, y))
    segments.append(trib3)

    # Sub-tributary from tributary 1 at ~40%
    if len(trib) > 60:
        bp4 = trib[60]
        trib4 = []
        for t in np.linspace(0, 1, 80):
            x = bp4[0] - 1500 * t
            y = bp4[1] + 1000 * t + 100 * math.sin(t * math.pi)
            if 0 <= x < MAP_W and 0 <= y < MAP_H:
                trib4.append((x, y))
        segments.append(trib4)

    return segments


RIVER_SEGMENTS = build_river_segments()
ALL_RIVER_POINTS = np.array([pt for seg in RIVER_SEGMENTS for pt in seg])


def distance_to_river(x, y):
    """Minimum distance from (x,y) to nearest river point."""
    dists = np.sqrt((ALL_RIVER_POINTS[:, 0] - x) ** 2 + (ALL_RIVER_POINTS[:, 1] - y) ** 2)
    return dists.min()


def distribute_river_food(env, total):
    """
    Food density decays extremely sharply with distance from river.
    - On the river (dist < 80): very high density
    - Near (80-300): moderate, dropping fast
    - Far (300-800): sparse
    - Beyond 800: almost nothing
    """
    placed = 0
    max_attempts = total * 50
    attempts = 0

    while placed < total and attempts < max_attempts:
        x = random.uniform(0, MAP_W - 1)
        y = random.uniform(0, MAP_H - 1)
        dist = distance_to_river(x, y)

        # Extremely sharp decay: most food within ~150 units of river
        decay = 80.0
        if dist > 800:
            p = 0.0
        elif dist > 400:
            p = 0.002 * math.exp(-(dist - 400) / 200)
        else:
            p = 0.02 + 0.98 * math.exp(-dist / decay)

        if random.random() < p:
            env.add_food(Food(), x, y)
            placed += 1
        attempts += 1


def setup_organisms(env, count):
    for _ in range(count):
        x = random.randint(0, MAP_W - 1)
        y = random.randint(0, MAP_H - 1)
        dna = chr(40) * 4
        env.add_organism(Organism(Genes(dna)), x, y)


def render_frame(env, gen, stats):
    """Render frame: map (left), time-series stats (top-right), scatter (bottom-right)."""
    fig = plt.figure(figsize=(20, 10))
    gs = fig.add_gridspec(2, 2, width_ratios=[2, 1], hspace=0.35, wspace=0.25)

    ax_map = fig.add_subplot(gs[:, 0])
    ax_ts = fig.add_subplot(gs[0, 1])
    ax_scatter = fig.add_subplot(gs[1, 1])

    # --- Draw river ---
    river_colors = ["#3377bb", "#5599dd", "#66aadd", "#77bbee", "#88ccee"]
    river_widths = [8, 5, 5, 4, 3]
    for idx, seg in enumerate(RIVER_SEGMENTS):
        seg_arr = np.array(seg)
        ax_map.plot(
            seg_arr[:, 0], seg_arr[:, 1],
            color=river_colors[idx % len(river_colors)],
            linewidth=river_widths[idx % len(river_widths)],
            alpha=0.5, zorder=0,
        )
        ax_map.plot(
            seg_arr[:, 0], seg_arr[:, 1],
            color=river_colors[idx % len(river_colors)],
            linewidth=river_widths[idx % len(river_widths)] * 6,
            alpha=0.06, zorder=0,
        )

    # --- Food ---
    foods = env.get_all_foods()
    food_pos = np.array([f.get_position() for f in foods if f.can_be_eaten()])
    if food_pos.size > 0:
        ax_map.scatter(food_pos[:, 0], food_pos[:, 1], s=1.5, color="green", alpha=0.35, zorder=1)

    # --- Organisms ---
    organisms = env.get_all_organisms()
    if len(organisms) == 0:
        plt.close(fig)
        return None

    org_pos = np.array([o.get_position() for o in organisms])
    org_sizes = np.array([o.get_size() for o in organisms])

    norm = Normalize(vmin=max(org_sizes.min() - 2, 1), vmax=org_sizes.max() + 2)
    cmap = plt.cm.coolwarm
    colors = cmap(norm(org_sizes))

    for i in range(len(org_pos)):
        circle = plt.Circle(
            (org_pos[i, 0], org_pos[i, 1]),
            max(org_sizes[i] * 1.5, 5),
            color=colors[i], alpha=0.6, zorder=2,
        )
        ax_map.add_patch(circle)

    sm = ScalarMappable(cmap=cmap, norm=norm)
    sm.set_array([])
    cbar = plt.colorbar(sm, ax=ax_map, fraction=0.025, pad=0.01)
    cbar.set_label("Body Size", fontsize=8)

    ax_map.set_xlim(0, MAP_W)
    ax_map.set_ylim(0, MAP_H)
    ax_map.set_aspect("equal")
    ax_map.set_facecolor("#f0ead6")
    ax_map.set_title(f"River Ecosystem — Gen {gen}  |  Organisms: {len(organisms)}", fontsize=13)
    ax_map.set_xlabel("X")
    ax_map.set_ylabel("Y")

    # --- Time-series stats (top-right) ---
    panels = [
        ("Population", stats["count"], "steelblue"),
        ("Avg Size", stats["avg_size"], "crimson"),
        ("Avg Speed", stats["avg_speed"], "darkorange"),
        ("Avg Awareness", stats["avg_awareness"], "purple"),
    ]
    ax_ts.clear()
    n = len(stats["count"])
    x = list(range(n))
    for idx, (label, data, color) in enumerate(panels):
        inset = ax_ts.inset_axes([0, 1 - (idx + 1) * 0.25, 1, 0.22])
        inset.plot(x, data, color=color, linewidth=1)
        inset.set_ylabel(label, fontsize=7)
        inset.tick_params(labelsize=6)
        if idx < len(panels) - 1:
            inset.set_xticklabels([])
        else:
            inset.set_xlabel("Generation", fontsize=7)
    ax_ts.axis("off")
    ax_ts.set_title("Global Stats", fontsize=10)

    # --- Per-zone comparison (bottom-right): Near / Mid / Far from river ---
    org_dists = np.array([distance_to_river(org_pos[i, 0], org_pos[i, 1]) for i in range(len(org_pos))])

    zone_labels = ["Riverside\n(<150)", "Near\n(150-400)", "Mid\n(400-800)", "Far\n(>800)"]
    zone_bounds = [(0, 150), (150, 400), (400, 800), (800, float("inf"))]
    zone_data = {l: {"sizes": [], "speeds": [], "awareness": []} for l in zone_labels}

    for i in range(len(org_pos)):
        d = org_dists[i]
        for j, (lo, hi) in enumerate(zone_bounds):
            if lo <= d < hi:
                zone_data[zone_labels[j]]["sizes"].append(org_sizes[i])
                zone_data[zone_labels[j]]["speeds"].append(organisms[i].get_speed())
                zone_data[zone_labels[j]]["awareness"].append(organisms[i].get_awareness())
                break

    bar_x = np.arange(len(zone_labels))
    bar_w = 0.25
    avg_sz = [np.mean(zone_data[l]["sizes"]) if zone_data[l]["sizes"] else 0 for l in zone_labels]
    avg_sp = [np.mean(zone_data[l]["speeds"]) if zone_data[l]["speeds"] else 0 for l in zone_labels]
    avg_aw = [np.mean(zone_data[l]["awareness"]) if zone_data[l]["awareness"] else 0 for l in zone_labels]
    counts = [len(zone_data[l]["sizes"]) for l in zone_labels]

    ax_scatter.bar(bar_x - bar_w, avg_sz, bar_w, label="Avg Size", color="indianred", alpha=0.7)
    ax_scatter.bar(bar_x, avg_sp, bar_w, label="Avg Speed", color="steelblue", alpha=0.7)
    ax_scatter.bar(bar_x + bar_w, avg_aw, bar_w, label="Avg Awareness", color="mediumpurple", alpha=0.7)

    for i, c in enumerate(counts):
        top_val = max(avg_sz[i], avg_sp[i], avg_aw[i]) if c > 0 else 0
        ax_scatter.text(i, top_val + 0.5, f"n={c}", ha="center", fontsize=7, color="gray")

    ax_scatter.set_xticks(bar_x)
    ax_scatter.set_xticklabels(zone_labels, fontsize=7)
    ax_scatter.legend(fontsize=7, loc="upper right")
    ax_scatter.set_title("Per-Zone Comparison by River Distance", fontsize=10)
    ax_scatter.set_ylabel("Value", fontsize=8)

    fig.tight_layout()
    path = os.path.join(OUTPUT_DIR, f"frame_{gen:04d}.png")
    fig.savefig(path, dpi=100)
    plt.close(fig)
    return path


# --- Main ---
env = Environment(MAP_W, MAP_H, type="optimized")
setup_organisms(env, INITIAL_ORGANISMS)

stats = {"count": [], "avg_size": [], "avg_speed": [], "avg_awareness": []}
frames = []

for gen in range(NUM_GENERATIONS):
    distribute_river_food(env, FOOD_PER_GEN)

    env.simulate_iteration(STEPS_PER_GEN)

    organisms = env.get_all_organisms()
    n = len(organisms)
    stats["count"].append(n)
    if n > 0:
        sizes = [o.get_size() for o in organisms]
        speeds = [o.get_speed() for o in organisms]
        awareness = [o.get_awareness() for o in organisms]
        stats["avg_size"].append(np.mean(sizes))
        stats["avg_speed"].append(np.mean(speeds))
        stats["avg_awareness"].append(np.mean(awareness))
    else:
        stats["avg_size"].append(0)
        stats["avg_speed"].append(0)
        stats["avg_awareness"].append(0)

    pct = (gen + 1) / NUM_GENERATIONS * 100
    print(f"[{pct:5.1f}%] Gen {gen:>4d}/{NUM_GENERATIONS} | organisms: {n:>4d} | avg_size: {stats['avg_size'][-1]:.1f} | avg_speed: {stats['avg_speed'][-1]:.1f}")

    if gen % FRAME_EVERY == 0:
        path = render_frame(env, gen, stats)
        if path:
            frames.append(path)

    reproduce_organisms(env)
    remove_all_foods(env)

    if n == 0:
        print("All organisms died. Stopping.")
        break

# Final frame
path = render_frame(env, gen, stats)
if path:
    frames.append(path)

# --- Create video ---
print(f"\nCreating video from {len(frames)} frames...")
video_path = os.path.join(OUTPUT_DIR, "river_simulation.mp4")
writer = imageio.get_writer(video_path, fps=VIDEO_FPS)
for f in frames:
    writer.append_data(imageio.imread(f))
writer.close()
print(f"Video saved to: {video_path}")
print("Done!")
