#include <stdlib.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <set>
#include <thread>
#include <vector>
#include <mutex>

#include "core/graph.h"
#include "core/utils.h"

std::mutex union_find_mutex;

struct ThreadResult{
    size_t edges_processed;
    uintV weight_sum;
    double time_taken;
    uintV num_vertices;
    ThreadResult() : edges_processed(0), weight_sum(0), time_taken(0), num_vertices(0) {}
};

void mst_parallel_worker(Graph &g, UnionFind &union_find, const std::vector<edge_t> &edges_subset, std::vector<edge_t> &mst_edges_local, ThreadResult &result) {
    timer t1;
    double time_taken = 0.0;
    t1.start();
    for (auto &edge : edges_subset) {
        std::lock_guard<std::mutex> guard(union_find_mutex);
        uintV root1 = union_find.find(edge.from);
        uintV root2 = union_find.find(edge.to);
        result.num_vertices++;
        if (root1 != root2) {
            mst_edges_local.push_back(edge);
            union_find.merge(root1, root2);
            result.edges_processed++;
            result.weight_sum += edge.weight;
        }
    }
    result.time_taken = t1.stop();
}

void merge_local_msts(std::vector<edge_t> &mst_edges, const std::vector<std::vector<edge_t>> &mst_edges_local, UnionFind &union_find) {
    
    std::vector<edge_t> all_edges;
    for (const auto &local_edges : mst_edges_local) {
        all_edges.insert(all_edges.end(), local_edges.begin(), local_edges.end());
    }

    std::sort(all_edges.begin(), all_edges.end(), [](const edge_t &a, const edge_t &b) {
        return a.weight < b.weight;
    });

    for (const auto &edge : all_edges) {
        uintV root1 = union_find.find(edge.from);
        uintV root2 = union_find.find(edge.to);

        if (root1 != root2) { 
            mst_edges.push_back(edge); 
            union_find.merge(root1, root2); 
        }
    }
}

void mst_parallel(Graph &g) {
    UnionFind union_find(g.n_);
    std::vector<edge_t> mst_edges;
    timer t1;
    double time_taken = 0.0;

    t1.start();

    size_t num_threads = 4;
    std::vector<std::thread> threads;
    std::vector<std::vector<edge_t>> mst_edges_local(num_threads);
    std::vector<ThreadResult> results(num_threads);
    std::vector<UnionFind> union_finds;
    for(size_t i =0 ; i<num_threads;++i){
        union_finds.emplace_back(g.n_);
    }
    
    size_t edges_per_thread = g.edges.size() / num_threads;
    for (size_t i = 0; i < num_threads; ++i) {
        size_t start = i * edges_per_thread;
        size_t end = (i + 1 == num_threads) ? g.edges.size() : start + edges_per_thread;
        
        threads.emplace_back(mst_parallel_worker, std::ref(g), std::ref(union_finds[i]), std::vector<edge_t>(g.edges.begin() + start, g.edges.begin() + end), std::ref(mst_edges_local[i]),std::ref(results[i]));
    }

    for (auto &thread : threads) {
        thread.join();
    }

    merge_local_msts(mst_edges,mst_edges_local,union_find);

    time_taken = t1.stop();

    std::cout << "Statistics" << std::endl;
    std::cout << "Total number of vertices in the graph: " << g.n_ << std::endl;
    std::set<uintV> vertices_in_mst;
    for (auto edge : mst_edges) {
        vertices_in_mst.insert(edge.from);
        vertices_in_mst.insert(edge.to);
    }

    for(size_t i = 0; i<num_threads; ++i){
        std::cout<<"Thread " << i << " Process vertices: " << results[i].num_vertices <<": Processed edges: " << results[i].edges_processed << ", Total weight: "<<results[i].weight_sum <<std::endl;
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
    std::sort(
    g.edges.begin(), g.edges.end(),
        [](const edge_t &a, const edge_t &b) { return a.weight < b.weight; });

    mst_parallel(std::ref(g));

    return 0;
}
