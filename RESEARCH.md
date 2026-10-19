# OptiQL Lock Performance Evaluation

## Overview

This repository contains experimental work evaluating reader-writer synchronization mechanisms using the original OptiQL project as a starting point.

OptiQL is an optimistic locking mechanism designed for memory-optimized indexes. The original implementation and published research belong to their respective authors.

## Research Objective

Evaluate the throughput and scalability of two reader-writer lock implementations under selected workloads.

- STDRWLock: uses C++ std::shared_mutex.
- MCSRWLock: uses the project's queue-based reader-writer lock implementation.

## Experimental Environment

- Operating system: Ubuntu 24.04 under WSL2
- Logical CPUs reported: 12
- Benchmark duration: 5 seconds per run
- Read workload: 80%
- Acquire/release workload: 20%
- Distribution: uniform
- Critical-section cycles: 50
- Post-critical-section cycles: 0
- Array sizes: 5, 128 and 1024
- Repetitions in the fifth experiment: 5 per lock and array size

## Experiments

1. Thread scalability of STDRWLock.
2. Throughput across different array sizes.
3. Repeatability testing.
4. Run-order variation and repeatability testing.
5. Comparative throughput evaluation of STDRWLock and MCSRWLock.

Raw benchmark logs and summaries are stored in latch-microbenchmarks/results/.

## Preliminary Findings

In Experiment 5, MCSRWLock achieved higher mean throughput at array sizes 5 and 128. STDRWLock achieved higher mean throughput at array size 1024.

These are preliminary results from one WSL2 environment and one selected workload configuration. Host-load variation and run-to-run variability limit the strength of the conclusions. The measurements do not establish that either lock is universally superior.

## Limitations and Future Work

- Repeat experiments under more controlled CPU and host-load conditions.
- Investigate run-to-run variation.
- Evaluate additional thread counts and read/write ratios.
- Measure latency, writer waiting time and fairness alongside throughput.
- Repeat tests across additional workloads before drawing broader conclusions.

## Original Project and Citation

Ge Shi, Ziyi Yan and Tianzheng Wang. "OptiQL: Robust Optimistic Locking for Memory-Optimized Indexes." Proceedings of the ACM on Management of Data, Vol. 1, No. 3 (SIGMOD 2024).

Original repository: https://github.com/sfu-dis/optiql

Paper: https://www.cs.sfu.ca/~tzwang/optiql.pdf

The measurements in this repository are separate from the results reported in the original paper.
