
#include <atomic>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <new>
#include <thread>
#include <vector>

#include "index-benchmarks/latches/MCSRW.h"

#ifdef OMCS_OFFSET
namespace mcsrw {
MCSRWQNode *base_qnode = nullptr;
}
#endif

int main() {
    constexpr int kThreads = 8;
    constexpr int kIterations = 10000;

    mcsrw::MCSRWLock lock;

#ifdef OMCS_OFFSET
    alignas(128) static mcsrw::MCSRWQNode
        nodes[mcsrw::MCSRWLock::kNumQueueNodes];
#endif

#ifdef OMCS_OFFSET
    mcsrw::base_qnode = nodes;
#endif

    std::atomic<int> active_readers{0};
    std::atomic<int> active_writers{0};
    std::atomic<int> violations{0};
    std::atomic<bool> start{false};

    int shared_counter = 0;
    std::vector<std::thread> workers;

    for (int t = 0; t < kThreads; ++t) {
        workers.emplace_back([&, t]() {
#ifdef OMCS_OFFSET
            auto *qnode = &nodes[t];
#else
            mcsrw::MCSRWQNode local_node;
            auto *qnode = &local_node;
#endif

            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            for (int i = 0; i < kIterations; ++i) {
                new (qnode) mcsrw::MCSRWQNode;

                if (t < 4) {
                    lock.read_lock(qnode);

                    active_readers.fetch_add(1);
                    if (active_writers.load() != 0) {
                        violations.fetch_add(1);
                    }

                    int value = shared_counter;
                    (void)value;

                    active_readers.fetch_sub(1);
                    lock.read_unlock(qnode);
                } else {
                    lock.lock(qnode);

                    active_writers.fetch_add(1);
                    if (active_writers.load() != 1 ||
                        active_readers.load() != 0) {
                        violations.fetch_add(1);
                    }

                    ++shared_counter;

                    active_writers.fetch_sub(1);
                    lock.unlock(qnode);
                }
            }
        });
    }

    start.store(true, std::memory_order_release);

    for (auto &worker : workers) {
        worker.join();
    }

    std::cout << "Expected counter: " << 4 * kIterations << "\n";
    std::cout << "Actual counter:   " << shared_counter << "\n";
    std::cout << "Violations:       " << violations.load() << "\n";

    if (shared_counter != 4 * kIterations ||
        violations.load() != 0) {
        std::cerr << "FAIL: Reader-writer correctness test\n";
        return 1;
    }

    std::cout << "PASS: Reader-writer correctness test\n";
    return 0;
}
