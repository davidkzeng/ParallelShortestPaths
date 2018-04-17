// Graph.h
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "util.h"

#define MAXLINE 1024

#ifndef GRAPH_H
#define GRAPH_H

class Graph
{
  public:
    int nnode;
    int nedge;

    // Structures representing inedges.
    int *in_edge_count;
    int *in_neighbor;
    int *in_neighbor_start;

    int *neighbor;
    int *neighbor_start;
    Graph(FILE *infile);
    //Returns pointer to start of node_id's neighbor list
    int* get_neighbors(int node_id);
    //Returns number of neighbors of node_id
    int num_neighbors(int node_id);
    //Returns pointer to start of node_id's in neighbor list
    int* get_in_neighbors(int node_id);
    //Returns number of in neighbors of node_id
    int num_in_neighbors(int node_id);
    bool new_graph(int num_node, int num_edge);
    bool read_graph(FILE *infile);
    void show_graph();
    void show_in_graph();
    bool create_in_graph(FILE *infile);
};

#endif
