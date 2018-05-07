#include "graph.h"
#include "uysp.h"
#include "cycletimer.h"
#include "dijkstra.h"
#include "deltastep.h"

#include <stdio.h>
#include <stdlib.h>
#include <cmath>

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

UYSP create_uysp(Graph g) {
  int rho = pow(g.nnode, 0.5) + 1;
  UYSP sp(&g, rho);
  return sp;
}

void query_print(UYSP sp, int source, int dest) {
  printf("Distance from vtx %d to %d: %d\n",source,dest, sp.query(source, dest));
}

Graph* create_graph(char* filename) {
  FILE *gfile = fopen(filename, "r");
  Graph *ret = new Graph(gfile);
  fclose(gfile);
  return ret;
}

void print_graph(Graph *g) {
  g->show_graph();
}

void print_details(Graph *g) {
  printf("Number of nodes: %d\n", g->nnode);
  printf("Number of edges: %d\n", g->nedge);
  //printf("Number of inedges for node 0: %d\n",g.num_in_neighbors(0));
  //printf("Number of inedges for node 1: %d\n",g.num_in_neighbors(1));
  //printf("Number of inedges for node 2: %d\n",g.num_in_neighbors(2));

}

void compareBFSWithUYSP() {
  FILE *gfile = NULL;
  gfile = fopen("data/long_tile500000.gph", "r");

  Graph test(gfile);
  int rho = sqrt(test.nnode + 1);
  UYSP sp(&test, rho);

  SET_START(0);
  sp.doPrecomputation();
  SET_END(0);

  SET_START(1);
  int bfsDist = sp.BFS(100, 199000);
  SET_END(1);

  SET_START(2);
  int uyspDist = sp.query(100, 199000);
  SET_END(2);

  printf("Distance BFS: %d, Distance UY: %d\n", bfsDist, uyspDist);
  printf("%.4f %.4f %.4f\n", totalTime[0], totalTime[1], totalTime[2]);
}

void benchmarkDelta(char* graphName, char* fileName) {
  printf("---------------------------------\n");
  printf("Benchmark results for %s\n", graphName);
  Graph *weighted = create_graph(fileName);
  print_details(weighted);

  // print_graph(weighted);

  // printf("Max weight = %d\n", weighted.max_weight);

  for (int test_num = 0; test_num < 1; test_num++) {
    SET_START(0);
    Dijkstra *d1 = new Dijkstra(weighted, test_num);
    SET_END(0);
    // d1.showDistances();

    SET_START(1);
    DeltaStep *d2 = new DeltaStep(weighted);
    SET_END(1);
    SET_START(2);
    d2->runSSSP(test_num);
    SET_END(2);

    // d2.showDistances();

    bool match = true;
    for(int i = 0; i < d1->size ; i++) {
      if(d2->tent[i] != d1->distances[i]) {
        match = false;
        break;
      }
    }
    if (match) {
      printf("Results match\n");
    }else {
      printf("Results do not match\n");
    }

    delete d1;
    delete d2;
  }

  for (int i = 0; i < 3; i++) {
    if(i == 0){
      printf("Dijkstra time: ");
    }else if (i == 1) {
      printf("Deltastep precomputation time: ");
    } else if (i == 2) {
      printf("Deltastep query time: ");
    }
    printf("%.4f\n", totalTime[i]);
    startTime[i] = 0;
    totalTime[i] = 0;
  }

  delete weighted;
}

int main(){

  //compareBFSWithUYSP();
  /*
  //Small graph test
  Graph g_1 = create_graph("data/g_3_4.gph");
  print_details(g_1);
  UYSP sp = create_uysp(g_1);
  sp.doPrecomputation();
  query_print(sp,0,2);
  query_print(sp,2,1);
  printf("\n");

  Graph g_22_22 = create_graph("data/g_22_22.gph");
  print_details(g_22_22);
  UYSP sp_22_22 = create_uysp(g_22_22);
  sp_22_22.doPrecomputation();
  query_print(sp_22_22, 0,12);

  Graph g_u25600 = create_graph("data/g-u25600.gph");
  print_details(g_u25600);
  UYSP sp_u25600 = create_uysp(g_u25600);
  sp_u25600.doPrecomputation();
  query_print(sp_u25600, 0, 25599);
  */
  //Graph *weighted = create_graph("/afs/cs.cmu.edu/academic/class/15418-s18/data/vkang-data/amazon_w.gph");
  benchmarkDelta("Wiki Topcats","/afs/cs.cmu.edu/academic/class/15418-s18/data/vkang-data/wiki-topcats_w.gph");
  benchmarkDelta("Wiki Topcats Geometric Weights","/afs/cs.cmu.edu/academic/class/15418-s18/data/vkang-data/wiki-topcats_w_geom.gph");

  benchmarkDelta("Amazon","/afs/cs.cmu.edu/academic/class/15418-s18/data/vkang-data/amazon_w.gph");
  benchmarkDelta("Amazon Geometric weights","/afs/cs.cmu.edu/academic/class/15418-s18/data/vkang-data/amazon0302_w_geom.gph");
  benchmarkDelta("Gowalla","/afs/cs.cmu.edu/academic/class/15418-s18/data/vkang-data/loc-gowalla_edges_w.gph");
  benchmarkDelta("Gowalla Geometric weights","/afs/cs.cmu.edu/academic/class/15418-s18/data/vkang-data/loc-gowalla_edges_w_geom.gph");
  benchmarkDelta("Ring 200000 Uniform", "/afs/cs.cmu.edu/academic/class/15418-s18/data/vkang-data/ring_w_unif.gph");
  benchmarkDelta("Ring 200000 Geometric", "/afs/cs.cmu.edu/academic/class/15418-s18/data/vkang-data/ring_w_geom.gph");
  benchmarkDelta("Random 200000 Uniform", "/afs/cs.cmu.edu/academic/class/15418-s18/data/vkang-data/random_w_unif.gph");
  benchmarkDelta("Random 200000 Geometric", "/afs/cs.cmu.edu/academic/class/15418-s18/data/vkang-data/random_w_geom.gph");
  benchmarkDelta("Ring 10000000 Uniform", "/afs/cs.cmu.edu/academic/class/15418-s18/data/vkang-data/ring_big_w_unif.gph");
  //benchmarkDelta("Ring 10000000 Geometric", "/afs/cs.cmu.edu/academic/class/15418-s18/data/vkang-data/ring_big_w_geom.gph");

  return 0;
}
