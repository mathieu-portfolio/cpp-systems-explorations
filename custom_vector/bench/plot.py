from __future__ import annotations

import argparse
import csv
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt


def load_rows(path: Path) -> list[dict]:
    with path.open(newline="", encoding="utf-8") as f:
        rows = list(csv.DictReader(f))

    for row in rows:
        row["size"] = int(row["size"])
        row["iterations"] = int(row["iterations"])
        row["best_ms"] = float(row["best_ms"])
        row["avg_ms"] = float(row["avg_ms"])
        row["throughput_ops_per_sec"] = float(row["throughput_ops_per_sec"])
        row["speedup_vs_std"] = float(row["speedup_vs_std"])

    return rows


def plot_growth(rows: list[dict], out_dir: Path) -> None:
    subset = [r for r in rows if r["scenario"] in {"growth", "reserve_growth"}]
    if not subset:
        return

    plt.figure(figsize=(8, 5))
    for scenario in ["growth", "reserve_growth"]:
        for mode in ["std_vector", "custom_vector"]:
            items = sorted(
                [r for r in subset if r["scenario"] == scenario and r["mode"] == mode and r["element_type"] == "int"],
                key=lambda r: r["size"],
            )
            if not items:
                continue
            plt.plot(
                [r["size"] for r in items],
                [r["best_ms"] for r in items],
                marker="o",
                label=f"{mode}:{scenario}",
            )

    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("Size")
    plt.ylabel("Best time (ms)")
    plt.title("Growth vs reserve growth")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "growth.png", dpi=150)
    plt.close()


def plot_copy_move(rows: list[dict], out_dir: Path) -> None:
    subset = [r for r in rows if r["scenario"] in {"copy", "move"}]
    if not subset:
        return

    plt.figure(figsize=(8, 5))
    for scenario in ["copy", "move"]:
        for mode in ["std_vector", "custom_vector"]:
            items = sorted(
                [r for r in subset if r["scenario"] == scenario and r["mode"] == mode and r["element_type"] == "int"],
                key=lambda r: r["size"],
            )
            if not items:
                continue
            plt.plot(
                [r["size"] for r in items],
                [r["best_ms"] for r in items],
                marker="o",
                label=f"{mode}:{scenario}",
            )

    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("Size")
    plt.ylabel("Best time (ms)")
    plt.title("Copy vs move")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "copy_move.png", dpi=150)
    plt.close()


def plot_types(rows: list[dict], out_dir: Path) -> None:
    subset = [r for r in rows if r["scenario"] == "reserve_growth"]
    if not subset:
        return

    plt.figure(figsize=(8, 5))
    for mode in ["std_vector", "custom_vector"]:
        for element_type in ["int", "string", "heavy"]:
            items = sorted(
                [r for r in subset if r["mode"] == mode and r["element_type"] == element_type],
                key=lambda r: r["size"],
            )
            if not items:
                continue
            plt.plot(
                [r["size"] for r in items],
                [r["best_ms"] for r in items],
                marker="o",
                label=f"{mode}:{element_type}",
            )

    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("Size")
    plt.ylabel("Best time (ms)")
    plt.title("Element type sensitivity")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "types.png", dpi=150)
    plt.close()


def write_summary(rows: list[dict], out_dir: Path) -> None:
    custom_rows = [r for r in rows if r["mode"] == "custom_vector"]
    if not custom_rows:
        return

    best = max(custom_rows, key=lambda r: r["speedup_vs_std"])
    weakest = min(custom_rows, key=lambda r: r["speedup_vs_std"])

    summary = f"""# Benchmark Summary

Best observed result relative to std::vector:
- scenario: {best["scenario"]}
- element_type: {best["element_type"]}
- size: {best["size"]}
- speedup_vs_std: {best["speedup_vs_std"]:.3f}x

Weakest observed result relative to std::vector:
- scenario: {weakest["scenario"]}
- element_type: {weakest["element_type"]}
- size: {weakest["size"]}
- speedup_vs_std: {weakest["speedup_vs_std"]:.3f}x
"""
    (out_dir / "summary.md").write_text(summary, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Plot vector benchmark results")
    parser.add_argument("--csv", required=True, help="Path to benchmark CSV")
    parser.add_argument("--out-dir", default="results", help="Directory for plots and summary")
    args = parser.parse_args()

    csv_path = Path(args.csv)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    rows = load_rows(csv_path)
    plot_growth(rows, out_dir)
    plot_copy_move(rows, out_dir)
    plot_types(rows, out_dir)
    write_summary(rows, out_dir)

    print(f"Wrote plots and summary to {out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
