#include <numa.h>
#include <mutex>

#include "bench_config.hpp"
#include "mcsrw_bench.hpp"

#include <glog/logging.h>
#include <gtest/gtest.h>

#ifdef OMCS_OFFSET
MCSRWBench::QNode *mcsrw::base_qnode = nullptr;
#ifdef OMCS_OFFSET_NUMA_QNODE
#define PAGE_SIZE 4096
struct socket_queue_node_index {
  std::atomic<uint64_t> index;
  char padding[CACHELINE_SIZE - sizeof(index)];
};
socket_queue_node_index *socket_qnode_index;
#else
std::atomic<uint32_t> next_node(0);
#endif  // OMCS_OFFSET_NUMA_QNODE
#endif  // OMCS_OFFSET

MCSRWBench::QNode *MCSRWBench::GetNode() {
#ifdef OMCS_OFFSET
  thread_local QNode *qnode = nullptr;
  if (!qnode) {
#ifdef OMCS_OFFSET_NUMA_QNODE
    uint32_t socket = numa_node_of_cpu(sched_getcpu());
    uint32_t qnodes_per_page = PAGE_SIZE / sizeof(MCSRWBench::QNode);
    uint32_t index = socket_qnode_index[socket].index++;

    uint32_t nsockets = numa_max_node() + 1;
    uint32_t page_num = index / qnodes_per_page * nsockets + socket; 
    index = page_num * qnodes_per_page + index % qnodes_per_page;
    qnode = &mcsrw::base_qnode[index];
#else
    qnode = &mcsrw::base_qnode[next_node++];
#endif  // OMCS_OFFSET_NUMA_QNODE
  }
  return qnode;
#else
  LOG(FATAL) << "Not supported";
#endif
}

bool MCSRWBench::LatchVersionRead(size_t node, size_t idx) {
  Latch *latch = GetLatch(node, idx);
#ifdef OMCS_OFFSET
  QNode *qnode = GetNode();
  QNode &q = *qnode;
  new (qnode) QNode;
#else
  QNode q;
#endif
  latch->read_lock(&q);
  CriticalSection();
  latch->read_unlock(&q);
  return true;
}

std::mutex l;
void MCSRWBench::LatchAcquireRelease(size_t node, size_t idx) {
  Latch *latch = GetLatch(node, idx);
#ifdef OMCS_OFFSET
  QNode *qnode = GetNode();
  QNode &q = *qnode;
  new (qnode) QNode;
#else
  QNode q;
#endif
  latch->lock(&q);
  CriticalSection();
  latch->unlock(&q);
}

