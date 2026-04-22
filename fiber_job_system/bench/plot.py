from __future__ import annotations

import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt


def load_rows(path: Path) -> list[dict]:
    with path.open(newline="", encoding="utf-8") as f:
        rows = list(csv.DictReader(f))

    for row in rows:
        row["workers"] = int(row["workers"])
        row["fibers"] = int(row["fibers"])
        row["work_units"] = int(row["work_units"])
        row["yield_count"] = int(row["yield_count"])
        row["iterations"] = int(row["iterations"])
        row["best_ms"] = float(row["best_ms"])
        row["avg_ms"] = float(row["avg_ms"])
        row["throughput_fibers_per_sec"] = float(row["throughput_fibers_per_sec"])
        row["avg_ns_per_fiber"] = float(row["avg_ns_per_fiber"])

    return rows


def plot_workers(rows: list[dict], out_dir: Path) -> None:
    subset = [r for r in rows if r["work_units"] == 1000 and r["yield_count"] == 10]
    subset = [r for r in subset if r["mode"] in {"no_yield", "yield_once"}]
    if not subset:
        return

    plt.figure(figsize=(8, 5))
    for mode in ["no_yield", "yield_once"]:
        items = sorted([r for r in subset if r["mode"] == mode], key=lambda r: r["workers"])
        if not items:
            continue
        plt.plot(
            [r["workers"] for r in items],
            [r["throughput_fibers_per_sec"] for r in items],
            marker="o",
            label=mode,
        )

    plt.xlabel("Worker count")
    plt.ylabel("Throughput (fibers/sec)")
    plt.title("Worker scaling")
    plt.xticks(sorted({r["workers"] for r in subset}))
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "workers.png", dpi=150)
    plt.close()


def plot_granularity(rows: list[dict], out_dir: Path) -> None:
    subset = [r for r in rows if r["workers"] == 4 and r["mode"] in {"no_yield", "yield_once"}]
    if not subset:
        return

    plt.figure(figsize=(8, 5))
    for mode in ["no_yield", "yield_once"]:
        items = sorted([r for r in subset if r["mode"] == mode], key=lambda r: r["work_units"])
        if not items:
            continue
        plt.plot(
            [r["work_units"] for r in items],
            [r["avg_ns_per_fiber"] for r in items],
            marker="o",
            label=mode,
        )

    plt.xscale("log")
    plt.xlabel("Work units per phase")
    plt.ylabel("Average ns per fiber")
    plt.title("Granularity sensitivity")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "granularity.png", dpi=150)
    plt.close()


def plot_yield_cost(rows: list[dict], out_dir: Path) -> None:
    subset = [r for r in rows if r["mode"] == "yield_many" and r["workers"] == 4 and r["work_units"] == 1000]
    if not subset:
        return

    items = sorted(subset, key=lambda r: r["yield_count"])

    plt.figure(figsize=(8, 5))
    plt.plot(
        [r["yield_count"] for r in items],
        [r["avg_ns_per_fiber"] for r in items],
        marker="o",
    )
    plt.xlabel("Yields per fiber")
    plt.ylabel("Average ns per fiber")
    plt.title("Yield/resume cost")
    plt.tight_layout()
    plt.savefig(out_dir / "yield_cost.png", dpi=150)
    plt.close()


def write_summary(rows: list[dict], out_dir: Path) -> None:
    no_yield = [r for r in rows if r["mode"] == "no_yield"]
    yield_once = [r for r in rows if r["mode"] == "yield_once"]
    yield_many = [r for r in rows if r["mode"] == "yield_many"]

    lines = ["# Benchmark Summary", ""]

    if no_yield:
        best = min(no_yield, key=lambda r: r["best_ms"])
        lines.extend([
            "Best no-yield run:",
            f"- workers: {best['workers']}",
            f"- fibers: {best['fibers']}",
            f"- work_units: {best['work_units']}",
            f"- best_ms: {best['best_ms']:.3f}",
            "",
        ])

    if yield_once:
        best = min(yield_once, key=lambda r: r["best_ms"])
        lines.extend([
            "Best yield-once run:",
            f"- workers: {best['workers']}",
            f"- fibers: {best['fibers']}",
            f"- work_units: {best['work_units']}",
            f"- best_ms: {best['best_ms']:.3f}",
            "",
        ])

    if yield_many:
        lowest = min(yield_many, key=lambda r: r["avg_ns_per_fiber"])
        highest = max(yield_many, key=lambda r: r["avg_ns_per_fiber"])
        lines.extend([
            "Yield-many range:",
            f"- cheapest avg_ns_per_fiber: {lowest['avg_ns_per_fiber']:.1f} at yield_count={lowest['yield_count']}",
            f"- most expensive avg_ns_per_fiber: {highest['avg_ns_per_fiber']:.1f} at yield_count={highest['yield_count']}",
            "",
        ])

    (out_dir / "summary.md").write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Plot fiber_job_system benchmark results")
    parser.add_argument("--csv", required=True, help="Path to benchmark CSV")
    parser.add_argument("--out-dir", default="results", help="Directory for plots and summary")
    args = parser.parse_args()

    csv_path = Path(args.csv)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    rows = load_rows(csv_path)
    plot_workers(rows, out_dir)
    plot_granularity(rows, out_dir)
    plot_yield_cost(rows, out_dir)
    write_summary(rows, out_dir)

    print(f"Wrote plots and summary to {out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
