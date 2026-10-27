# Adaptive EWMA-Based Spinning for MCSRW Reader–Writer Locks

## Abstract

Reader–writer locks are synchronization mechanisms that allow
multiple readers to access shared data concurrently while
ensuring exclusive access for writers.

The performance of these locks can depend on how waiting threads
spin under different contention conditions.

This project investigates an adaptive spinning policy for the
MCSRW reader–writer lock in the OptiQL project.

The proposed implementation uses an Exponentially Weighted
Moving Average (EWMA) of observed spin counts to dynamically
select spinning thresholds of 0, 64, or 256.

We implemented the adaptive mechanism and compared its
performance with a fixed threshold of 64.

Both implementations passed the conducted correctness tests,
with 40,000 expected and actual counter updates and zero
reported violations.

Experimental results showed that adaptive EWMA did not
demonstrate a statistically reliable throughput advantage
over Fixed64 in the tested workloads.

Instrumentation also revealed that threshold 64 was selected
frequently, limiting the observed differentiation between
the adaptive and fixed policies.

The project contributes an adaptive implementation, measurement
instrumentation, and an empirical evaluation of its behavior.

## 1. Introduction

Modern multicore systems execute many threads concurrently.
These threads frequently access shared data structures,
requiring synchronization to maintain correctness.

Reader–writer locks support concurrent readers while
providing exclusive access to writers.

However, lock performance may degrade when multiple threads
compete for access to shared resources.

One important design decision is how waiting threads should
spin while attempting to acquire a lock.

A fixed spinning threshold is simple, but it cannot directly
adapt to changes in observed waiting behavior.

This research investigates whether an EWMA-based adaptive
spinning policy can improve the performance of MCSRW
reader–writer locks.

The goal is not to assume that adaptive spinning is faster,
but to experimentally determine whether it provides
measurable benefits under controlled workloads.

## 2. Research Problem

The performance of MCSRW reader–writer locks depends partly
on how waiting threads behave during lock contention.

A fixed spinning threshold applies the same policy regardless
of previously observed waiting behavior.

An adaptive policy may respond to changing conditions, but
its additional computation can introduce overhead.

The central research question is:

Can EWMA-based adaptive spinning provide a measurable and
statistically reliable throughput improvement over Fixed64
spinning in MCSRW reader–writer locks?

## 3. Research Objectives

The objectives of this project are:

1. Study the existing MCSRW reader–writer lock implementation
   in the OptiQL repository.

2. Implement a thread-local EWMA mechanism that tracks
   observed spin counts.

3. Dynamically select spinning thresholds based on the
   smoothed waiting behavior.

4. Implement a Fixed64 baseline for controlled comparison.

5. Verify basic reader–writer correctness under concurrent
   execution.

6. Measure throughput across different lock-count
   configurations.

7. Analyze adaptive threshold selection and spin-count
   saturation using instrumentation.

8. Evaluate experimental differences using paired
   statistical analysis.

9. Identify limitations and determine whether the adaptive
   policy demonstrates a reliable performance advantage.

## 4. Research Hypothesis

The working hypothesis is that adaptive EWMA spinning
may improve throughput by adjusting spinning behavior
according to previously observed waiting conditions.

The evaluation does not assume that this hypothesis is true.

Experimental measurements are used to determine whether
the adaptive implementation demonstrates an advantage
over the Fixed64 baseline.

## 5. Proposed Adaptive EWMA Algorithm

### 5.1 Overview

The proposed method extends the existing MCSRW reader-writer
lock with an adaptive spinning policy.

Instead of always using one fixed spinning threshold, the
implementation maintains an estimate of previously observed
waiting-loop spin counts.

This estimate is used to select a spinning threshold for
subsequent waiting operations.

### 5.2 Thread-Local Adaptive State

Each thread maintains its own adaptive state containing:

- smoothed_wait: EWMA estimate of observed spin counts.
- threshold: currently selected spinning threshold.

The initial values are:

    smoothed_wait = 64
    threshold = 64

Thread-local state avoids introducing a shared EWMA variable
that would require additional synchronization.

### 5.3 EWMA Calculation

After observing a waiting-loop spin count, the smoothed
estimate is updated using:

    smoothed_wait = (3 * smoothed_wait + spin_count) / 4

