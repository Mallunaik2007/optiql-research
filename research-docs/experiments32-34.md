# Experiments 32-34: Adaptive EWMA vs Fixed64

## Objective

Evaluate whether Adaptive EWMA spinning improves MCSRW
throughput compared with a fixed spin threshold of 64.

## Experimental Setup

- Threads: 4
- CPU affinity: 0,2,4,6
- Duration: 5 seconds per trial
- Repetitions: 5 paired trials per configuration
- Lock instances: 128, 5, 1
- Distribution: uniform
- Version reads: 80%
- Acquire/release: 20%
- Critical-section cycles: 50
- Parallel-section cycles: 0
- Build type: Release
- Environment: WSL Ubuntu

## Results

| Experiment | Locks | Fixed64 mean | EWMA mean | EWMA difference |
|---|---:|---:|---:|---:|
| 32 | 128 | 44.5609 | 43.2747 | -2.89% |
| 33 | 5 | 17.1926 | 16.7256 | -2.72% |
| 34 | 1 | 7.0200 | 6.8963 | -1.76% |

Throughput units: million operations per second (Mops/s).

## Statistical Interpretation

Experiment 32 paired 95% CI:
-4.5004 to +1.9280 Mops/s.

Experiment 33 paired 95% CI:
approximately -1.216 to +0.282 Mops/s.

Experiment 34 paired 95% CI:
-0.6820 to +0.4347 Mops/s.

All confidence intervals include zero.

## Findings

Fixed64 achieved higher mean throughput in all three
tested lock-count configurations.

However, the observed differences were not statistically
conclusive with five paired trials.

Experiment 29 also showed that the adaptive algorithm
frequently selects threshold 64, suggesting that the
adaptive mechanism may behave similarly to Fixed64
for these workloads.

## Limitations

- Small number of paired trials
- WSL scheduling and CPU frequency variability
- Limited workload configurations
- CPU topology is virtualized
- No demonstrated statistically significant improvement
- Other fixed thresholds may perform differently

## Conclusion

The adaptive EWMA implementation is functional, but
these experiments do not demonstrate a reliable
throughput advantage over Fixed64.

Further investigation should focus on adaptive threshold
selection, overhead, and additional workloads.
