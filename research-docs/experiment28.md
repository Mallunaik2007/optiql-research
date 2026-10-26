# Experiment 28: Fixed-64 vs Adaptive EWMA

## Objective

Compare the throughput of the verified fixed-threshold MCSRW
implementation against the adaptive EWMA implementation.

## Experimental Environment

- Processor: Intel Core i5-13420H
- Environment: Ubuntu under WSL
- CPU affinity: Logical CPUs 0-3
- Threads: 4
- Lock instances: 5
- Workload: 80% version reads, 20% acquire-release
- Access distribution: Uniform
- Critical-section work: 50 cycles
- Parallel-section work: 0 cycles
- Duration: 5 seconds per run
- Repetitions: 5 per implementation

## Throughput Results

Throughput is measured in millions of operations per second.

| Run | Fixed-64 | Adaptive EWMA |
|-----|----------|---------------|
| 1 | 18.836 | 18.079 |
| 2 | 16.335 | 18.285 |
| 3 | 17.679 | 16.439 |
| 4 | 16.838 | 17.762 |
| 5 | 17.375 | 18.341 |

## Results Summary

- Fixed-64 mean: approximately 17.412 M operations/s
- Adaptive EWMA mean: approximately 17.781 M operations/s
- Relative difference: approximately +2.12% for EWMA
- EWMA had higher throughput in 3 of 5 runs.

## Interpretation

Adaptive EWMA achieved slightly higher mean throughput in
this specific experiment.

However, substantial run-to-run variation was observed.
Five runs are insufficient to establish a reliable general
performance advantage.

These measurements should be treated as preliminary.

## Build Configurations

Fixed-64:
- MCSRW_FIXED_MODE enabled
- MCSRW_SPIN_THRESHOLD=64
- Release build

Adaptive EWMA:
- MCSRW_FIXED_MODE disabled
- Release build

Both implementations were compiled from the same source
revision before Experiment 28.

## Limitations

- WSL scheduling and virtualization may influence throughput.
- Only one workload configuration was tested in this comparison.
- Additional independent runs and native Linux testing are needed.
- A higher mean alone does not establish statistical significance.

## Reproduction Instructions

Run these commands from the OptiQL repository root.

### 1. Build Fixed-64

    cmake -S latch-microbenchmarks -B latch-microbenchmarks/build-fixed64-verified -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-DMCSRW_FIXED_MODE -DMCSRW_SPIN_THRESHOLD=64"

    cmake --build latch-microbenchmarks/build-fixed64-verified --target mcsrw_offset_bench -j 4

### 2. Build Adaptive EWMA

    cmake -S latch-microbenchmarks -B latch-microbenchmarks/build-ewma-verified -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=""

    cmake --build latch-microbenchmarks/build-ewma-verified --target mcsrw_offset_bench -j 4

### 3. Run the Comparison

    FIXED=latch-microbenchmarks/build-fixed64-verified/benchmarks/mcsrw_offset_bench
    EWMA=latch-microbenchmarks/build-ewma-verified/benchmarks/mcsrw_offset_bench

    mkdir -p latch-microbenchmarks/results/experiment28-reproduction

    ARGS="--threads=4 --seconds=5 --array_size=5 --ver_read_pct=80 --acq_rel_pct=20 --dist=uniform --cs_cycles=50 --ps_cycles=0"

    for i in 1 2 3 4 5; do
        taskset -c 0-3 "$FIXED" $ARGS > "latch-microbenchmarks/results/experiment28-reproduction/fixed_$i.txt"
        taskset -c 0-3 "$EWMA" $ARGS > "latch-microbenchmarks/results/experiment28-reproduction/ewma_$i.txt"
    done

    grep -H '^All,' latch-microbenchmarks/results/experiment28-reproduction/*.txt

The first numeric field following `All,` represents aggregate
throughput in operations per second.

Results may vary depending on hardware, scheduling, and contention.

## Statistical Analysis

Five paired Fixed-64 and Adaptive EWMA runs were analyzed.

- Fixed-64 mean: 17.4125 Mops/s
- Fixed-64 sample standard deviation: 0.9476 Mops/s
- Adaptive EWMA mean: 17.7812 Mops/s
- Adaptive EWMA sample standard deviation: 0.7836 Mops/s
- Mean paired difference (EWMA minus Fixed-64): +0.3687 Mops/s
- Approximate 95% paired t confidence interval:
  [-1.2764, +2.0139] Mops/s

The confidence interval includes zero. Therefore, this experiment
does not provide sufficient evidence of a consistent throughput
advantage for Adaptive EWMA over Fixed-64.

The analysis assumes independent, approximately normally
distributed paired differences. WSL scheduling variation and
the small sample size limit the strength of this conclusion.

The observed 2.12% mean improvement is descriptive, not
a statistically established performance gain.
