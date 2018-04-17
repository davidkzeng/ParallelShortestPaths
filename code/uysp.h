#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "graph.h"

class UYSP {

  int rho;
  Graph *g;
  int *hop_node_list;
  int *hop_node_flag;
  int *hop_graph;

  public:
    UYSP(Graph *g, int rho);
    int hop_adj(int i, int j);
    void doPrecomputation();
    int query(int a, int b);
};

