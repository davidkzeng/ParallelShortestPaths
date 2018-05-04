#include "deltastep.h"

#define INF INT_MAX

#include "cycletimer.h"

#define TIMER_SIZE 8
#define NUM_THREADS 12

#define SET_START(arg) set_start(arg)
#define SET_END(arg) set_end(arg)

static double startTime[TIMER_SIZE];
static double totalTime[TIMER_SIZE];

static inline void set_start(int activity) {
    startTime[activity] = currentSeconds() * 1000;
}

static inline void set_end(int activity) {
    double timeSpent = (currentSeconds() * 1000) - startTime[activity];
          totalTime[activity] += timeSpent;
}

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

#pragma omp parallel for schedule(static)
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

DeltaGraph::~DeltaGraph() {
  free(light_neighbor);
  free(heavy_neighbor);
  free(light_neighbor_weights);
  free(heavy_neighbor_weights);
  free(light_neighbor_start);
  free(heavy_neighbor_start);
  free(light_neighbor_end);
  free(heavy_neighbor_end);
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

  delta = 20;
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
#pragma omp parallel for schedule(static)
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

        // Setting Req
        int array_spot;
        for (int k = 0; k < num_light_neighbors; k++) {
#pragma omp atomic capture
          array_spot = numNeighbors++;

          neighborNodes[array_spot] = light_neighbors[k];
          neighborNodeDists[array_spot] = light_weights[k] + tent[nid];
        }

      }
#pragma omp parallel for schedule(static, 32)
      for (int j = 0; j < bucketSize; j++) {
        int nid = bucketStore[j];
        deletedNodes[numDeleted + j] = nid;
      }

      numDeleted += bucketSize;
      // B[i] = 0
      bucket->clear();
      // foreach (v,x) in Req do relax(v,x)
      for (int j = 0; j < numNeighbors; j++) {
        relax(neighborNodes[j], neighborNodeDists[j]);
      }
    }

    // Sets "Req" to heavy edges
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

    // Relax previously deferred edges
    for (int j = 0; j < numNeighbors; j++) {
      relax(neighborNodes[j], neighborNodeDists[j]);
    }

    i++;
  }

  free(deletedNodes);
  free(neighborNodes);
  free(neighborNodeDists);
  free(timestamp);
}

void DeltaStep::relax(int v, int new_tent) {
  // Shorter path to v?
  if (new_tent < tent[v]) {
    // Insert into new bucket
    int new_bucket = new_tent / delta;
    b->insert(new_bucket, v);
    tent[v] = new_tent;
  }
}


void DeltaStep::showDistances() {
  for (int i = 0; i < nnode; i++) {
    printf("Min distance from src 0 to dest %d is: %d\n", i, tent[i]);
  }
}

