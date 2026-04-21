from __future__ import annotations

import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt


def load_rows(path: Path) -> list[dict]:
    with path.open(newline="", encoding="utf-8") as f:
        rows = list(csv.DictReader(f))

    for row in rows:
        row["allocation_size"] = int(row["allocation_size"])
        row["alignment"] = int(row["alignment"])
        row["count"] = int(row["count"])
        row["iterations"] = int(row["iterations"])
        row["best_ms"] = float(row["best_ms"])
        row["avg_ms"] = float(row["avg_ms"])
        row["throughput_ops_per_sec"] = float(row["throughput_ops_per_sec"])
        row["speedup_vs_heap"] = float(row["speedup_vs_heap"])

    return rows


def plot_sizes(rows: list[dict], out_dir: Path) -> None:
    subset = [r for r in rows if r["scenario"] == "alloc_only" and r["alignment"] == 16]
    if not subset:
        return

    plt.figure(figsize=(8, 5))
    for mode in ["heap", "arena"]:
        items = sorted([r for r in subset if r["mode"] == mode], key=lambda r: r["allocation_size"])
        if not items:
            continue
        plt.plot(
            [r["allocation_size"] for r in items],
            [r["best_ms"] for r in items],
            marker="o",
            label=mode,
        )

    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("Allocation size (bytes)")
    plt.ylabel("Best time (ms)")
    plt.title("Allocation size sensitivity")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "sizes.png", dpi=150)
    plt.close()


def plot_alignments(rows: list[dict], out_dir: Path) -> None:
    subset = [r for r in rows if r["scenario"] == "alloc_only" and r["allocation_size"] == 64]
    if not subset:
        return

    plt.figure(figsize=(8, 5))
    for mode in ["heap", "arena"]:
        items = sorted([r for r in subset if r["mode"] == mode], key=lambda r: r["alignment"])
        if not items:
            continue
        plt.plot(
            [r["alignment"] for r in items],
            [r["best_ms"] for r in items],
            marker="o",
            label=mode,
        )

    plt.xscale("log", base=2)
    plt.yscale("log")
    plt.xlabel("Alignment")
    plt.ylabel("Best time (ms)")
    plt.title("Alignment sensitivity")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "alignments.png", dpi=150)
    plt.close()


def plot_patterns(rows: list[dict], out_dir: Path) -> None:
    subset = [r for r in rows if r["allocation_size"] == 64 and r["alignment"] == 16]
    subset = [r for r in subset if r["scenario"] in {"alloc_only", "alloc_reset", "mark_rewind"}]
    if not subset:
        return

    scenarios = ["alloc_only", "alloc_reset", "mark_rewind"]
    heap_vals = []
    arena_vals = []

    for scenario in scenarios:
        heap_row = next((r for r in subset if r["scenario"] == scenario and r["mode"] == "heap"), None)
        arena_row = next((r for r in subset if r["scenario"] == scenario and r["mode"] == "arena"), None)
        if heap_row and arena_row:
            heap_vals.append(heap_row["best_ms"])
            arena_vals.append(arena_row["best_ms"])

    if not heap_vals or not arena_vals:
        return

    x = list(range(len(heap_vals)))
    width = 0.35

    plt.figure(figsize=(8, 5))
    plt.bar([v - width / 2 for v in x], heap_vals, width=width, label="heap")
    plt.bar([v + width / 2 for v in x], arena_vals, width=width, label="arena")

    plt.xticks(x, scenarios)
    plt.ylabel("Best time (ms)")
    plt.title("Allocation pattern comparison")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "patterns.png", dpi=150)
    plt.close()


def write_summary(rows: list[dict], out_dir: Path) -> None:
    arena_rows = [r for r in rows if r["mode"] == "arena"]
    if not arena_rows:
      return

    best = max(arena_rows, key=lambda r: r["speedup_vs_heap"])
    weakest = min(arena_rows, key=lambda r: r["speedup_vs_heap"])

    summary = f"""# Benchmark Summary

Best observed result relative to heap allocation:
- scenario: {best["scenario"]}
- allocation_size: {best["allocation_size"]}
- alignment: {best["alignment"]}
- speedup_vs_heap: {best["speedup_vs_heap"]:.3f}x

Weakest observed result relative to heap allocation:
- scenario: {weakest["scenario"]}
- allocation_size: {weakest["allocation_size"]}
- alignment: {weakest["alignment"]}
- speedup_vs_heap: {weakest["speedup_vs_heap"]:.3f}x
"""
    (out_dir / "summary.md").write_text(summary, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Plot arena benchmark results")
    parser.add_argument("--csv", required=True, help="Path to benchmark CSV")
    parser.add_argument("--out-dir", default="results", help="Directory for plots and summary")
    args = parser.parse_args()

    csv_path = Path(args.csv)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    rows = load_rows(csv_path)
    plot_sizes(rows, out_dir)
    plot_alignments(rows, out_dir)
    plot_patterns(rows, out_dir)
    write_summary(rows, out_dir)

    print(f"Wrote plots and summary to {out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
