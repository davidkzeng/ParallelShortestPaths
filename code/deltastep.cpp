#include "deltastep.h"

#define INF INT_MAX

DeltaGraph::DeltaGraph(Graph *g, int delta) {
  this->g = g;

  this->nnode = g->nnode;
  this->nedge = g->nedge;
  this->delta = delta;

  light_neighbor = (int *) calloc(nedge, sizeof(int));
  heavy_neighbor = (int *) calloc(nedge, sizeof(int));

  light_neighbor_weights = (int *) calloc(nedge, sizeof(int));
  heavy_neighbor_weights = (int *) calloc(nedge, sizeof(int));

  light_neighbor_start = (int *) calloc(nnode, sizeof(int));
  heavy_neighbor_start = (int *) calloc(nnode, sizeof(int));

  light_neighbor_end = (int *) calloc(nnode, sizeof(int));
  heavy_neighbor_end = (int *) calloc(nnode, sizeof(int));

  for (int i = 0; i < nnode; i++) {
    int *neighbors = g->get_neighbors(i);
    int *weights = g->get_weights(i);

    int offset = g->neighbor_start[i];
    light_neighbor_start[i] = offset;
    heavy_neighbor_start[i] = offset;

    int num_neighbors = g->num_neighbors(i);

    int num_light_neighbor = 0;
    int num_heavy_neighbor = 0;

    for (int j = 0; j < num_neighbors; j++) {
      int weight = weights[j];
      if (weight <= delta) {
        light_neighbor[offset + num_light_neighbor] = neighbors[j];
        light_neighbor_weights[offset + num_light_neighbor] = weights[j];
        num_light_neighbor++;
      } else {
        heavy_neighbor[offset + num_heavy_neighbor] = neighbors[j];
        heavy_neighbor_weights[offset + num_heavy_neighbor] = weights[j];
        num_heavy_neighbor++;
      }
    }

    light_neighbor_end[i] = offset + num_light_neighbor;
    heavy_neighbor_end[i] = offset + num_heavy_neighbor;
  }
}

int DeltaGraph::num_light_neighbor(int node_id) {
  return light_neighbor_end[node_id] - light_neighbor_start[node_id];
}

int DeltaGraph::num_heavy_neighbor(int node_id) {
  return heavy_neighbor_end[node_id] - heavy_neighbor_start[node_id];
}

int* DeltaGraph::get_light_neighbors(int node_id) {
  return &(light_neighbor[light_neighbor_start[node_id]]);
}

int* DeltaGraph::get_heavy_neighbors(int node_id) {
  return &(heavy_neighbor[heavy_neighbor_start[node_id]]);
}

int* DeltaGraph::get_light_weights(int node_id) {
  return &(light_neighbor_weights[light_neighbor_start[node_id]]);
}

int* DeltaGraph::get_heavy_weights(int node_id) {
  return &(heavy_neighbor_weights[heavy_neighbor_start[node_id]]);
}

DeltaStep::DeltaStep(Graph *g) {
  this->g = g;

  int delta = g->max_weight / 10; // Temporary
  dg = new DeltaGraph(g, delta);
  b = new BucketStore(delta, g->max_weight);

  nnode = g->nnode;
  nedge = g->nedge;

  tent = (int *) calloc(nnode, sizeof(int));

  for (int i = 0; i < nnode; i++) {
    tent[i] = INF;
  }
}

DeltaStep::~DeltaStep() {
  delete dg;
  delete b;

  free(tent);
}

void DeltaStep::runSSSP() {

}

void relax(int v, int new_tent) {

}




