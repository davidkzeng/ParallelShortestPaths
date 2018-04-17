// Ullman and Yanakakis Shortest Path

#include "uysp.h"

struct Frontier {
  int count;
  int max;
  int *vertices;
};

void init_frontier_set(Frontier *f, int max) {
  f->max = max;
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
}

// Reverse = 0 for normal BFS, = 1 for following in edges
// target is an optinal argument (set -1 for no use), it adds an additional
// node to search for
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

  for (int depth = 1; depth <= bfs_limit; depth++) {
    for (int i = 0; i < cur->count; i++) {
      int v = cur->vertices[i];

      if (v == target) {
        return depth - 1;
      }
      visited_set[v] = 1;
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
          if (hop_node_flag[neighbor]) {
            store[hop_node_flag[neighbor]] = depth;
          }
          next->vertices[next->count] = neighbor;
          next->count++;
        }
      }
    }
    tmp = cur;
    cur = next;
    next = tmp;
  }

  free(cur->vertices);
  free(next->vertices);
  free(cur);
  free(next);
  free(visited_set);

  // Did not find target
  return -1;
}


void UYSP::doPrecomputation() {
  int n = g->nnode;
  // First, we need to sample rho hop nodes
  for (int i = 0; i < rho; i++) {
    int rand_vertex = rand() % n;
    if (hop_node_flag[rand_vertex]) {
      i--;
      continue;
    }
    hop_node_list[i] = rand_vertex;
    hop_node_flag[rand_vertex] = i;
  }

  bfs_limit = std::min((int) (6 * rho * log(n)), 10);
  for (int i = 0; i < rho; i++) {
    BFSStoreHopDepth(hop_node_list[i], &hop_graph[rho * i], 0, -1);
  }


  for (int k = 0; k < rho; k++) {
    for (int i = 0; i < rho; i++) {
      for (int j = 0; j < rho; j++) {
        hop_graph[rho * i + j] = ((hop_adj(i, j) == 0 && i != j) ||
          hop_adj(i, k) + hop_adj(k, j) < hop_adj(i, j) ? hop_adj(i, k)
          + hop_adj(k, j) : hop_adj(i, j));
      }
    }
  }

}

int UYSP::query(int s, int t) {
  // TODO: Do BFS with the in edges
  // Loop over hop nodes to find the minimum
  int *s_dist_to_hop = (int *) calloc(rho, sizeof(int));
  int *t_dist_from_hop = (int *) calloc(rho, sizeof(int));

  int res = BFSStoreHopDepth(s, s_dist_to_hop, 0, t);
  if (res != -1) {
    return res;
  }

  BFSStoreHopDepth(t, t_dist_from_hop, 1, -1);

  // Compute final shortest paths

  int min_dist = INT_MAX;

  for (int s_cand = 0; s_cand < rho; s_cand++) {
    // Distance 0 and not itself means not reachable within bfs_limit
    if (s_dist_to_hop[s_cand] == 0 && hop_node_list[s_cand] != s) {
      continue;
    }
    for (int t_cand = 0; t_cand < rho; t_cand++) {
      if (t_dist_from_hop[t_cand] && hop_node_list[t_cand] != t) {
        continue;
      }
      int dist = s_dist_to_hop[s_cand] + hop_adj(s_cand, t_cand) +
          t_dist_from_hop[t_cand];
      min_dist = dist < min_dist ? dist : min_dist;
    }
  }

  free(s_dist_to_hop);
  free(t_dist_from_hop);
  return min_dist;
}
