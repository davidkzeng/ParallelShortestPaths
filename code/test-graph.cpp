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

Graph create_graph(char* filename) {
  FILE *gfile = fopen(filename, "r");
  Graph ret (gfile);
  fclose(gfile);
  return ret;
}

void print_graph(Graph g) {
  g.show_graph();
}

void print_details(Graph g) {
  printf("Number of nodes: %d\n", g.nnode);
  printf("Number of edges: %d\n", g.nedge);
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

int main(){
  /*
  compareBFSWithUYSP();

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

  Graph weighted = create_graph("data/g_22_22_w.gph");
  print_details(weighted);

  /*print_graph(weighted);
  */
  printf("Max weight = %d\n", weighted.max_weight);

  Dijkstra d1 = Dijkstra(&weighted, 0);
  d1.showDistances();
  /*

  DeltaStep d2 = DeltaStep(&weighted);
  d2.runSSSP(0);
  d2.showDistances();
  */

  return 0;
}
