# CMPT 431 Project

Serial, Parallel, and Distributed Implementations of the Minimum Spanning Tree Algorithm (Kruskal)

## Project Tree

CMPT431_PROJECT

```
.
├── Makefile
├── README.md
├── core
│   ├── cxxopts.h
│   ├── get_time.h
│   ├── graph.h
│   ├── quick_sort.h
│   └── utils.h
├── input_graph
│   └── graph_generator.cpp
├── mst_MPI.cpp
├── mst_parallel.cpp
└── mst_serial.cpp

```

## Graph Generator

This is a simple program to generate a graph in textfile. It uses edge list format:

[node1 node2 weight], the numbers are randomly generated. Running `make` folder from the project root directory will compile graph_generator as well.

## How to Run the Program

> [!TIP]
> Run `make` -> execute `graph_generator`, which creates an input graph -> run each algorithm

Generate a graph using graph_generator.cpp. You can adjust the number of vertices and the maximum weight of edge. Keep in mind that setting the maximum too high can cause an overflow.

> [!WARNING]
> Warning: Suggested minimum number of vertices is 10000, but it will take quite a lot of time to generate the text file.

Again, running `make` will compile the program and generate 3 executables, along with `graph_generator.cpp`.

Sample commands:

```
./mst_serial
./mst_parallel --nThreads 4
mpirun -n 4 ./mst_MPI
```
