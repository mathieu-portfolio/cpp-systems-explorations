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
        row["jobs"] = int(row["jobs"])
        row["work_units"] = int(row["work_units"])
        row["iterations"] = int(row["iterations"])
        row["best_ms"] = float(row["best_ms"])
        row["avg_ms"] = float(row["avg_ms"])
        row["throughput_jobs_per_sec"] = float(row["throughput_jobs_per_sec"])
        row["speedup_vs_direct"] = float(row["speedup_vs_direct"])

    return rows


def plot_scaling(rows: list[dict], out_dir: Path) -> None:
    subset = [r for r in rows if r["scenario"] == "uniform" and r["work_units"] == 1000]
    subset = [r for r in subset if r["mode"] in {"thread_pool", "work_stealing_pool"}]
    if not subset:
        return

    plt.figure(figsize=(8, 5))
    for mode in ["thread_pool", "work_stealing_pool"]:
        items = sorted([r for r in subset if r["mode"] == mode], key=lambda r: r["workers"])
        if not items:
            continue
        plt.plot(
            [r["workers"] for r in items],
            [r["speedup_vs_direct"] for r in items],
            marker="o",
            label=mode,
        )

    plt.xlabel("Worker count")
    plt.ylabel("Speedup vs direct")
    plt.title("Scaling on uniform jobs")
    plt.xticks(sorted({r["workers"] for r in subset}))
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "scaling.png", dpi=150)
    plt.close()


def plot_granularity(rows: list[dict], out_dir: Path) -> None:
    subset = [r for r in rows if r["scenario"] == "uniform" and (r["workers"] == 4 or r["mode"] == "direct")]
    if not subset:
        return

    plt.figure(figsize=(8, 5))
    for mode in ["direct", "thread_pool", "work_stealing_pool"]:
        items = sorted([r for r in subset if r["mode"] == mode], key=lambda r: r["work_units"])
        if not items:
            continue
        plt.plot(
            [r["work_units"] for r in items],
            [r["throughput_jobs_per_sec"] for r in items],
            marker="o",
            label=mode,
        )

    plt.xscale("log")
    plt.xlabel("Work units per job")
    plt.ylabel("Throughput (jobs/sec)")
    plt.title("Granularity sensitivity on uniform jobs")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "granularity.png", dpi=150)
    plt.close()


def plot_imbalance(rows: list[dict], out_dir: Path) -> None:
    subset = [r for r in rows if r["workers"] == 4 and r["jobs"] == 10000 and r["work_units"] == 1000]
    subset = [r for r in subset if r["mode"] in {"thread_pool", "work_stealing_pool"}]
    if not subset:
        return

    scenarios = ["uniform", "skewed", "bursty"]

    plt.figure(figsize=(8, 5))
    for mode in ["thread_pool", "work_stealing_pool"]:
        xs = []
        ys = []
        for i, scenario in enumerate(scenarios):
            match = next((r for r in subset if r["mode"] == mode and r["scenario"] == scenario), None)
            if match:
                xs.append(i)
                ys.append(match["best_ms"])
        if xs:
            plt.plot(xs, ys, marker="o", label=mode)

    plt.xticks(range(len(scenarios)), scenarios)
    plt.ylabel("Best time (ms)")
    plt.title("Workload-shape cost")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "imbalance.png", dpi=150)
    plt.close()


def write_summary(rows: list[dict], out_dir: Path) -> None:
    wsp_rows = [r for r in rows if r["mode"] == "work_stealing_pool"]
    if not wsp_rows:
        return

    best = max(wsp_rows, key=lambda r: r["speedup_vs_direct"])
    weakest = min(wsp_rows, key=lambda r: r["speedup_vs_direct"])

    summary = f"""# Benchmark Summary

Best observed work-stealing-pool speedup vs direct execution:
- scenario: {best["scenario"]}
- workers: {best["workers"]}
- jobs: {best["jobs"]}
- work_units: {best["work_units"]}
- speedup_vs_direct: {best["speedup_vs_direct"]:.3f}x

Weakest observed work-stealing-pool result vs direct execution:
- scenario: {weakest["scenario"]}
- workers: {weakest["workers"]}
- jobs: {weakest["jobs"]}
- work_units: {weakest["work_units"]}
- speedup_vs_direct: {weakest["speedup_vs_direct"]:.3f}x
"""
    (out_dir / "summary.md").write_text(summary, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Plot work stealing pool benchmark results")
    parser.add_argument("--csv", required=True, help="Path to benchmark CSV")
    parser.add_argument("--out-dir", default="results", help="Directory for plots and summary")
    args = parser.parse_args()

    csv_path = Path(args.csv)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    rows = load_rows(csv_path)
    plot_scaling(rows, out_dir)
    plot_granularity(rows, out_dir)
    plot_imbalance(rows, out_dir)
    write_summary(rows, out_dir)

    print(f"Wrote plots and summary to {out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
