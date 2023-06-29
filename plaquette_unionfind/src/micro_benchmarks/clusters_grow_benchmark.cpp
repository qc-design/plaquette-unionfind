#include <benchmark/benchmark.h>

#include "DecodingGraph.hpp"
#include "Clusters.hpp"
#include "Utils.hpp"


// sudo cpupower frequency-set --governor performance
// ./mybench
// sudo cpupower frequency-set --governor powersave

using namespace Plaquette;

// clang++-11 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include benchmark.cpp
// $GBENCH_DIR/lib/libbenchmark.a -pthread  -lrt -lm -o benchmark

void clusters_grow_benchmark_0(benchmark::State &state) {

  const unsigned int N = state.range(0);
  Plaquette::DecodingGraph graph(
      6, {{0, 1}, {1, 2}, {3, 4}, {4, 5}, {1, 4}, {3, 5}},
      {true, false, true, false, false, false});

  std::vector<bool> syndrome = {false, false, false, false, false, false};
  syndrome[N] = true;
  std::vector<bool> initial_cluster_edges = {false, false, false,
                                             false, false, false};
  std::vector<float> edge_increments = {1, 1, 1, 1, 1, 1};

  for (auto _ : state) {
    Clusters cluster_set(graph, syndrome, initial_cluster_edges, edge_increments);
    benchmark::DoNotOptimize(cluster_set.GrowCluster(N));
  }
  
  state.SetItemsProcessed(N * state.iterations());
}

BENCHMARK(clusters_grow_benchmark_0)->Arg(0);
BENCHMARK(clusters_grow_benchmark_0)->Arg(1);
BENCHMARK(clusters_grow_benchmark_0)->Arg(2);
BENCHMARK(clusters_grow_benchmark_0)->Arg(3);
BENCHMARK(clusters_grow_benchmark_0)->Arg(4);
BENCHMARK(clusters_grow_benchmark_0)->Arg(5);

BENCHMARK_MAIN();
