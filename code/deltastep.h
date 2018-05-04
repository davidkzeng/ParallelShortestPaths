#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <algorithm>

#include "graph.h"
#include "bucket.h"

class DeltaGraph {
  public:
    int nnode;
    int nedge;
    int delta;

    Graph *g;

    int *light_neighbor;
    int *heavy_neighbor;

    int *light_neighbor_weights;
    int *heavy_neighbor_weights;

    int *light_neighbor_start;
    int *heavy_neighbor_start;

    int *light_neighbor_end;
    int *heavy_neighbor_end;

    DeltaGraph(Graph *g, int delta);
    ~DeltaGraph();

    int num_light_neighbor(int node_id);
    int num_heavy_neighbor(int node_id);

    int* get_light_neighbors(int node_id);
    int* get_heavy_neighbors(int node_id);

    int* get_light_weights(int node_id);
    int* get_heavy_weights(int node_id);
};

class DeltaStep {

  Graph *g;
  DeltaGraph *dg;
  BucketStore *b;

  int *tent; // Will contain distances after calling runSSSP

  int nnode;
  int nedge;

  int delta;

  public:
    DeltaStep(Graph *g);
    ~DeltaStep();

    void runSSSP(int v);
    void relax(int v, int new_tent);

    void showDistances();
};
