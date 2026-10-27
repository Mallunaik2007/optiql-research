# Experiment 30: Spin-Counter Saturation Analysis

## Objective

Measure how frequently the Adaptive EWMA MCSRW lock reaches
the maximum recorded spin count of 256 iterations.

## Experimental Setup

- Threads: 4
- CPU affinity: 0-3
- Duration: 5 seconds per configuration
- Lock instances: 128, 5, 1
- Distribution: uniform
- Version reads: 80%
- Acquire/release: 20%
- Critical-section cycles: 50
- Parallel-section cycles: 0
- Environment: WSL Ubuntu

## Results

| Locks | Total waits | Waits at 256 | Cap rate |
|---|---:|---:|---:|
| 128 | 1,350,713 | 8,296 | 0.6142% |
| 5 | 12,738,751 | 23,366 | 0.1834% |
| 1 | 30,656,533 | 19,581 | 0.0639% |

## Findings

The spin-count cap was reached in fewer than 1% of recorded
waits in all three configurations.

The highest observed saturation rate occurred with 128 locks.

These results do not establish that spin-count saturation
explains the dominance of threshold 64.

## Limitations

- One benchmark run per configuration
- WSL scheduling variability
- Spin count saturates at 256
- Instrumentation may affect benchmark behavior
- Lock count is only a proxy for contention

## Conclusion

The spin-count cap is reached relatively infrequently in
these workloads. Further investigation is required before
modifying the adaptive threshold-selection policy.
