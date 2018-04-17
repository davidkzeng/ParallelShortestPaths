#include "graph.h"
#include "uysp.h"

#include <stdio.h>
#include <stdlib.h>
#include <cmath>

UYSP create_uysp(Graph g) {
  int rho = sqrt(g.nnode) + 1;
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
int main(){

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

  Graph g_u1024 = create_graph("data/g-u1024.gph");
  print_details(g_u1024);
  UYSP sp_u1024 = create_uysp(g_u1024);
  sp_u1024.doPrecomputation();
  query_print(sp_u1024, 0, 1023);

  return 0;
}