int main(int argc, char **argv) {
  ::google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

#ifdef OMCS_OFFSET
#ifdef OMCS_OFFSET_NUMA_QNODE
  static_assert(PAGE_SIZE % sizeof(MCSRWBench::QNode) == 0);

  // Round up to the proper number of pages
  uint32_t qnodes_per_page = PAGE_SIZE / sizeof(MCSRWBench::QNode);
  uint32_t npages = 0;
  uint32_t qnodes = 0;
  uint32_t sockets = numa_max_node() + 1;
  while (qnodes < mcsrw::MCSRWLock::kNumQueueNodes) {
    qnodes += qnodes_per_page * sockets;
    npages += sockets;
  }

  std::cout << "Allocated " << qnodes << " queue nodes over "
            << npages << " pages across " << sockets << " sockets" << std::endl;

  socket_qnode_index = (socket_queue_node_index *)malloc(sockets * sizeof(socket_queue_node_index));
  for (uint32_t i = 0; i < sockets; ++i) {
    socket_qnode_index[i].index = 0;
  }

  mcsrw::base_qnode = (MCSRWBench::QNode *)numa_alloc_interleaved(npages * PAGE_SIZE);
  if (!mcsrw::base_qnode) {
    abort();
  }
#else
  int ret = posix_memalign((void **)&mcsrw::base_qnode, 
                           CACHELINE_SIZE * 2,
                           sizeof(MCSRWBench::QNode) * mcsrw::MCSRWLock::kNumQueueNodes);
  if (ret) {
    abort();
  }

  for (uint32_t i = 0; i < mcsrw::MCSRWLock::kNumQueueNodes; ++i) {
    new (mcsrw::base_qnode + i) MCSRWBench::QNode;
  }
#endif  // OMCS_OFFSET_NUMA_QNODE
#endif  // OMCS_OFFSET

  MCSRWBench test;
  test.Run();

#ifdef MCSRW_EWMA_INSTRUMENT
  uint64_t threshold_0 = 0;
  uint64_t threshold_64 = 0;
  uint64_t threshold_256 = 0;
  uint64_t writer_updates = 0;
  uint64_t reader_updates = 0;
  uint64_t wait_0_31 = 0;
  uint64_t wait_32_63 = 0;
  uint64_t wait_64_127 = 0;
  uint64_t wait_128_256 = 0;
  // Aggregated TSC waiting-time statistics
  uint64_t tsc_wait_samples = 0;
  uint64_t tsc_wait_total = 0;
  uint64_t tsc_wait_min = UINT64_MAX;
  uint64_t tsc_wait_max = 0;

  for (const auto& counters : test.ewma_results) {
    threshold_0 += counters.threshold_0;
    threshold_64 += counters.threshold_64;
    threshold_256 += counters.threshold_256;
    writer_updates += counters.writer_updates;
    reader_updates += counters.reader_updates;
    wait_0_31 += counters.wait_0_31;
    wait_32_63 += counters.wait_32_63;
    wait_64_127 += counters.wait_64_127;
    wait_128_256 += counters.wait_128_256;
    // Aggregate TSC waiting-time statistics
    tsc_wait_samples += counters.tsc_wait_samples;
    tsc_wait_total += counters.tsc_wait_total;

    if (counters.tsc_wait_samples > 0) {
        if (counters.tsc_wait_min < tsc_wait_min) {
            tsc_wait_min = counters.tsc_wait_min;
        }

        if (counters.tsc_wait_max > tsc_wait_max) {
            tsc_wait_max = counters.tsc_wait_max;
        }
    }
  }

  const uint64_t total_selections =
      threshold_0 + threshold_64 + threshold_256;

  const uint64_t total_updates =
      writer_updates + reader_updates;

  std::cout << "\n=== EWMA Instrumentation Summary ===\n";
  std::cout << "Threshold 0 selections:   " << threshold_0 << '\n';
  std::cout << "Threshold 64 selections:  " << threshold_64 << '\n';
  std::cout << "Threshold 256 selections: " << threshold_256 << '\n';
  std::cout << "Reader updates:           " << reader_updates << '\n';
  std::cout << "Writer updates:           " << writer_updates << '\n';
  std::cout << "Total selections:         " << total_selections << '\n';
  std::cout << "Total updates:            " << total_updates << '\n';

  std::cout << "Counter consistency:      "
            << (total_selections == total_updates ? "PASS" : "FAIL")
            << '\n';
  const uint64_t total_waits =
      wait_0_31 + wait_32_63 + wait_64_127 + wait_128_256;

  std::cout << "\n=== Spin Wait Distribution ===\n";
  std::cout << "Wait 0-31:      " << wait_0_31 << '\n';
  std::cout << "Wait 32-63:     " << wait_32_63 << '\n';
  std::cout << "Wait 64-127:    " << wait_64_127 << '\n';
  std::cout << "Wait 128-256:   " << wait_128_256 << '\n';
  std::cout << "Total waits:    " << total_waits << '\n';

  std::cout << "Wait consistency: "
            << (total_waits == total_updates ? "PASS" : "FAIL")
            << '\n';          
  std::cout << "\n=== TSC Waiting-Time Statistics ===\n";
std::cout << "Valid TSC samples:   " << tsc_wait_samples << '\n';
std::cout << "Total waiting ticks: " << tsc_wait_total << '\n';

if (tsc_wait_samples > 0) {
    double average_ticks =
        static_cast<double>(tsc_wait_total) / tsc_wait_samples;

    std::cout << "Average wait (ticks): " << average_ticks << '\n';
    std::cout << "Minimum wait (ticks): " << tsc_wait_min << '\n';
    std::cout << "Maximum wait (ticks): " << tsc_wait_max << '\n';
} else {
    std::cout << "No valid TSC waiting-time samples.\n";
}

std::cout << "Rejected TSC samples: "
          << (total_waits - tsc_wait_samples) << '\n';

std::cout << "TSC sample consistency: "
          << (tsc_wait_samples <= total_waits ? "PASS" : "FAIL")
          << '\n';          
#endif

  return 0;
}

