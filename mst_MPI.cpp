#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "core/graph.h"
#include "core/utils.h"

#define DEFAULT_FILE_PATH "input_graph/graph.txt"

struct MSTData {
    unsigned long edges_processed;
    unsigned long weight_sum;
    double time_taken;
    MSTData() : edges_processed(0), weight_sum(0), time_taken(0.0){}
    MSTData(unsigned long _edges_processed, unsigned long _weight_sum, double _time_taken)
        : edges_processed(_edges_processed), weight_sum(_weight_sum), time_taken(_time_taken) {}
};


MSTData mst_parallel_worker(Graph &g, const std::vector<edge_t> &edges_subset, std::vector<edge_t> &mst_edges_local) {
    timer t;
    t.start();
    unsigned long edges_processed = 0;
    unsigned long weight_sum = 0;
    UnionFind union_find(g.n_);
    std::vector<edge_t> sorted_edges_subset = edges_subset;
    std::sort(sorted_edges_subset.begin(), sorted_edges_subset.end(), [](const edge_t &a, const edge_t &b) {
        return a.weight < b.weight;
    });
    
    for (const auto &edge : edges_subset) {
        uintV root1 = union_find.find(edge.from);
        uintV root2 = union_find.find(edge.to);
        if (root1 != root2) {
            edges_processed++;
            mst_edges_local.push_back(edge);
            union_find.merge(root1, root2);
            weight_sum += edge.weight;
        }
    }
    double time_taken = t.stop();

    return MSTData{edges_processed, weight_sum, time_taken};

}

void distribute_edges(const Graph &g, std::vector<edge_t> &local_edges, int rank, int size) {
    int total_edges = g.edges.size();
    int edges_per_proc = total_edges / size;
    int extra_edges = total_edges % size;
    int start_index = rank * edges_per_proc + std::min(rank, extra_edges);
    int end_index = start_index + edges_per_proc + (rank < extra_edges ? 1 : 0);

    local_edges.insert(local_edges.end(), g.edges.begin() + start_index, g.edges.begin() + end_index);
}

uintE calculate_local_mst_weight(std::vector<edge_t> &edges, uintV num_vertices, uint& count) {
    std::sort(edges.begin(), edges.end(), [](const edge_t &a, const edge_t &b) {
        return a.weight < b.weight;
    });
    UnionFind union_find(num_vertices);
    uintE weight_sum = 0;
    for (const auto& edge : edges) {
        uintV root1 = union_find.find(edge.from);
        uintV root2 = union_find.find(edge.to);
        
        if (root1 != root2) {
            union_find.merge(root1, root2);
            weight_sum += edge.weight;
            count++;
        }
    }
      
    return weight_sum;
}

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    cxxopts::Options options("MST_MPI",
                           "Minimum Spanning Tree Algorithm");
    options.add_options(
      "custom",
      {
          {"inputFile", "Input file path",
           cxxopts::value<std::string>()->default_value(DEFAULT_FILE_PATH)}
      });
    auto cl_options = options.parse(argc, argv);
    std::string input_file_path = cl_options["inputFile"].as<std::string>();


    Graph g;
    if (rank == 0) {
        g.readGraphFromTextFile(input_file_path);
    }
    timer main_time;
    main_time.start();
    MPI_Bcast(&g.n_, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);
    MPI_Bcast(&g.m_, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);
    std::vector<edge_t> local_edges(5);
    if (rank == 0) {
        for (int i = 1; i < size; i++) {
            std::vector<edge_t> proc_edges;
            distribute_edges(g, proc_edges, i, size);
            MPI_Send(proc_edges.data(), proc_edges.size() * sizeof(edge_t), MPI_BYTE, i, 0, MPI_COMM_WORLD);
        }
        distribute_edges(g, local_edges, rank, size);
    } else {
        int recv_size = (g.m_ / size) + (rank < g.m_ % size ? 1 : 0);
        local_edges.resize(recv_size);
        MPI_Recv(local_edges.data(), recv_size * sizeof(edge_t), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    std::vector<edge_t> mst_edges_local;
    MSTData localResult = mst_parallel_worker(g, local_edges, mst_edges_local);
    
    int local_size = mst_edges_local.size();
    std::vector<int> sizes;
    if (rank == 0) {
     sizes.resize(size);
    }
    MPI_Gather(&local_size, 1, MPI_INT, sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<edge_t> final;
    if (rank == 0) {
     for (int i = 0; i < size; ++i) {
      std::vector<edge_t> temp(sizes[i]);
      if (i == 0) {
       std::copy(mst_edges_local.begin(), mst_edges_local.end(), std::back_inserter(final));
      } else {
       MPI_Recv(temp.data(), sizes[i] * sizeof(edge_t), MPI_BYTE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
       std::copy(temp.begin(), temp.end(), std::back_inserter(final));
      }
     }
    }
    if (rank != 0) {
     MPI_Send(mst_edges_local.data(), local_size * sizeof(edge_t), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    }

    std::vector<MSTData> allData;
    std::vector<int> displs(size, 0);
    if (rank == 0) {
        allData.resize(size);
    }
    
    MPI_Gather(&localResult, sizeof(MSTData), MPI_BYTE, allData.data(), sizeof(MSTData), MPI_BYTE, 0,MPI_COMM_WORLD);
    if (rank == 0) {
        uint count = 1;
        uintE total_weight = calculate_local_mst_weight(final, g.n_, count);
        double total_time = main_time.stop();
        std::cout << "Statistics" << std::endl;
        std::cout << "Total number of vertices in the graph: " << g.n_ << std::endl;
        for (int i = 0; i < size; ++i) {
            std::cout << "Process " << i << ": "
                      << "Edges processed: " << allData[i].edges_processed << ", "
                      << "Total weight: " << allData[i].weight_sum << ", "
                      << "Time: " << allData[i].time_taken << std::endl;
        }
        std::cout << "Number of vertices in the MST: " << count<< std::endl;
        std::cout<<"Total weight of the MST: "<< total_weight <<std::endl;
        std::cout<<"Time taken (in seconds) : "<< total_time << std::endl;
    }

    MPI_Finalize();

    return 0;
}
