#include <stdlib.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#include "core/graph.h"
#include "core/utils.h"

#define DEFAULT_NUMBER_OF_THREADS "1"

std::mutex union_find_mutex;

struct ThreadResult {
  uintE edges_processed;
  uintV weight_sum;
  double time_taken;
  uintE num_edges;
  ThreadResult()
      : edges_processed(0), weight_sum(0), time_taken(0), num_edges(0) {}
};

void mst_parallel_worker(Graph &g, UnionFind &union_find,
                         const std::vector<edge_t> &edges_subset,
                         std::vector<edge_t> &mst_edges_local,
                         ThreadResult &result) {
  std::vector<edge_t> sorted_edges_subset = edges_subset;
  std::sort(
      sorted_edges_subset.begin(), sorted_edges_subset.end(),
      [](const edge_t &a, const edge_t &b) { return a.weight < b.weight; });

  timer t1;
  t1.start();
  for (auto &edge : sorted_edges_subset) {
    std::lock_guard<std::mutex> guard(union_find_mutex);
    uintV root1 = union_find.find(edge.from);
    uintV root2 = union_find.find(edge.to);
    result.num_edges++;
    if (root1 != root2) {
      mst_edges_local.push_back(edge);
      union_find.merge(root1, root2);
      result.edges_processed++;
      result.weight_sum += edge.weight;
    }
  }
  result.time_taken = t1.stop();
}

void merge_local_msts(std::vector<edge_t> &mst_edges,
                      const std::vector<std::vector<edge_t>> &mst_edges_local,
                      UnionFind &union_find) {
  std::vector<edge_t> all_edges;
  for (const auto &local_edges : mst_edges_local) {
    all_edges.insert(all_edges.end(), local_edges.begin(), local_edges.end());
  }

  std::sort(
      all_edges.begin(), all_edges.end(),
      [](const edge_t &a, const edge_t &b) { return a.weight < b.weight; });

  for (const auto &edge : all_edges) {
    uintV root1 = union_find.find(edge.from);
    uintV root2 = union_find.find(edge.to);

    if (root1 != root2) {
      mst_edges.push_back(edge);
      union_find.merge(root1, root2);
    }
  }
}

void mst_parallel(Graph &g, uint n_threads) {
  UnionFind union_find(g.n_);
  std::vector<edge_t> mst_edges;
  timer t1;
  double time_taken = 0.0;

  t1.start();

  std::vector<std::thread> threads;
  std::vector<std::vector<edge_t>> mst_edges_local(n_threads);
  std::vector<ThreadResult> results(n_threads);
  std::vector<UnionFind> union_finds(n_threads, UnionFind(g.n_));

  uintE total_edges = g.edges.size();
  uintE base_edges_per_thread = total_edges / n_threads;
  uintE remainder = total_edges % n_threads;

  uint start = 0;
  for (uint i = 0; i < n_threads; ++i) {
    uintE num_edges_assigned = base_edges_per_thread + (i < remainder ? 1 : 0);
    uint end = start + num_edges_assigned;

    threads.emplace_back(
        mst_parallel_worker, std::ref(g), std::ref(union_finds[i]),
        std::vector<edge_t>(g.edges.begin() + start, g.edges.begin() + end),
        std::ref(mst_edges_local[i]), std::ref(results[i]));

    start = end;
  }
  for (auto &thread : threads) {
    thread.join();
  }

  merge_local_msts(mst_edges, mst_edges_local, union_find);

  time_taken = t1.stop();

  std::cout << "Statistics" << std::endl;
  std::cout << "Total number of vertices in the graph: " << g.n_ << std::endl;
  std::set<uintV> vertices_in_mst;
  for (auto edge : mst_edges) {
    vertices_in_mst.insert(edge.from);
    vertices_in_mst.insert(edge.to);
  }

  for (uint i = 0; i < n_threads; ++i) {
    std::cout << "Thread " << i
              << ": Processed edges: " << results[i].edges_processed
              << ", Total weight: " << results[i].weight_sum
              << ", Time: " << results[i].time_taken << std::endl;
  }

  std::cout << "Number of vertices in the MST: " << vertices_in_mst.size()
            << std::endl;
  uintE total_weight = 0;
  for (auto edge : mst_edges) {
    total_weight += edge.weight;
  }

  std::cout << "Total weight of the MST: " << total_weight << std::endl;
  std::cout << "Time taken (in seconds) : " << time_taken << std::endl;
}

int main(int argc, char *argv[]) {
  Graph g;
  cxxopts::Options options("MST_Parallel", "Minimum Spanning Tree Algorithm");
  options.add_options(
      "custom",
      {{"nThreads", "Number of threads",
        cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_THREADS)}});
  auto cl_options = options.parse(argc, argv);
  uint n_threads = cl_options["nThreads"].as<uint>();
  g.readGraphFromTextFile("input_graph/graph.txt");
  if (n_threads > g.edges.size()) {
    std::cout << "nThreads must be less than number of edges!" << std::endl;
    return -1;
  }
  mst_parallel(std::ref(g), n_threads);

  return 0;
}
