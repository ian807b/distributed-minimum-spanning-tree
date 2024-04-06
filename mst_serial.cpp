#include <stdlib.h>

#include <algorithm>
#include <iomanip>
#include <iostream>

#include "core/graph.h"
#include "core/utils.h"

void mst_serial(Graph &g) {
  UnionFind union_find(g.n_);
  std::vector<edge_t> mst_edges;
  timer t1;
  double time_taken = 0.0;

  t1.start();

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
  std::cout << "Edges in the MST" << std::endl;
  std::cout << "From" << "To" << "Weight" << std::endl;
  for (auto edge : mst_edges) {
    std::cout << edge.from << ", " << edge.to << ", " << edge.weight
              << std::endl;
  }
  std::cout << "Time taken (in seconds) : " << time_taken << std::endl;
}

int main() {
  Graph g;
  g.readGraphFromTextFile("input_graph/graph.txt");
  std::sort(
      g.edges.begin(), g.edges.end(),
      [](const edge_t &a, const edge_t &b) { return a.weight < b.weight; });

  mst_serial(std::ref(g));

  return 0;
}
