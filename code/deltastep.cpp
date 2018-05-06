#include "deltastep.h"

#define INF INT_MAX

#include "cycletimer.h"

#define TIMER_SIZE 8
#define NUM_THREADS 16

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

  delta = 5;
  //delta = g->max_weight; // Temporary

  dg = new DeltaGraph(g, delta);

  b = new BucketStore(delta, g->max_weight, g->nnode);

  nnode = g->nnode;
  nedge = g->nedge;

#if OMP
  tent = (std::atomic<int> *) calloc(nnode, sizeof(std::atomic<int>));
#else
  tent = (int *) calloc(nnode, sizeof(int));
#endif

  for (int i = 0; i < nnode; i++) {
    tent[i] = INF;
  }
}

DeltaStep::~DeltaStep() {
  delete dg;
  delete b;

  free(tent);
}

void DeltaStep::runSeqSSSP(int v) {
  int *deletedNodes = (int *) calloc(nedge, sizeof(int));

  int *neighborNodes = (int *) calloc(nedge, sizeof(int));
  int *neighborNodeDists = (int *) calloc(nedge, sizeof(int));

  int *timestamp = (int *) calloc(nnode, sizeof(int));
  int curTime = 0;

  relax(v, 0);
  int i = 0;

  while (!b->isEmpty()) {
    int numDeleted = 0;
    while (!b->isBucketEmpty(i)) {
      curTime++;
      int numNeighbors = 0;
      UBA *bucket = b->getBucket(i);
      int bucketSize = bucket->size;
      int *bucketStore = bucket->store;
      SET_START(0);
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

        int array_spot;
        for (int k = 0; k < num_light_neighbors; k++) {
          array_spot = numNeighbors++;

          neighborNodes[array_spot] = light_neighbors[k];
          neighborNodeDists[array_spot] = light_weights[k] + tent[nid];
        }

      }
      for (int j = 0; j < bucketSize; j++) {
        int nid = bucketStore[j];
        deletedNodes[numDeleted + j] = nid;
      }
      SET_END(0);
      numDeleted += bucketSize;
      b->clearBucket(i);
      SET_START(1);
      for (int j = 0; j < numNeighbors; j++) {
        relax(neighborNodes[j], neighborNodeDists[j]);
      }
      SET_END(1);
    }
    int numNeighbors = 0;
    SET_START(2);
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
    SET_END(2);
    SET_START(3);
    for (int j = 0; j < numNeighbors; j++) {
      relax(neighborNodes[j], neighborNodeDists[j]);
    }
    SET_END(3);
    i++;
  }

  free(deletedNodes);
  free(neighborNodes);
  free(neighborNodeDists);
  free(timestamp);
  printf("%.4f %.4f %.4f %.4f\n", totalTime[0], totalTime[1], totalTime[2], totalTime[3]);

}

inline bool update_min(std::atomic<int>& atom, const int val)
{
  int atom_val = atom;
  while (atom_val > val) {
    if (atom.compare_exchange_weak(atom_val, val, std::memory_order_relaxed))
        return true;
  }
  return false;
}

