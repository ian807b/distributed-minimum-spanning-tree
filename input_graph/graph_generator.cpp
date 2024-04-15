#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <set>
#include <utility>

#include "../core/utils.h"

#define NUM_OF_VERTICES 20000
#define MAX_WEIGHT 1000

int main() {
  std::ofstream output_file("graph.txt");
  std::set<std::pair<uintV, uintV>> edge_set;

  srand(time(0));

  // Ensure all nodes are reachable (connected)
  for (uintV i = 1; i < NUM_OF_VERTICES; i++) {
    uintV node1 = i - 1;
    uintV node2 = i;
    uintE edge_weight = 1 + (rand() % MAX_WEIGHT);
    output_file << node1 << " " << node2 << " " << edge_weight << '\n';
    edge_set.insert({std::min(node1, node2), std::max(node1, node2)});
  }

  // Randomly add edges between nodes
  for (uintV node1 = 0; node1 < NUM_OF_VERTICES; node1++) {
    for (uintV node2 = node1 + 1; node2 < NUM_OF_VERTICES; node2++) {
      if (rand() % 2) {
        if (!edge_set.count({node1, node2})) {
          uintE edge_weight = 1 + (rand() % MAX_WEIGHT);
          output_file << node1 << " " << node2 << " " << edge_weight
                      << std::endl;
          edge_set.insert({node1, node2});
        }
      }
    }
  }

  output_file.close();

  // For validation of the implementations
  std::cout << "Total edges generated: " << edge_set.size() << std::endl;

  return 0;
}