This corresponds to an EWMA with a smoothing factor of 0.25,
using integer arithmetic.

The previous estimate contributes 75 percent and the latest
observed spin count contributes 25 percent before integer
rounding.

### 5.4 Threshold Selection

The implementation selects one of three thresholds:

    smoothed_wait < 32   -> threshold = 256
    smoothed_wait < 128  -> threshold = 64
    otherwise            -> threshold = 0

The selected threshold controls when the waiting loop begins
issuing processor pause instructions.

The policy is based on observed spin iterations rather than
direct measurements of elapsed waiting time.

### 5.5 Spin-Count Limit

The recorded spin count is capped at 256.

This bounds the observation supplied to the EWMA mechanism.

### 5.6 Fixed64 Baseline

A compile-time configuration disables adaptive EWMA updates
and uses a fixed spinning threshold of 64.

This baseline enables controlled performance comparisons
between the adaptive and non-adaptive policies.

### 5.7 Implementation Scope

The adaptive mechanism is integrated into the MCSRW reader
and writer waiting paths.

The project also adds instrumentation to observe threshold
selection, spin-count distributions, and waiting behavior.

The underlying research objective is to evaluate adaptive
spinning, not to replace the MCSRW locking protocol.

## 6. Implementation and Correctness Verification

### 6.1 Implementation Environment

The research implementation was developed using:

- Programming language: C++
- Operating environment: Ubuntu under WSL
- Compiler: GCC
- Original repository: OptiQL
- Synchronization mechanism: MCSRW reader-writer lock
- Benchmark framework: OptiQL latch microbenchmarks

The primary implementation changes were made in:

    index-benchmarks/latches/MCSRW.h

Additional benchmark configuration and instrumentation
changes were introduced to support experimental evaluation.

### 6.2 Adaptive and Fixed64 Implementations

Two configurations were evaluated:

Adaptive EWMA:
Uses thread-local EWMA state to dynamically select
spinning thresholds of 0, 64, or 256.

Fixed64:
Uses a constant spinning threshold of 64 and disables
the adaptive EWMA update mechanism.

Both configurations retain the underlying MCSRW
reader-writer synchronization protocol.

### 6.3 Correctness Testing

A concurrent correctness test was executed for both
implementations.

The test checked the expected shared counter value
and reported reader-writer synchronization violations.

Results:

| Metric | Adaptive EWMA | Fixed64 |
|---|---:|---:|
| Expected counter | 40000 | 40000 |
| Actual counter | 40000 | 40000 |
| Reported violations | 0 | 0 |
| Result | PASS | PASS |

Both implementations passed the conducted test.

### 6.4 Interpretation

The results provide evidence that both configurations
maintained the tested synchronization invariants
during the executed workloads.

However, these tests do not constitute a formal proof
of correctness and cannot exclude every possible
concurrency defect or interleaving.

## 7. Experimental Methodology

### 7.1 Experimental Objective

The experiments evaluate whether Adaptive EWMA spinning
provides higher throughput than Fixed64 spinning in
MCSRW reader-writer locks.

The evaluation also investigates adaptive threshold
selection and spin-count saturation.

### 7.2 Hardware and Software Environment

Experiments were conducted using:

- Processor: Intel Core i5-13420H
- Operating system: Ubuntu under Windows Subsystem for Linux
- Programming language: C++
- Build configuration: Release
- Benchmark framework: OptiQL latch microbenchmarks

WSL scheduling and virtualized CPU topology may affect
the reproducibility of performance measurements.

### 7.3 Benchmark Configuration

The main comparison experiments used:

- Threads: 4
- Benchmark duration: 5 seconds per trial
- Workload distribution: Uniform
- Version-read operations: 80%
- Acquire-release operations: 20%
- Critical-section cycles: 50
- Parallel-section cycles: 0
- Lock counts: 128, 5, and 1
- Repetitions: 5 paired trials per configuration

Experiments 32-34 used CPU affinity 0,2,4,6.

Experiment 28 used CPU affinity 0-3 and 5 locks.

### 7.4 Performance Metric

Throughput was measured in millions of operations
per second (Mops/s).