void DeltaStep::runSSSP(int v) {
#if !OMP
  runSeqSSSP(v);
  return;
#endif
#if OMP
  // Reusable list. Corresponds to S in psuedocode
  int *deletedNodes = (int *) calloc(nedge, sizeof(int));

  // Resuable list. Corresponds to Req in the psuedocode
  //int *neighborNodes = (int *) calloc(nedge, sizeof(int));
  //int *neighborNodeDists = (int *) calloc(nedge, sizeof(int));

  int **neighborNodes = (int **) calloc(NUM_THREADS, sizeof(int*));
  int **neighborNodeDists = (int **) calloc(NUM_THREADS, sizeof(int*));
  int *numNeighbors = (int*) calloc(NUM_THREADS, sizeof(int));

  for (int i = 0 ; i < NUM_THREADS; i++) {
    neighborNodes[i] = (int*) calloc(nedge,sizeof(int));
    neighborNodeDists[i] = (int*) calloc(nedge,sizeof(int));
  }

  int *timestamp = (int *) calloc(nnode, sizeof(int));
  int curTime = 0;

  relax(v, 0);
  int i = 0; // cur_bucket

  while (!b->isEmpty()) {
    int numDeleted = 0;
    while (!b->isBucketEmpty(i)) {
      curTime++;
      UBA *bucket = b->getBucket(i);
      int bucketSize = bucket->size;
      int *bucketStore = bucket->store;
      SET_START(0);
#pragma omp parallel num_threads(NUM_THREADS)
      {
        int numNeighborsPrivate = 0;
        int thread_id = omp_get_thread_num();

#pragma omp for schedule(static)
        for (int j = 0; j < bucketSize; j++) {
          int nid = bucketStore[j];

          if (timestamp[nid] == curTime || (tent[nid] / delta) < i) {
            continue;
          }

          timestamp[nid] = curTime;

          int num_light_neighbors = dg->num_light_neighbor(nid);
          int *light_neighbors = dg->get_light_neighbors(nid);
          int *light_weights = dg->get_light_weights(nid);

          // Setting Req
          for (int k = 0; k < num_light_neighbors; k++) {

            neighborNodes[thread_id][numNeighborsPrivate] = light_neighbors[k];
            neighborNodeDists[thread_id][numNeighborsPrivate] =
              light_weights[k] + tent[nid];
            numNeighborsPrivate++;
          }

        }
        numNeighbors[thread_id] = numNeighborsPrivate;
      }
      // This is very weird, maybe investigate further
      //#pragma omp simd
      for (int j = 0; j < bucketSize; j++) {
        int nid = bucketStore[j];
        deletedNodes[numDeleted + j] = nid;
      }
      SET_END(0);
      numDeleted += bucketSize;
      // B[i] = {}
      b->clearBucket(i);
      SET_START(1);
      // foreach (v,x) in Req do relax(v,x)
      for (int i = 0; i < NUM_THREADS; i++) {
        for (int j = 0; j < numNeighbors[i]; j++) {
          relax(neighborNodes[i][j], neighborNodeDists[i][j]);
        }
      }
      SET_END(1);
    }
    // Sets "Req" to heavy edges
    SET_START(2);
    if (numDeleted > 100) {
#pragma omp parallel num_threads(NUM_THREADS)
    {
      int numNeighborsDelPrivate = 0;
      int thread_id = omp_get_thread_num();

#pragma omp for schedule(static)
      for (int j = 0; j < numDeleted; j++) {
        int nid = deletedNodes[j];
        int num_heavy_neighbors = dg->num_heavy_neighbor(nid);
        int *heavy_neighbors = dg->get_heavy_neighbors(nid);
        int *heavy_weights = dg->get_heavy_weights(nid);

        for (int k = 0; k < num_heavy_neighbors; k++) {
          neighborNodes[thread_id][numNeighborsDelPrivate] = heavy_neighbors[k];
          neighborNodeDists[thread_id][numNeighborsDelPrivate] = heavy_weights[k] + tent[nid];
          numNeighborsDelPrivate++;
        }
      }

      numNeighbors[thread_id] = numNeighborsDelPrivate;
    }
    SET_END(2);
    SET_START(3);
    // Relax previously deferred edges
#pragma omp parallel for schedule(static)
    for (int thread_id = 0; thread_id < NUM_THREADS; thread_id++) {
      for (int j = 0; j < numNeighbors[thread_id]; j++) {
        relaxAtomic(neighborNodes[thread_id][j], neighborNodeDists[thread_id][j]);
      }
    }
    } else {
      int numNeighborsDelPrivate = 0;
      int thread_id = 0;
      for (int j = 0; j < numDeleted; j++) {
        int nid = deletedNodes[j];
        int num_heavy_neighbors = dg->num_heavy_neighbor(nid);
        int *heavy_neighbors = dg->get_heavy_neighbors(nid);
        int *heavy_weights = dg->get_heavy_weights(nid);

        for (int k = 0; k < num_heavy_neighbors; k++) {
          neighborNodes[thread_id][numNeighborsDelPrivate] = heavy_neighbors[k];
          neighborNodeDists[thread_id][numNeighborsDelPrivate] = heavy_weights[k] + tent[nid];
          numNeighborsDelPrivate++;
        }
      }

      numNeighbors[thread_id] = numNeighborsDelPrivate;
      SET_END(2);
      SET_START(3);
      for (int j = 0; j < numNeighbors[thread_id]; j++) {
        relax(neighborNodes[thread_id][j], neighborNodeDists[thread_id][j]);
      }
    }
    SET_END(3);
    i++;
  }
  free(deletedNodes);


  for (int i = 0 ; i < NUM_THREADS; i++) {
    free(neighborNodes[i]);
    free(neighborNodeDists[i]);
  }

  free(neighborNodes);
  free(neighborNodeDists);
  free(timestamp);
  printf("%.4f %.4f %.4f %.4f\n", totalTime[0], totalTime[1], totalTime[2], totalTime[3]);
#endif
}

void DeltaStep::relax(int v, int new_tent) {
#if OMP
  if (update_min(tent[v], new_tent)) {
    // Insert into new bucket
    int new_bucket = new_tent / delta;
    b->insert(new_bucket, v);
  }
#else
  if (new_tent < tent[v]) {
    tent[v] = new_tent;
    int new_bucket = new_tent / delta;
    b->insert(new_bucket, v);
  }
#endif
}

void DeltaStep::relaxAtomic(int v, int new_tent) {
#if OMP
  // Shorter path to v?
  if (update_min(tent[v], new_tent)) {
    // Insert into new bucket
    int new_bucket = new_tent / delta;
    b->atomicInsert(new_bucket, v);
  }
#endif
}


void DeltaStep::showDistances() {
  for (int i = 0; i < nnode; i++) {
    int val = tent[i];
    printf("Min distance from src 0 to dest %d is: %d\n", i, val);
  }
}

