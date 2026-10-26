# Adaptive EWMA-Based Spinning for MCSRW Reader–Writer Locks

## 1. Project Overview

This project investigates whether an adaptive spinning policy
can improve the performance of MCSRW reader–writer locks.

The implementation extends the OptiQL MCSRW lock with an
Exponentially Weighted Moving Average (EWMA) mechanism.

The adaptive policy uses observed waiting-loop spin counts
to select a spinning threshold dynamically.

## 2. Research Problem

A fixed spinning policy may not respond effectively to changes
in lock contention and waiting behavior.

However, adaptive policies introduce additional computation
and may not always improve performance.

Research question:

Can EWMA-based adaptive spinning provide measurable performance
benefits over a fixed spinning threshold in MCSRW locks?

## 3. Proposed Approach

Each thread maintains its own adaptive spinning state.

The implementation:

1. Observes spin counts during lock waiting.
2. Updates a smoothed estimate using EWMA.
3. Selects a threshold from 0, 64, and 256.
4. Uses the selected threshold to control CPU pause behavior.

The EWMA calculation is:

    smoothed_wait = (3 * smoothed_wait + spin_count) / 4

The policy selects:

    smoothed_wait < 32    -> threshold 256
    smoothed_wait < 128   -> threshold 64
    otherwise             -> threshold 0

The implementation also supports a compile-time fixed-threshold
mode for controlled comparisons.

## 4. Preliminary Experimental Findings

Experiment 28 compared verified Fixed-64 and Adaptive EWMA
implementations using:

- 4 threads pinned to CPUs 0-3
- 5 lock instances
- 80% version-read operations
- 20% acquire-release operations
- Uniform access distribution
- 50-cycle critical sections
- 5-second benchmark runs
- 5 runs per implementation

Average throughput:

- Fixed-64: approximately 17.412 million operations/second
- Adaptive EWMA: approximately 17.781 million operations/second

Adaptive EWMA showed approximately 2.12% higher mean throughput
in this experiment.

However, the results varied across runs, and the experiment
does not establish a statistically significant or general
performance improvement.

## 5. Current Limitations

- Evaluation was performed under WSL.
- Only a limited number of verified comparison runs were collected.
- The adaptive policy frequently selected threshold 64.
- The spinning policy has not been proven optimal.
- Additional workloads and hardware environments are needed.

## 6. Repository

Original project:
https://github.com/sfu-dis/optiql

Research repository:
https://github.com/Mallunaik2007/optiql-research