The percentage difference was calculated as:

    Difference (%) =
        ((EWMA throughput - Fixed64 throughput)
         / Fixed64 throughput) * 100

A positive difference indicates higher observed
throughput for Adaptive EWMA.

A negative difference indicates higher observed
throughput for Fixed64.

### 7.5 Statistical Evaluation

Five paired trials were used for each main
performance comparison.

Mean throughput and paired differences were analyzed.

Approximate 95% confidence intervals were calculated
for the mean paired throughput difference.

A confidence interval containing zero does not
establish a statistically reliable performance
difference at the corresponding confidence level.

### 7.6 Instrumentation

Additional experiments collected information about:

- Adaptive threshold selections
- Waiting-loop spin-count distributions
- Frequency of reaching the spin-count cap
- Measured waiting behavior

Instrumentation results were analyzed separately
because measurement overhead may affect performance.

### 7.7 Experimental Limitations

The evaluation is limited by:

- WSL scheduling variability
- Small numbers of paired trials
- Limited workload configurations
- Virtualized CPU topology
- Potential instrumentation overhead

Results should not be generalized to all hardware,
contention patterns, or reader-writer workloads.

## 8. Experimental Results

### 8.1 Throughput Comparison

Adaptive EWMA and Fixed64 were evaluated using the
OptiQL latch microbenchmark framework.

The following table summarizes the main experiments.

| Experiment | Locks | Fixed64 (Mops/s) | EWMA (Mops/s) | Difference |
|---|---:|---:|---:|---:|
| 28 | 5 | 17.4125 | 17.7812 | +2.12% |
| 32 | 128 | 44.5609 | 43.2747 | -2.89% |
| 33 | 5 | 17.1926 | 16.7256 | -2.72% |
| 34 | 1 | 7.0200 | 6.8963 | -1.76% |

Experiment 28 used CPU affinity 0-3.

Experiments 32-34 used CPU affinity 0,2,4,6.

The results therefore represent different experimental
configurations and should not be interpreted as identical
repeated measurements.

### 8.2 Statistical Analysis

Approximate 95% confidence intervals for the mean paired
difference (EWMA minus Fixed64) were:

| Experiment | Paired 95% CI (Mops/s) |
|---|---|
| 28 | [-1.2764, +2.0139] |
| 32 | [-4.5004, +1.9280] |
| 33 | [-1.2160, +0.2820] |
| 34 | [-0.6820, +0.4347] |

All intervals contain zero.

Consequently, these experiments do not establish a
statistically reliable throughput improvement for
Adaptive EWMA over Fixed64.

### 8.3 Adaptive Threshold Behavior

Experiment 29 investigated the frequency of adaptive
threshold selection.

Threshold 64 dominated the observed selections:

- 128 locks: approximately 97.30%
- 5 locks: approximately 99.84%
- 1 lock: approximately 99.997%

These observations indicate that the adaptive policy
frequently selected the same threshold as the Fixed64
baseline under the tested workloads.

This may help explain the limited performance
differentiation, but does not establish causality.

### 8.4 Spin-Count Saturation

Experiment 30 measured how frequently waiting-loop
spin counts reached the cap of 256.

| Locks | Total waits | Waits at cap | Cap rate |
|---|---:|---:|---:|
| 128 | 1350713 | 8296 | 0.6142% |
| 5 | 12738751 | 23366 | 0.1834% |
| 1 | 30656533 | 19581 | 0.0639% |

The spin-count cap was reached in fewer than 1%
of recorded waits in all tested configurations.

These measurements do not establish that spin-count
saturation caused the dominance of threshold 64.

### 8.5 Overall Findings

Adaptive EWMA achieved a higher observed mean throughput
in Experiment 28.

Fixed64 achieved higher observed mean throughput in
Experiments 32, 33, and 34.

However, none of these comparisons demonstrated a
statistically reliable throughput advantage.

The adaptive implementation was functional, but the
available measurements do not demonstrate that it
consistently outperforms Fixed64.

## 9. Discussion and Limitations

### 9.1 Interpretation of Performance Results

The experimental evaluation did not demonstrate a
statistically reliable throughput advantage for
Adaptive EWMA over the Fixed64 baseline.

Although Adaptive EWMA achieved higher mean throughput
in Experiment 28, Fixed64 achieved higher mean
throughput in Experiments 32, 33, and 34.

