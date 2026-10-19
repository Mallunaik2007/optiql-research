# Experiment 4 — Controlled Repeatability (stdrw_bench)

## Configuration and order

Executable: latch-microbenchmarks/build/benchmarks/stdrw_bench

Every invocation used:

~~~text
--threads=12 --seconds=5 --array_size={5,128,1024} --ver_read_pct=80 \
--acq_rel_pct=20 --dist=uniform --cs_cycles=50 --ps_cycles=0
~~~

Each array size ran five times. Runs were sequential and the order was rotated by round:

| Round | Order |
|---:|:---|
| 1 | 5, 128, 1024 |
| 2 | 128, 1024, 5 |
| 3 | 1024, 5, 128 |
| 4 | 5, 128, 1024 |
| 5 | 128, 1024, 5 |

The per-run order is recorded in run_order.csv.

## Executable integrity

The SHA-256 before and after the experiment was identical:

~~~text
cc5ec0de16b11039d7c33b046ec52e5e16f12fbe318a37be6d4af36beec37aea
~~~

The executable modification time was unchanged: 2026-10-15 17:17:17.858460000 +0000. Its size was 557,600 bytes and inode was 2814749767446028 at both boundary captures. The pre- and post-run captures are in metadata_before.txt and metadata_after.txt.

## System and load snapshots

The host reported 12 logical CPUs on a WSL2 Linux guest (6.6.87.2-microsoft-standard-WSL2) running on an Intel Core i5-13420H. Boundary snapshots recorded the following available indicators:

| Indicator | Before (2026-10-19T15:42:56Z) | After (2026-10-19T15:45:25Z) |
|:---|:---|:---|
| /proc/loadavg (1/5/15 min) | 0.01 / 0.27 / 0.43 | 2.68 / 1.31 / 0.80 |
| procs_running | 1 | 1 |
| procs_blocked | 0 | 0 |
| Context-switch counter | 40,612,033 | 51,247,819 |

lscpu, uptime, /proc/loadavg, and /proc/stat were available and captured. mpstat and pidstat were unavailable, so no interval CPU-utilization, per-CPU utilization, per-process scheduling, frequency, affinity, or migration samples were collected. Boundary load averages cannot attribute load to the benchmark or establish conditions during individual runs.

## Validation and measurements

All 15 raw logs recorded exit_status=0, the requested printed setup, and exactly one All aggregate row. Aggregate operations/s is the first numeric field of each All row. Standard deviation is sample SD (n - 1); coefficient of variation (CV) is sample SD divided by mean.

| Array size | Rep 1 ops/s | Rep 2 ops/s | Rep 3 ops/s | Rep 4 ops/s | Rep 5 ops/s | Mean ops/s | Sample SD | Minimum | Maximum | CV |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 5 | 3,558,704.8 | 3,336,922.2 | 3,067,474.6 | 3,454,773.2 | 2,948,478.4 | 3,273,270.6 | 258,025.9 | 2,948,478.4 | 3,558,704.8 | 7.88% |
| 128 | 31,033,605.8 | 33,063,079.2 | 25,758,480.8 | 29,494,151.0 | 30,872,348.2 | 30,044,333.0 | 2,713,315.7 | 25,758,480.8 | 33,063,079.2 | 9.03% |
| 1024 | 82,384,367.0 | 95,518,640.2 | 99,000,459.4 | 80,328,156.4 | 70,409,841.4 | 85,528,292.9 | 11,691,873.0 | 70,409,841.4 | 99,000,459.4 | 13.67% |

Raw logs:

- stdrw_array-5_rep-{1,2,3,4,5}.log
- stdrw_array-128_rep-{1,2,3,4,5}.log
- stdrw_array-1024_rep-{1,2,3,4,5}.log

## Comparison with Experiments 2 and 3

| Array size | Experiment 2 mean (n=3) | Experiment 3 mean (n=5) | Experiment 4 mean (n=5) | E4 vs E2 | E4 vs E3 |
|---:|---:|---:|---:|---:|---:|
| 5 | 3,322,548.7 | 3,530,320.2 | 3,273,270.6 | -1.48% | -7.28% |
| 128 | 21,462,410.5 | 18,005,499.6 | 30,044,333.0 | +39.99% | +66.86% |
| 1024 | 100,089,876.7 | 61,410,608.0 | 85,528,292.9 | -14.55% | +39.27% |

Experiment 4 is less variable at array size 128 than Experiment 2 or 3 by both absolute sample SD (2.71M vs. 10.47M and 7.23M) and CV (9.03%). It does not remove between-experiment variation, especially for size 1024.

## Limitations

Sequential runs and a rotated order reduce a fixed first-run ordering effect, but cannot eliminate all environmental effects. The available snapshots do not characterize CPU load, scheduler placement, CPU frequency, or contention throughout each run. In addition, array size can change memory-access patterns as well as possible lock contention. These measurements do not prove any causal explanation for the observed throughput differences.
