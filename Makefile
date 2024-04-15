ifdef USE_INT
MACRO = -DUSE_INT
endif

CXX = g++
MPICXX = mpic++
CXXFLAGS = -std=c++14 -O3 -pthread $(MACRO)

COMMON = core/utils.h core/cxxopts.h core/get_time.h core/graph.h core/quick_sort.h
G++ = mst_serial mst_parallel
MPI = mst_MPI
GRAPH_GEN = input_graph/graph_generator
ALL = $(G++) $(MPI) $(GRAPH_GEN)

all : $(ALL)

$(GRAPH_GEN): input_graph/graph_generator.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(G++) : % : %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(MPI) : % : %.cpp
	$(MPICXX) $(CXXFLAGS) -o $@ $<

.PHONY : clean

clean :
	rm -f *.o *.obj $(ALL)