All reported paired 95% confidence intervals
included zero.

Therefore, the observed differences cannot be
treated as conclusive evidence of a performance
advantage for either implementation.

### 9.2 Dominance of Threshold 64

Instrumentation showed that the adaptive policy
selected threshold 64 for most recorded observations.

This indicates that the implemented EWMA mechanism
often selected the same threshold as the fixed baseline.

One possible explanation for the limited throughput
difference is that the adaptive policy did not
frequently switch to alternative thresholds.

However, the experiments do not establish that
threshold dominance caused the observed performance.

### 9.3 Adaptive Policy Overhead

Adaptive EWMA requires additional operations to
maintain the smoothed waiting estimate and select
a threshold.

These computations may introduce overhead compared
with a fixed-threshold implementation.

The experiments did not isolate the cost of EWMA
updates sufficiently to quantify this overhead.

Therefore, we cannot conclude that adaptive
computation caused the observed throughput differences.

### 9.4 Spin-Count Saturation

Experiment 30 showed that the recorded spin count
reached its cap of 256 in fewer than 1% of waits
across the tested configurations.

This suggests that saturation was relatively
uncommon in these workloads.

However, the measurements do not prove that
saturation has no effect on adaptive behavior.

### 9.5 Experimental Limitations

The main limitations of this study are:

1. Experiments were conducted under WSL rather
   than a dedicated native Linux environment.

2. Only five paired trials were collected for
   each main throughput comparison.

3. The evaluation covered a limited set of
   lock counts and workload configurations.

4. Virtualized CPU topology and scheduling
   may affect measured throughput.

5. Instrumentation may introduce additional
   execution overhead.

6. Only Fixed64 was used as the primary
   fixed-threshold comparison baseline.

7. The adaptive mechanism uses spin iterations
   rather than direct elapsed-time measurements.

8. The correctness tests do not constitute
   a formal verification of the locking protocol.

### 9.6 Research Significance

The contribution of this study is not a proven
improvement in throughput.

Instead, the project provides:

- An implemented adaptive spinning mechanism
  for MCSRW reader-writer locks.

- A controlled Fixed64 comparison baseline.

- Correctness testing for both configurations.

- Instrumentation of adaptive threshold behavior.

- Experimental evidence showing that adaptation
  does not necessarily improve performance.

- Identification of limitations that can guide
  future research on adaptive lock waiting.

The negative and inconclusive performance findings
are useful because they distinguish an implemented
adaptive mechanism from a demonstrated optimization.

## 10. Future Work

The experimental findings identify several opportunities
for future research on adaptive spinning in MCSRW
reader-writer locks.

### 10.1 Time-Based Waiting Measurements

The current implementation uses waiting-loop spin counts
as input to the EWMA mechanism.

Future research could investigate elapsed-time-based
measurements using processor timestamp counters.

This may provide a different representation of waiting
behavior, but timestamp measurement overhead must be
evaluated carefully.

### 10.2 Improved Threshold Selection

The current adaptive policy selects thresholds of
0, 64, and 256.

Instrumentation showed that threshold 64 dominated
the observed selections.

Future research could evaluate alternative threshold
boundaries, additional threshold values, or different
adaptation strategies.

Such modifications would require new correctness
testing and controlled performance evaluation.

### 10.3 Evaluation on Native Linux

The experiments were conducted under WSL.

Future evaluations could use native Linux on dedicated
hardware to reduce virtualization-related variability.

Testing across different processor architectures
could also help assess generalizability.

### 10.4 Additional Workload Configurations

Future experiments could investigate:

- Larger thread counts
- Different reader-to-writer ratios
- Different critical-section durations
- Different lock-access distributions
- Additional contention patterns

These experiments could help determine whether
adaptive spinning is beneficial under conditions
not covered by the current study.

### 10.5 Adaptive Policy Overhead

Future work could isolate the computational cost
of EWMA updates and threshold selection.

This would help distinguish adaptation overhead
from other sources of throughput variation.

### 10.6 Stronger Statistical Evaluation

Additional paired trials and repeated experiments
could improve the precision of throughput estimates.

Future studies could also use dedicated benchmark
environments and more comprehensive statistical
analysis.

