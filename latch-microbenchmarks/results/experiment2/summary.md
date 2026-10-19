# Experiment 2 — Contention Analysis (`stdrw_bench`)

## Configuration

Executable: `latch-microbenchmarks/build/benchmarks/stdrw_bench`

Each array size was run sequentially three times with:

```text
--threads=12 --seconds=5 --ver_read_pct=80 --acq_rel_pct=20 \
--dist=uniform --cs_cycles=50 --ps_cycles=0
```

The varying argument was `--array_size={1,5,128,1024}`. Aggregate operations/s below is the first numeric value from each `All,...` record. Mean is across the three repetitions; standard deviation is the sample standard deviation (n - 1 denominator).

## Measurements

| Array size | Rep 1 aggregate ops/s | Rep 2 aggregate ops/s | Rep 3 aggregate ops/s | Mean aggregate ops/s | Sample SD |
|---:|---:|---:|---:|---:|---:|
| 1 | 2,053,619.6 | 2,072,318.0 | 1,991,869.2 | 2,039,268.9 | 42,100.6 |
| 5 | 3,500,095.0 | 3,415,014.2 | 3,052,537.0 | 3,322,548.7 | 237,675.1 |
| 128 | 26,366,040.8 | 9,439,467.4 | 28,581,723.4 | 21,462,410.5 | 10,470,944.7 |
| 1024 | 102,126,961.0 | 98,288,392.2 | 99,854,276.8 | 100,089,876.7 | 1,930,099.2 |

Raw outputs:

- `stdrw_array-1_rep-{1,2,3}.log`
- `stdrw_array-5_rep-{1,2,3}.log`
- `stdrw_array-128_rep-{1,2,3}.log`
- `stdrw_array-1024_rep-{1,2,3}.log`

## Validation and comparison

All 12 raw logs recorded `exit_status=0`, exactly 12 per-thread records, and exactly one `All` aggregate record. The captured stdout/stderr contains no CPU-affinity-related diagnostics (`affin`, `sched_setaffinity`, or `cpu`), so the runs do not establish whether threads were pinned.

In these measurements, the mean aggregate rate rises with array size: about 2.04M ops/s (1), 3.32M (5), 21.46M (128), and 100.09M (1024). The size-128 condition is notably variable: its 9.44M–28.58M ops/s range produces a 10.47M sample SD. The other conditions have lower relative variation, particularly array size 1.

## Limitations

- Only three 5-second repetitions were collected per condition; this is insufficient to attribute a causal performance change to contention alone.
- No CPU-affinity diagnostic was emitted, and CPU placement, frequency scaling, host load, NUMA effects, and scheduler activity were not controlled or measured here.
- The uniform access distribution and fixed 80/20 operation mix, critical section, and parallel section characterize only this workload; results should not be generalized to other mixes or platforms.
- No baseline or alternate implementation was run as part of this experiment, so this is a within-workload comparison across array sizes, not evidence of an implementation improvement.
