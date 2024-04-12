#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

#include "../core/utils.h"

#define NUM_OF_VERTICES 10000000
#define NUM_OF_EDGES 100000000
#define MAX_WEIGHT 100000000

int main() {
  std::ofstream output_file("graph.txt");

  srand(time(0));

  // Ensures a "complete" graph
  for (uintV i = 1; i < NUM_OF_VERTICES; i++) {
    uintV node1 = i - 1;
    uintV node2 = i;
    uintE edge_weight = 1 + (rand() % MAX_WEIGHT);
    output_file << node1 << " " << node2 << " " << edge_weight << std::endl;
  }

  for (uintE i = 0; i < NUM_OF_EDGES; ++i) {
    uintV node1 = rand() % NUM_OF_VERTICES;
    uintV node2 = rand() % NUM_OF_VERTICES;

    // For optimization, removes a loop
    while (node1 == node2) {
      node2 = rand() % NUM_OF_VERTICES;
    }
    uintE weight = 1 + (rand() % MAX_WEIGHT);

    output_file << node1 << " " << node2 << " " << weight << std::endl;
  }

  output_file.close();

  std::cout << "edges = " << NUM_OF_EDGES << std::endl;
  return 0;
}
