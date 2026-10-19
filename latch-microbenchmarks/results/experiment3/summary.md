# Experiment 3 — STDRWLock Validation (`stdrw_bench`)

## Configuration

Executable: `latch-microbenchmarks/build/benchmarks/stdrw_bench`

Each condition was run sequentially five times:

```text
--threads=12 --seconds=5 --array_size={5,128,1024} --ver_read_pct=80 \
--acq_rel_pct=20 --dist=uniform --cs_cycles=50 --ps_cycles=0
```

The aggregate operations/s is the first numeric field of the `All,...` record in each raw log. Standard deviation uses the sample formula (n - 1 denominator).

## Measurements

| Array size | Rep 1 aggregate ops/s | Rep 2 aggregate ops/s | Rep 3 aggregate ops/s | Rep 4 aggregate ops/s | Rep 5 aggregate ops/s | Mean aggregate ops/s | Sample SD | Minimum | Maximum |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 5 | 3,536,828.6 | 3,523,226.0 | 3,468,984.4 | 3,541,756.0 | 3,580,806.0 | 3,530,320.2 | 40,417.7 | 3,468,984.4 | 3,580,806.0 |
| 128 | 9,497,791.2 | 10,725,299.6 | 23,816,847.0 | 23,328,101.4 | 22,659,458.6 | 18,005,499.6 | 7,230,898.9 | 9,497,791.2 | 23,816,847.0 |
| 1024 | 85,054,265.6 | 58,558,417.2 | 47,405,316.4 | 52,010,414.0 | 64,024,626.8 | 61,410,608.0 | 14,650,106.4 | 47,405,316.4 | 85,054,265.6 |

Raw logs:

- `stdrw_array-5_rep-{1,2,3,4,5}.log`
- `stdrw_array-128_rep-{1,2,3,4,5}.log`
- `stdrw_array-1024_rep-{1,2,3,4,5}.log`

## Validation

All 15 runs recorded `exit_status=0` and exactly one aggregate `All` row. Each raw log's printed setup matches the configuration above. No runs failed.

## Comparison with Experiment 2

Experiment 2 used the same workload configuration, but three repetitions per array size. The values below compare mean aggregate operations/s; percentage change is `(Experiment 3 - Experiment 2) / Experiment 2`.

| Array size | Experiment 2 mean (n=3) | Experiment 2 sample SD | Experiment 3 mean (n=5) | Experiment 3 sample SD | Mean difference | Change |
|---:|---:|---:|---:|---:|---:|---:|
| 5 | 3,322,548.7 | 237,675.1 | 3,530,320.2 | 40,417.7 | +207,771.5 | +6.25% |
| 128 | 21,462,410.5 | 10,470,944.7 | 18,005,499.6 | 7,230,898.9 | -3,456,911.0 | -16.11% |
| 1024 | 100,089,876.7 | 1,930,099.2 | 61,410,608.0 | 14,650,106.4 | -38,679,268.7 | -38.64% |

The array-size-128 validation remains highly variable (9.50M–23.82M ops/s, 7.23M sample SD), although its sample SD is lower than Experiment 2's 10.47M. The array-size-1024 validation also shows substantial variation and a lower mean than Experiment 2.

Changing array size may affect memory-access patterns as well as lock contention. These results alone therefore do not prove that contention, memory behavior, or any other single factor caused the observed differences.
