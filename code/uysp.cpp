// Ullman and Yanakakis Shortest Path
// TODO: Fix bugs when graph is not connected
#include "uysp.h"
#include "cycletimer.h"

#define TIMER_SIZE 8

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

struct Frontier {
  int count;
  int max;
  int *vertices;
};

void init_frontier_set(Frontier *f, int max) {
  f->max = max;
  f->count = 0;
  f->vertices = (int *) calloc(max, sizeof(int));
}

void reset_frontier_set(Frontier *f) {
  f->count = 0;
}

inline int UYSP::hop_adj(int i, int j) {
  return hop_graph[rho * i + j];
}

UYSP::UYSP(Graph *g, int rho) {
  this->g = g;
  this->rho = rho;

  hop_node_list = (int *) calloc(rho, sizeof(int));
  hop_node_flag = (int *) calloc(g->nnode, sizeof(int));

  hop_graph = (int *) calloc(rho * rho, sizeof(int));
  for (int i = 0; i < rho; i++) {
    for (int j = 0; j < rho; j++) {
      hop_graph[rho * i + j] = (i == j ? 0 : -1);
    }
  }

}

/**
 * Wrapper around BFS
 */
int UYSP::BFS(int start, int target) {
  int saved_bfs_limit = bfs_limit;
  bfs_limit = g->nnode;;
  int ans = BFSStoreHopDepth(start, NULL, 0, target);
  bfs_limit = saved_bfs_limit;
  return ans;
}

/**
 * Performs a BFS starting at start for bfs_limit iterations.
 * store (int *) : array where depth of the hop nodes encountered should be stored
 * NULL if no need to store anything
 * reverse (int) : 0 for normal BFS, 1 for BFS along in-edges
 * target (int) : -1 for ignore, t for returning early with the depth of target
 * if found
 */
int UYSP::BFSStoreHopDepth(int start, int *store, int reverse, int target) {
  // Perform a BFS
  int n = g->nnode;

  Frontier *cur = (Frontier *) calloc(1, sizeof(Frontier));
  Frontier *next = (Frontier *) calloc(1, sizeof(Frontier));
  Frontier *tmp;

  init_frontier_set(cur, n);
  init_frontier_set(next, n);

  int *visited_set = (int *) calloc(g->nnode, sizeof(int));

  cur->vertices[cur->count] = start;
  cur->count++;
  visited_set[start] = 1;

  for (int depth = 1; depth <= bfs_limit; depth++) {
    for (int i = 0; i < cur->count; i++) {
      int v = cur->vertices[i];
      if (v == target) {
        return depth - 1;
      }
      int* neighbors;
      int num_neighbors;
      if (reverse) {
        neighbors = g->get_in_neighbors(v);
        num_neighbors = g->num_in_neighbors(v);
      } else {
        neighbors = g->get_neighbors(v);
        num_neighbors = g->num_neighbors(v);
      }

      for (int j = 0; j < num_neighbors; j++) {
        int neighbor = neighbors[j];
        if (!visited_set[neighbor]) {
          if (store != NULL && hop_node_flag[neighbor] >= 0) {
            store[hop_node_flag[neighbor]] = depth;
          }
          next->vertices[next->count] = neighbor;
          next->count++;
          visited_set[neighbor] = 1;
        }
      }
    }
    tmp = cur;
    cur = next;
    next = tmp;
    reset_frontier_set(next);
  }

  free(cur->vertices);
  free(next->vertices);
  free(cur);
  free(next);
  free(visited_set);

  // Did not find target
  return -1;
}

/**
 * Performs precomputation needed for queries
 */
void UYSP::doPrecomputation() {
  int n = g->nnode;
  for (int i = 0; i < n; i++) {
    hop_node_flag[i] = -1;
  }

  // First, we need to sample rho hop nodes
  for (int i = 0; i < rho; i++) {
    int rand_vertex = rand() % n;
    if (hop_node_flag[rand_vertex] >= 0) {
      i--;
      continue;
    }
    hop_node_list[i] = rand_vertex;
    hop_node_flag[rand_vertex] = i;
  }

  bfs_limit = std::max((int) (6 * (n / rho) * log(n)), 8);
#pragma omp parallel for schedule(dynamic)
  for (int i = 0; i < rho; i++) {
    BFSStoreHopDepth(hop_node_list[i], &hop_graph[rho * i], 0, -1);
  }

  for (int k = 0; k < rho; k++) {
#pragma omp parallel for schedule(static)
    for (int i = 0; i < rho; i++) {
      for (int j = 0; j < rho; j++) {
        if (hop_adj(k, j) == -1 || hop_adj(i, k) == -1) {
          continue;
        }
        hop_graph[rho * i + j] = ((hop_adj(i, j) == -1 ||
          hop_adj(i, k) + hop_adj(k, j) < hop_adj(i, j)) ? hop_adj(i, k)
          + hop_adj(k, j) : hop_adj(i, j));
      }
    }
  }


}

/**
 * s (int) : source vertex
 * t (int) : target vertex
 * returns distance from s to t if connected, otherwise returns -1
 */
int UYSP::query(int s, int t) {
  int *s_dist_to_hop = (int *) calloc(rho, sizeof(int));
  int *t_dist_from_hop = (int *) calloc(rho, sizeof(int));

  for (int i = 0; i < rho; i++) {
    s_dist_to_hop[i] = -1;
    t_dist_from_hop[i] = -1;
  }

  int res = BFSStoreHopDepth(s, s_dist_to_hop, 0, t);
  if (res != -1) {
    printf("shortcircuit");
    return res;
  }

  BFSStoreHopDepth(t, t_dist_from_hop, 1, -1);

  // Compute final shortest paths

  int min_dist = INT_MAX;

  for (int s_cand = 0; s_cand < rho; s_cand++) {
    // Distance 0 and not itself means not reachable within bfs_limit
    if (s_dist_to_hop[s_cand] == -1) {
      continue;
    }
    for (int t_cand = 0; t_cand < rho; t_cand++) {
      if (t_dist_from_hop[t_cand] == -1 || hop_adj(s_cand, t_cand) == -1) {
        continue;
      }

      int dist = s_dist_to_hop[s_cand] + hop_adj(s_cand, t_cand) +
          t_dist_from_hop[t_cand];
      min_dist = dist < min_dist ? dist : min_dist;
    }
  }

  free(s_dist_to_hop);
  free(t_dist_from_hop);
  return min_dist == INT_MAX ? -1 : min_dist;
}
