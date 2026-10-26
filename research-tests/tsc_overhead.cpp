#include <x86intrin.h>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    constexpr int N = 100000;
    std::vector<uint64_t> samples;
    samples.reserve(N);

    for (int i = 0; i < N; ++i) {
        unsigned int aux1, aux2;

        _mm_lfence();
        uint64_t start = __rdtscp(&aux1);
        _mm_lfence();

        uint64_t end = __rdtscp(&aux2);
        _mm_lfence();

        if (aux1 == aux2) {
            samples.push_back(end - start);
        }
    }

    std::sort(samples.begin(), samples.end());

    if (samples.empty()) {
        std::cerr << "No valid samples\n";
        return 1;
    }

    std::cout << "Valid samples: " << samples.size() << '\n';
    std::cout << "Minimum ticks: " << samples.front() << '\n';
    std::cout << "Median ticks: "
              << samples[samples.size() / 2] << '\n';
    std::cout << "95th percentile ticks: "
              << samples[(samples.size() * 95) / 100] << '\n';
    std::cout << "Maximum ticks: " << samples.back() << '\n';

    return 0;
}
