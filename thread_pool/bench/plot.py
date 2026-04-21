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
        row["workers"] = int(row["workers"])
        row["producers"] = int(row["producers"])
        row["jobs"] = int(row["jobs"])
        row["work_units"] = int(row["work_units"])
        row["iterations"] = int(row["iterations"])
        row["best_ms"] = float(row["best_ms"])
        row["avg_ms"] = float(row["avg_ms"])
        row["throughput_jobs_per_sec"] = float(row["throughput_jobs_per_sec"])
        row["speedup_vs_direct"] = float(row["speedup_vs_direct"])

    return rows


def plot_scaling(rows: list[dict], out_dir: Path) -> None:
    subset = [
        r for r in rows
        if r["scenario"].startswith("scaling_") and r["mode"] == "thread_pool"
    ]
    grouped: dict[int, list[dict]] = defaultdict(list)

    for row in subset:
        grouped[row["work_units"]].append(row)

    if not subset:
        return

    plt.figure(figsize=(8, 5))
    for work_units, items in sorted(grouped.items()):
        items = sorted(items, key=lambda r: r["workers"])
        plt.plot(
            [r["workers"] for r in items],
            [r["speedup_vs_direct"] for r in items],
            marker="o",
            label=f"work={work_units}",
        )

    plt.xlabel("Worker count")
    plt.ylabel("Speedup vs direct")
    plt.title("Thread pool scaling")
    plt.xticks(sorted({r["workers"] for r in subset}))
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "scaling.png", dpi=150)
    plt.close()


def plot_granularity(rows: list[dict], out_dir: Path) -> None:
    subset = [r for r in rows if r["scenario"].startswith("granularity_")]
    direct = sorted(
        [r for r in subset if r["mode"] == "direct"],
        key=lambda r: r["work_units"],
    )
    pool = sorted(
        [r for r in subset if r["mode"] == "thread_pool"],
        key=lambda r: r["work_units"],
    )

    if not subset:
        return

    plt.figure(figsize=(8, 5))
    plt.plot(
        [r["work_units"] for r in direct],
        [r["throughput_jobs_per_sec"] for r in direct],
        marker="o",
        label="direct",
    )
    plt.plot(
        [r["work_units"] for r in pool],
        [r["throughput_jobs_per_sec"] for r in pool],
        marker="o",
        label="thread_pool",
    )

    plt.xscale("log")
    plt.xlabel("Work units per job")
    plt.ylabel("Throughput (jobs/sec)")
    plt.title("Sensitivity to job granularity")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "granularity.png", dpi=150)
    plt.close()


def plot_contention(rows: list[dict], out_dir: Path) -> None:
    subset = [
        r for r in rows
        if r["scenario"].startswith("contention_") and r["mode"] == "thread_pool"
    ]
    subset = sorted(subset, key=lambda r: r["producers"])

    if not subset:
        return

    plt.figure(figsize=(8, 5))
    plt.plot(
        [r["producers"] for r in subset],
        [r["throughput_jobs_per_sec"] for r in subset],
        marker="o",
    )

    plt.xlabel("Producer count")
    plt.ylabel("Throughput (jobs/sec)")
    plt.title("Submission contention")
    plt.xticks([r["producers"] for r in subset])
    plt.tight_layout()
    plt.savefig(out_dir / "contention.png", dpi=150)
    plt.close()


def write_summary(rows: list[dict], out_dir: Path) -> None:
    pool_rows = [r for r in rows if r["mode"] == "thread_pool"]
    if not pool_rows:
        return

    best_speedup = max(pool_rows, key=lambda r: r["speedup_vs_direct"])
    weakest = min(pool_rows, key=lambda r: r["speedup_vs_direct"])

    summary = f"""# Benchmark Summary

Best observed speedup:
- scenario: {best_speedup["scenario"]}
- workers: {best_speedup["workers"]}
- producers: {best_speedup["producers"]}
- work_units: {best_speedup["work_units"]}
- speedup_vs_direct: {best_speedup["speedup_vs_direct"]:.3f}x

Weakest observed pool result:
- scenario: {weakest["scenario"]}
- workers: {weakest["workers"]}
- producers: {weakest["producers"]}
- work_units: {weakest["work_units"]}
- speedup_vs_direct: {weakest["speedup_vs_direct"]:.3f}x
"""
    (out_dir / "summary.md").write_text(summary, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Plot thread pool benchmark results")
    parser.add_argument("--csv", required=True, help="Path to benchmark CSV")
    parser.add_argument("--out-dir", default="results", help="Directory for plots and summary")
    args = parser.parse_args()

    csv_path = Path(args.csv)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    rows = load_rows(csv_path)
    plot_scaling(rows, out_dir)
    plot_granularity(rows, out_dir)
    plot_contention(rows, out_dir)
    write_summary(rows, out_dir)

    print(f"Wrote plots and summary to {out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