### 10.7 Summary

These directions represent possible extensions
of the research.

They are not part of the completed implementation
or experimental claims of the current project.

## 11. Conclusion

This research investigated the use of an adaptive
Exponentially Weighted Moving Average (EWMA) spinning
policy for MCSRW reader-writer locks.

The proposed implementation extends the OptiQL MCSRW
lock with thread-local adaptive state.

The algorithm observes waiting-loop spin counts,
updates an EWMA estimate, and selects a spinning
threshold from 0, 64, and 256.

A Fixed64 implementation was used as the baseline
for controlled performance comparisons.

Both Adaptive EWMA and Fixed64 passed the conducted
correctness tests, with 40,000 expected and actual
counter updates and zero reported violations.

Performance evaluation was conducted using the
OptiQL latch microbenchmark framework.

Experiment 28 showed a 2.12% higher observed mean
throughput for Adaptive EWMA.

However, Experiments 32, 33, and 34 showed lower
observed mean throughput for Adaptive EWMA,
with differences of -2.89%, -2.72%, and -1.76%.

All reported paired 95% confidence intervals
included zero.

Therefore, the experiments did not establish a
statistically reliable throughput advantage
for Adaptive EWMA over Fixed64.

Instrumentation revealed that threshold 64
dominated adaptive selections under the
tested workloads.

Spin-count saturation was relatively uncommon,
occurring in fewer than 1% of recorded waits
in Experiment 30.

These observations help characterize the
behavior and limitations of the adaptive policy.

The main contribution of this project is the
implementation and empirical evaluation of
EWMA-based adaptive spinning for MCSRW locks.

The findings demonstrate that introducing
adaptation does not automatically guarantee
higher synchronization performance.

Further evaluation under additional workloads
and hardware environments would be necessary
to establish whether the approach provides
benefits in other conditions.

Overall, the project successfully achieved
its implementation, correctness-testing,
instrumentation, and experimental-evaluation
objectives, while identifying important
limitations of the proposed adaptive policy.

## References

[1] OptiQL Research Repository.
    Simon Fraser University Database Systems Research.
    https://github.com/sfu-dis/optiql

[2] Adaptive EWMA-Based Spinning for MCSRW Reader-Writer
    Locks. Project implementation and experimental records.
    https://github.com/Mallunaik2007/optiql-research

[3] Mellor-Crummey, J. M., and Scott, M. L.
    Algorithms for Scalable Synchronization on
    Shared-Memory Multiprocessors.
    ACM Transactions on Computer Systems, 1991.

[4] Project benchmark documentation and experimental
    results stored in research-docs/ and
    latch-microbenchmarks/results/.

## Appendix A: Implementation Files

The primary implementation and benchmark files include:

- index-benchmarks/latches/MCSRW.h
- latch-microbenchmarks/benchmarks/bench_config.cpp
- latch-microbenchmarks/benchmarks/bench_config.hpp
- latch-microbenchmarks/benchmarks/mcsrw_bench.cpp
- research-tests/mcsrw_correctness.cpp

## Appendix B: Correctness Verification

The Adaptive EWMA and Fixed64 configurations were
tested using the concurrent correctness test.

Observed results for both configurations:

    Expected counter: 40000
    Actual counter:   40000
    Violations:       0
    Result:           PASS

Fixed64 was compiled with:

    -DMCSRW_LOCK_ONLY
    -DMCSRW_FIXED_MODE
    -DMCSRW_SPIN_THRESHOLD=64

These results demonstrate successful execution of
the tested correctness workload, not formal proof.

## Appendix C: Experimental Records

The detailed experiment documentation includes:

- research-docs/experiment28.md
- research-docs/experiment30.md
- research-docs/experiments32-34.md

Additional instrumentation and benchmark outputs
are stored in the research repository.

## Appendix D: Reproducibility Notes

The performance comparisons used separately compiled
Adaptive EWMA and Fixed64 benchmark executables.

Benchmark execution used taskset to control CPU affinity.

Throughput measurements may vary due to:

- Operating system scheduling
- Processor frequency changes
- Background processes
- WSL virtualization
- Lock contention and workload configuration

Reported results should be interpreted within
these experimental limitations.
