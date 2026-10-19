# Experiment 5 — Controlled STDRWLock vs. MCSRWLock Comparison

## Compatibility gate

The two existing executables use the same benchmark flags and common workload driver. In both, a version-read operation acquires a reader lock, performs the configured critical section, and releases the reader lock; an acquire-release operation uses exclusive lock/unlock around that same critical section.

MCSRW was run through mcsrw_offset_bench, compiled with MCSRW_LOCK_ONLY, OMCS_OFFSET, and OMCS_OFFSET_NUMA_QNODE. It allocates a queue-node pool before starting workers and uses one thread-local queue node per worker, reinitializing the node for each operation. Its configured pool has 1,024 nodes; 12 workers are within capacity. The source does not explicitly free that process-local allocation before exit, but each measurement was a separate normally exiting process, so it does not carry state or allocation into the next invocation.

The common aggregate row is:

~~~text
All,operations/s,successes/s,reads/s,read-successes/s
~~~

This report uses the first numeric field, aggregate operations/s, for throughput.

## Configuration and scheduling

Every run used:

~~~text
--threads=12 --seconds=5 --array_size={5,128,1024} --ver_read_pct=80 \
--acq_rel_pct=20 --dist=uniform --cs_cycles=50 --ps_cycles=0
~~~

There were five runs per lock/array-size condition, 30 runs total. Lock order alternated by repetition and array-size order rotated:

| Round | Lock order within each size | Array-size order |
|---:|:---|:---|
| 1 | STDRW, MCSRW | 5, 128, 1024 |
| 2 | MCSRW, STDRW | 128, 1024, 5 |
| 3 | STDRW, MCSRW | 1024, 5, 128 |
| 4 | MCSRW, STDRW | 5, 128, 1024 |
| 5 | STDRW, MCSRW | 128, 1024, 5 |

The exact sequence is in run_order.csv.

## Executable integrity and system snapshots

Both binaries had identical SHA-256, mtime, size, and inode values before and after the runs:

| Executable | SHA-256 | Modification time |
|:---|:---|:---|
| stdrw_bench | cc5ec0de16b11039d7c33b046ec52e5e16f12fbe318a37be6d4af36beec37aea | 2026-10-15 17:17:17.858460000 +0000 |
| mcsrw_offset_bench | 5c99319ec370bea248b6fe5eaa4d6427de61a7078c74c3cffcda5b6161fc3566 | 2026-10-15 17:17:18.072083200 +0000 |

The host reported 12 logical CPUs on WSL2. Boundary indicators were:

| Indicator | Before (2026-10-19T16:01:16Z) | After (2026-10-19T16:06:02Z) |
|:---|:---|:---|
| /proc/loadavg (1/5/15 min) | 0.12 / 0.10 / 0.30 | 5.08 / 3.11 / 1.51 |
| Context-switch counter | 51,812,630 | 62,702,281 |
| procs_running | 1 | 1 |
| procs_blocked | 0 | 0 |

lscpu, uptime, /proc/loadavg, and /proc/stat were available and captured in metadata_before.txt and metadata_after.txt. mpstat and pidstat were unavailable. No per-run CPU utilization, affinity, frequency, migration, or scheduler trace was captured; the boundary snapshots cannot attribute host load to either lock.

## Validation

All 30 raw logs recorded exit_status=0, the requested printed setup, and exactly one All row. No runs failed.

## Throughput results

Sample SD uses the n - 1 denominator. CV is sample SD divided by mean.

### STDRWLock

| Array size | Rep 1 ops/s | Rep 2 ops/s | Rep 3 ops/s | Rep 4 ops/s | Rep 5 ops/s | Mean | Sample SD | Minimum | Maximum | CV |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 5 | 3,507,851.8 | 3,189,179.6 | 2,960,047.4 | 3,453,308.2 | 3,413,896.6 | 3,304,856.7 | 227,711.0 | 2,960,047.4 | 3,507,851.8 | 6.89% |
| 128 | 32,221,783.0 | 32,178,351.2 | 30,344,763.8 | 33,005,635.0 | 32,401,265.8 | 32,030,359.8 | 998,562.9 | 30,344,763.8 | 33,005,635.0 | 3.12% |
| 1024 | 100,158,553.6 | 95,913,636.0 | 105,383,678.0 | 80,260,999.0 | 83,040,898.4 | 92,951,553.0 | 10,892,012.7 | 80,260,999.0 | 105,383,678.0 | 11.72% |

### MCSRWLock

| Array size | Rep 1 ops/s | Rep 2 ops/s | Rep 3 ops/s | Rep 4 ops/s | Rep 5 ops/s | Mean | Sample SD | Minimum | Maximum | CV |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 5 | 17,128,331.8 | 13,872,923.2 | 16,904,839.8 | 17,678,477.0 | 12,845,670.6 | 15,686,048.5 | 2,173,164.4 | 12,845,670.6 | 17,678,477.0 | 13.85% |
| 128 | 73,244,648.8 | 70,515,117.4 | 59,449,444.2 | 60,564,311.8 | 69,845,491.6 | 66,723,802.8 | 6,274,871.8 | 59,449,444.2 | 73,244,648.8 | 9.40% |
| 1024 | 87,656,138.6 | 66,546,734.4 | 77,731,808.0 | 85,988,532.4 | 96,590,631.0 | 82,902,768.9 | 11,332,836.5 | 66,546,734.4 | 96,590,631.0 | 13.67% |

## MCSRW relative to STDRW

| Array size | STDRW mean ops/s | MCSRW mean ops/s | Difference (MCSRW - STDRW) | Percentage difference |
|---:|---:|---:|---:|---:|
| 5 | 3,304,856.7 | 15,686,048.5 | +12,381,191.8 | +374.64% |
| 128 | 32,030,359.8 | 66,723,802.8 | +34,693,443.0 | +108.31% |
| 1024 | 92,951,553.0 | 82,902,768.9 | -10,048,784.1 | -10.81% |

## Limitations

This is a controlled comparison of these two existing implementations under one workload, host, and build. Alternating lock order and rotating array size reduce fixed-order bias, but sequential runs cannot remove changing host load, scheduling, CPU-frequency, or other environmental effects. MCSRW also has queue-node and NUMA-allocation mechanics that STDRW does not. Array size can change memory-access patterns as well as possible contention. The results do not establish causation or show that either implementation is universally better.
