#include "dijkstra.h"
#include <climits>
#include <queue>

using namespace std;

#define INF INT_MAX


struct vertex_distance{
  int id;
  int distance;
};

class CompareDist {
  public:
    bool operator()(vertex_distance& n1, vertex_distance& n2) {
      if (n1.distance > n2.distance){
        return true;
      } else {
        return false;
      }
    }

};

Dijkstra::Dijkstra(Graph *g, int src) {
  size = g->nnode;
  source = src;
  distances = (int *) calloc(size, sizeof(int));
  bool *visited = (bool *) calloc(size, sizeof(bool));
  for (int i = 0; i < size; i++) {
    distances[i] = INF;
    visited[i] = false;
  }

  // distance to itself is 0
  distances[src] = 0;

  // creates a priority queue
  priority_queue<vertex_distance, vector< vertex_distance >, CompareDist> pq;

  vertex_distance first = {src,0};
  pq.push(first);

  while(!pq.empty()) {
    vertex_distance temp = pq.top();
    pq.pop();
    int nid = temp.id;
    int* neighbors = g->get_neighbors(nid);
    int num_neighbors = g->num_neighbors(nid);
    int* weights = g->get_weights(nid);

    for(int i = 0 ; i < num_neighbors; i++){

      int cur_nid = neighbors[i];
      if (visited[cur_nid]){
        continue;
      }
      if (distances[cur_nid] > (distances[nid] + weights[i])){
        distances[cur_nid] = distances[nid] + weights[i];
        vertex_distance newVertex;
        newVertex.id = cur_nid;
        newVertex.distance = distances[cur_nid];
        pq.push(newVertex);
      }

    }
    visited[nid] = true;
  }

  free(visited);
}

void Dijkstra::showDistances(){
  for(int i = 0; i < size; i++) {
    printf("Min distance from src %d to dest %d is: %d\n", source, i, distances[i]);
  }
}


