#!/usr/bin/env bash
set -e

echo "============================================"
echo " Adaptive EWMA-Based Spinning for MCSRW"
echo " Operating Systems Research Project"
echo "============================================"

echo
echo "1. ADAPTIVE EWMA CORRECTNESS TEST"
./research-tests/mcsrw_correctness_adaptive

echo
echo "2. FIXED64 CORRECTNESS TEST"
./research-tests/mcsrw_correctness_fixed64_verified

echo
echo "3. ADAPTIVE EWMA ALGORITHM"
echo "EWMA = (3 * previous_EWMA + spin_count) / 4"
echo "EWMA < 32  : threshold = 256"
echo "EWMA < 128 : threshold = 64"
echo "Otherwise  : threshold = 0"

echo
echo "4. RECORDED THROUGHPUT RESULTS"
printf "%-12s %-12s %-12s %-12s\n" \
  "Experiment" "Fixed64" "EWMA" "Difference"
printf "%-12s %-12s %-12s %-12s\n" \
  "28" "17.4125" "17.7812" "+2.12%"
printf "%-12s %-12s %-12s %-12s\n" \
  "32" "44.5609" "43.2747" "-2.89%"
printf "%-12s %-12s %-12s %-12s\n" \
  "33" "17.1926" "16.7256" "-2.72%"
printf "%-12s %-12s %-12s %-12s\n" \
  "34" "7.0200" "6.8963" "-1.76%"

echo
echo "Throughput units: million operations/second"
echo "Results above are from previously completed experiments."

echo
echo "5. RESEARCH CONCLUSION"
echo "Both implementations passed the correctness tests."
echo "Adaptive EWMA did not demonstrate a statistically"
echo "reliable throughput advantage over Fixed64."
echo "All reported paired 95% confidence intervals included zero."

echo
echo "============================================"
echo " Demo completed"
echo "============================================"
