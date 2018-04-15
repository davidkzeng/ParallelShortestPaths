// Graph.h
#include <stdio.h>

#ifndef GRAPH_H
#define GRAPH_H

class Graph
{
  private:
    int nnode;
    int nedge;
    int *neighbor;
    int *neighbor_start;

  public:
    Graph(FILE *infile);
    int* get_neighbors(int node_id);
    int num_neighbors(int node_id);
};

#endif
