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

void UYSP::doPrecomputation() {
  int n = g->nnode;

  // First, we need to sample rho hop nodes
  for (int i = 0; i < rho; i++) {
    int rand_vertex = rand() % n;
    if (!hop_node_flag[rand_vertex]) {
      i--;
      continue;
    }
    hop_node_list[i] = rand_vertex;
    hop_node_flag[rand_vertex] = i;
  }

  int bfs_limit = rho * log(n);

  // Perform a BFS
  Frontier *cur = (Frontier *) calloc(1, sizeof(Frontier));
  Frontier *next = (Frontier *) calloc(1, sizeof(Frontier));
  Frontier *tmp;

  init_frontier_set(cur, n);
  init_frontier_set(next, n);

  int *visited_set = (int *) calloc(g->nnode, sizeof(int));

  for (int h_index = 0; h_index < rho; h_index++) {
    reset_frontier_set(next);
    memset(visited_set, 0, g->nnode * sizeof(int));

    cur->vertices[cur->count] = hop_node_list[h_index];
    cur->count++;

    for (int depth = 1; depth <= bfs_limit; depth++) {
      for (int i = 0; i < cur->count; i++) {
        int v = cur->vertices[i];
        visited_set[v] = 1;
        int* neighbors = g->get_neighbors(v);
        int num_neighbors = g->num_neighbors(v);
        for (int j = 0; j < num_neighbors; j++) {
          int neighbor = neighbors[j];
          if (!visited_set[neighbor]) {
            if (hop_node_flag[neighbor]) {
              hop_graph[(rho * i) + hop_node_flag[neighbor]] = depth;
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
  }

  // TODO:
  // Implementing APSP
  // Just doing floyd warshall for now lol maybe switch to something better

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
  int n = g->nnode;

  int *s_dist_to_hop = (int *) calloc(rho, sizeof(int));
  int *t_dist_from_hop = (int *) calloc(rho, sizeof(int));

  int bfs_limit = rho * log(n);

  Frontier *cur = (Frontier *) calloc(1, sizeof(Frontier));
  Frontier *next = (Frontier *) calloc(1, sizeof(Frontier));
  Frontier *tmp;

  init_frontier_set(cur, n);
  init_frontier_set(next, n);

  int *visited_set = (int *) calloc(g->nnode, sizeof(int));

  cur->vertices[cur->count] = s;
  cur->count++;

  for (int depth = 1; depth <= bfs_limit; depth++) {
    for (int i = 0; i < cur->count; i++) {
      int v = cur->vertices[i];
      visited_set[v] = 1;
      int* neighbors = g->get_neighbors(v);
      int num_neighbors = g->num_neighbors(v);
      for (int j = 0; j < num_neighbors; j++) {
        int neighbor = neighbors[j];
        if (!visited_set[neighbor]) {
          if (hop_node_flag[neighbor]) {
            s_dist_to_hop[hop_node_flag[neighbor]] = depth;
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

  // Fill up t_dist_from_hop

  // Compute final shortest paths
  return s - t;
}
