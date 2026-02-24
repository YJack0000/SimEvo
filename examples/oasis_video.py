"""
Oasis Experiment: Three oases with different sizes and food densities
on a large desert map.

- Oasis A (Large, Sparse):   big area but low food density
- Oasis B (Medium, Normal):  medium area with moderate food density
- Oasis C (Small, Dense):    tiny area packed with food

The vast desert between oases has almost no food.

Hypothesis: Different food densities select for different traits.
Dense oasis → larger, slower organisms. Sparse oasis → faster, wider awareness.

Outputs: frames saved to oasis_output/ and compiled into oasis_simulation.mp4
"""

import os
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

OUTPUT_DIR = "oasis_output"
os.makedirs(OUTPUT_DIR, exist_ok=True)

# --- Config ---
MAP_W, MAP_H = 5000, 5000
NUM_GENERATIONS = 600
STEPS_PER_GEN = 100
FOOD_PER_GEN = 350
INITIAL_ORGANISMS = 120
FRAME_EVERY = 5
VIDEO_FPS = 12

# Three oases: (name, center_x, center_y, radius, food_ratio, sigma, color_label)
# Large Sparse: big area, moderate total food but spread thin
# Medium Normal: balanced
# Small Dense: tiny area, lots of food packed tight
OASES = [
    {"name": "Large Sparse",  "cx": 1200, "cy": 2800, "radius": 700, "food_ratio": 0.20, "sigma": 400, "color": "#a8d08d"},
    {"name": "Medium Normal", "cx": 3800, "cy": 1200, "radius": 450, "food_ratio": 0.25, "sigma": 200, "color": "#6bb85c"},
    {"name": "Small Dense",   "cx": 2500, "cy": 4000, "radius": 250, "food_ratio": 0.35, "sigma": 100, "color": "#2d8c1f"},
]
DESERT_FOOD_RATIO = 0.20  # more desert food so organisms can migrate between oases


# --- Food distribution ---
def distribute_oasis_food(env, total):
    for oasis in OASES:
        count = int(total * oasis["food_ratio"])
        for _ in range(count):
            x = np.clip(random.gauss(oasis["cx"], oasis["sigma"]), 0, MAP_W - 1)
            y = np.clip(random.gauss(oasis["cy"], oasis["sigma"]), 0, MAP_H - 1)
            env.add_food(Food(), x, y)

    # Sparse desert background
    bg_count = int(total * DESERT_FOOD_RATIO)
    for _ in range(bg_count):
        x = random.randint(0, MAP_W - 1)
        y = random.randint(0, MAP_H - 1)
        env.add_food(Food(), x, y)


def setup_organisms(env, count):
    for _ in range(count):
        x = random.randint(0, MAP_W - 1)
        y = random.randint(0, MAP_H - 1)
        dna = chr(40) * 4
        env.add_organism(Organism(Genes(dna)), x, y)


def classify_organism(x, y):
    """Classify organism by which oasis it's closest to, or 'desert'."""
    for i, oasis in enumerate(OASES):
        dist = ((x - oasis["cx"]) ** 2 + (y - oasis["cy"]) ** 2) ** 0.5
        if dist <= oasis["radius"] * 1.5:
            return i
    return -1  # desert


