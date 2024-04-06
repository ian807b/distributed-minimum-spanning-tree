#include <fstream>
#include <vector>

#include "utils.h"

typedef struct UndirectWeightedEdge {
  uintV from;
  uintV to;
  uintE weight;

  UndirectWeightedEdge(uintV from_, uintV to_, uintE weight_)
      : from(from_), to(to_), weight(weight_) {}
} edge_t;

class Graph {
 public:
  uintV n_;
  uintE m_;
  std::vector<edge_t> edges;

  void readGraphFromTextFile(std::string input_file_path) {
    std::ifstream input_stream(input_file_path);
    if (!input_stream) {
      std::cout << "Input File: " << input_file_path
                << " does not exist. Terminating" << std::endl;
      exit(2);
    }

    uintV from, to;
    uintV max_vertex_id = 0;
    uintE weight;
    while (input_stream >> from >> to >> weight) {
      edges.emplace_back(from, to, weight);
      if (from > max_vertex_id) max_vertex_id = from;
      if (to > max_vertex_id) max_vertex_id = to;
    }

    input_stream.close();

    n_ = max_vertex_id + 1;
    m_ = edges.size();
  }
};
