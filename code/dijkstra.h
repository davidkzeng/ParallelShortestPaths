#include <queue>

#include "graph.h"

class Dijkstra {
  public:

    Dijkstra(Graph *g, int src);
    ~Dijkstra();
    int *distances;
    int size;
    int source;
    void showDistances();
};

