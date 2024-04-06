#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time()
#include <fstream>
#include <iostream>

#include "../core/utils.h"

#define NUM_OF_VERTICES 100000
#define NUM_OF_EDGES 50000
#define MAX_WEIGHT 100

int main() {
  std::ofstream outputFile("graph.txt");

  srand(time(0));

  for (uintE i = 0; i < NUM_OF_EDGES; ++i) {
    uintV node1 = rand() % NUM_OF_VERTICES;
    uintV node2 = rand() % NUM_OF_VERTICES;
    uintE weight = rand() % MAX_WEIGHT;

    outputFile << node1 << " " << node2 << " " << weight << std::endl;
  }

  outputFile.close();

  return 0;
}
