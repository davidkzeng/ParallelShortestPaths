#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <algorithm>


#include "graph.h"

class UYSP {

  int rho;
  Graph *g;
  int *hop_node_list;
  int *hop_node_flag;
  int *hop_graph;
  int bfs_limit;

  public:
    UYSP(Graph *g, int rho);
    int hop_adj(int i, int j);
    int BFSStoreHopDepth(int start, int *store, int reverse, int target);
    void doPrecomputation();
    int query(int a, int b);
};

