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

  delta = 2;
  //delta = g->max_weight; // Temporary
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

void DeltaStep::runSSSP(int v) {
  // Reusable list. Corresponds to S in psuedocode
  int *deletedNodes = (int *) calloc(nedge, sizeof(int));

  // Resuable list. Corresponds to Req in the psuedocode
  int *neighborNodes = (int *) calloc(nedge, sizeof(int));
  int *neighborNodeDists = (int *) calloc(nedge, sizeof(int));

  int *timestamp = (int *) calloc(nnode, sizeof(int));
  int curTime = 0;

  relax(v, 0);
  int i = 0; // cur_bucket

  while (!b->isEmpty()) {
    int numDeleted = 0;
    while (!b->isBucketEmpty(i)) {
      curTime++;
      int numNeighbors = 0;
      UBA *bucket = b->getBucket(i);
      int bucketSize = bucket->size;
      int *bucketStore = bucket->store;

      for (int j = 0; j < bucketSize; j++) {
        int nid = bucketStore[j];
        if ((tent[nid] / delta) < i) {
          continue;
        }
        if (timestamp[nid] == curTime) {
          continue;
        }
        timestamp[nid] = curTime;

        int num_light_neighbors = dg->num_light_neighbor(nid);
        int *light_neighbors = dg->get_light_neighbors(nid);
        int *light_weights = dg->get_light_weights(nid);

        for (int k = 0; k < num_light_neighbors; k++) {
          neighborNodes[numNeighbors] = light_neighbors[k];
          neighborNodeDists[numNeighbors] = light_weights[k] + tent[nid];
          numNeighbors++;
        }

        deletedNodes[numDeleted] = nid;
        numDeleted++;
      }

      bucket->clear();
      for (int j = 0; j < numNeighbors; j++) {
        relax(neighborNodes[j], neighborNodeDists[j]);
      }
    }

    int numNeighbors = 0;
    for (int j = 0; j < numDeleted; j++) {
      int nid = deletedNodes[j];
      int num_heavy_neighbors = dg->num_heavy_neighbor(nid);
      int *heavy_neighbors = dg->get_heavy_neighbors(nid);
      int *heavy_weights = dg->get_heavy_weights(nid);

      for (int k = 0; k < num_heavy_neighbors; k++) {
        neighborNodes[numNeighbors] = heavy_neighbors[k];
        neighborNodeDists[numNeighbors] = heavy_weights[k] + tent[nid];
        numNeighbors++;
      }
    }

    for (int j = 0; j < numNeighbors; j++) {
      relax(neighborNodes[j], neighborNodeDists[j]);
    }

    i++;
  }
}

void DeltaStep::relax(int v, int new_tent) {
  if (new_tent < tent[v]) {
    int new_bucket = new_tent / delta;
    b->insert(new_bucket, v);
    tent[v] = new_tent;
  }
}


void DeltaStep::showDistances() {
  for (int i = 0; i < nnode; i++) {
    printf("%d\n", tent[i]);
  }
}

