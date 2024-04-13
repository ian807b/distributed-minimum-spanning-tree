#include <stdlib.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <set>

#include "core/graph.h"
#include "core/utils.h"

void mst_serial(Graph &g) {
  UnionFind union_find(g.n_);
  std::vector<edge_t> mst_edges;
  timer t1;
  double time_taken = 0.0;

  t1.start();

  std::sort(
      g.edges.begin(), g.edges.end(),
      [](const edge_t &a, const edge_t &b) { return a.weight < b.weight; });

  for (auto edge : g.edges) {
    uintV root1 = union_find.find(edge.from);
    uintV root2 = union_find.find(edge.to);

    if (root1 != root2) {
      mst_edges.push_back(edge);
      union_find.merge(root1, root2);

      if (mst_edges.size() == g.n_ - 1) {
        break;
      }
    }
  }

  time_taken = t1.stop();

  // Print statistics
  std::cout << "Statistics" << std::endl;
  std::cout << "Total number of vertices in the graph: " << g.n_ << std::endl;
  std::set<uintV> vertices_in_mst;
  for (auto edge : mst_edges) {
    vertices_in_mst.insert(edge.from);
    vertices_in_mst.insert(edge.to);
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

int main() {
  Graph g;
  g.readGraphFromTextFile("input_graph/graph.txt");

  mst_serial(std::ref(g));

  return 0;
}