def render_frame(env, gen, stats):
    """Render frame with map + per-oasis stats."""
    fig = plt.figure(figsize=(20, 10))
    gs = fig.add_gridspec(2, 2, width_ratios=[2, 1], hspace=0.3, wspace=0.25)

    ax_map = fig.add_subplot(gs[:, 0])
    ax_stats_top = fig.add_subplot(gs[0, 1])
    ax_stats_bot = fig.add_subplot(gs[1, 1])

    # --- Map view ---
    foods = env.get_all_foods()
    food_pos = np.array([f.get_position() for f in foods if f.can_be_eaten()])

    organisms = env.get_all_organisms()
    if len(organisms) == 0:
        plt.close(fig)
        return None

    org_pos = np.array([o.get_position() for o in organisms])
    org_sizes = np.array([o.get_size() for o in organisms])

    # Draw oasis zones
    for oasis in OASES:
        circle_bg = plt.Circle(
            (oasis["cx"], oasis["cy"]), oasis["radius"],
            color=oasis["color"], alpha=0.12, zorder=0,
        )
        circle_border = plt.Circle(
            (oasis["cx"], oasis["cy"]), oasis["radius"],
            color=oasis["color"], alpha=0.3, fill=False, linewidth=1.5, linestyle="--", zorder=0,
        )
        ax_map.add_patch(circle_bg)
        ax_map.add_patch(circle_border)
        ax_map.text(
            oasis["cx"], oasis["cy"] - oasis["radius"] - 80,
            oasis["name"], ha="center", fontsize=9, color=oasis["color"],
            fontweight="bold", alpha=0.8,
        )

    # Food
    if food_pos.size > 0:
        ax_map.scatter(food_pos[:, 0], food_pos[:, 1], s=1.5, color="green", alpha=0.35, zorder=1)

    # Organisms colored by size
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
    ax_map.set_facecolor("#f5e6c8")  # sandy desert color
    ax_map.set_title(f"Oasis Experiment — Gen {gen}  |  Total Organisms: {len(organisms)}", fontsize=13)
    ax_map.set_xlabel("X")
    ax_map.set_ylabel("Y")

    # --- Global stats (top right) ---
    panels = [
        ("Population", stats["count"], "steelblue"),
        ("Avg Size", stats["avg_size"], "crimson"),
        ("Avg Speed", stats["avg_speed"], "darkorange"),
        ("Avg Awareness", stats["avg_awareness"], "purple"),
    ]
    ax_stats_top.clear()
    n = len(stats["count"])
    x = list(range(n))
    for idx, (label, data, color) in enumerate(panels):
        inset = ax_stats_top.inset_axes([0, 1 - (idx + 1) * 0.25, 1, 0.22])
        inset.plot(x, data, color=color, linewidth=1)
        inset.set_ylabel(label, fontsize=7)
        inset.tick_params(labelsize=6)
        if idx < len(panels) - 1:
            inset.set_xticklabels([])
        else:
            inset.set_xlabel("Generation", fontsize=7)
    ax_stats_top.axis("off")
    ax_stats_top.set_title("Global Stats", fontsize=10)

    # --- Per-oasis comparison (bottom right) ---
    oasis_labels = [o["name"] for o in OASES] + ["Desert"]
    zone_sizes = {label: [] for label in oasis_labels}
    zone_speeds = {label: [] for label in oasis_labels}
    zone_awareness = {label: [] for label in oasis_labels}
    zone_counts = {label: 0 for label in oasis_labels}

    for i in range(len(org_pos)):
        zone_idx = classify_organism(org_pos[i, 0], org_pos[i, 1])
        label = oasis_labels[zone_idx] if zone_idx >= 0 else "Desert"
        zone_sizes[label].append(org_sizes[i])
        zone_speeds[label].append(organisms[i].get_speed())
        zone_awareness[label].append(organisms[i].get_awareness())
        zone_counts[label] += 1

    ax_stats_bot.clear()
    bar_x = np.arange(len(oasis_labels))
    bar_w = 0.25
    avg_sz = [np.mean(zone_sizes[l]) if zone_sizes[l] else 0 for l in oasis_labels]
    avg_sp = [np.mean(zone_speeds[l]) if zone_speeds[l] else 0 for l in oasis_labels]
    avg_aw = [np.mean(zone_awareness[l]) if zone_awareness[l] else 0 for l in oasis_labels]
    counts = [zone_counts[l] for l in oasis_labels]

    ax_stats_bot.bar(bar_x - bar_w, avg_sz, bar_w, label="Avg Size", color="indianred", alpha=0.7)
    ax_stats_bot.bar(bar_x, avg_sp, bar_w, label="Avg Speed", color="steelblue", alpha=0.7)
    ax_stats_bot.bar(bar_x + bar_w, avg_aw, bar_w, label="Avg Awareness", color="mediumpurple", alpha=0.7)

    # Annotate counts on top
    for i, c in enumerate(counts):
        top_val = max(avg_sz[i], avg_sp[i], avg_aw[i])
        ax_stats_bot.text(i, top_val + 0.5, f"n={c}", ha="center", fontsize=7, color="gray")

    ax_stats_bot.set_xticks(bar_x)
    ax_stats_bot.set_xticklabels(oasis_labels, fontsize=8)
    ax_stats_bot.legend(fontsize=7, loc="upper right")
    ax_stats_bot.set_title("Per-Zone Comparison (current gen)", fontsize=10)
    ax_stats_bot.set_ylabel("Value", fontsize=8)

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
    distribute_oasis_food(env, FOOD_PER_GEN)

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
video_path = os.path.join(OUTPUT_DIR, "oasis_simulation.mp4")
writer = imageio.get_writer(video_path, fps=VIDEO_FPS)
for f in frames:
    writer.append_data(imageio.imread(f))
writer.close()
print(f"Video saved to: {video_path}")
print("Done!")
