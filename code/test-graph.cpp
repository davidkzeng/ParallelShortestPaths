#include "graph.h"
#include "uysp.h"

#include <stdio.h>
#include <stdlib.h>
#include <cmath>

int compareBFSWithUYSP() {
  FILE *gfile = NULL;
  gfile = fopen("data/g-u64.gph", "r");

  Graph test(gfile);
  int rho = sqrt(test.nnode + 1);
  UYSP sp(&test, rho);

  sp.doPrecomputation();

  int bfsDist = sp.BFS(5, 20);
  int uyspDist = sp.query(5, 20);

  printf("Distance BFS: %d, Distance UY: %d", bfsDist, uyspDist);
}

int main(){
  //Graph test (NULL);
  //printf("Number of neighbors:%d\n", test.num_neighbors(0));

  compareBFSWithUYSP();
  FILE *gfile = NULL;
  gfile = fopen("data/g_3_4.gph", "r");

  Graph test1 (gfile);
  //printf("Number of neighbors:%d\n", test1.num_neighbors(0));
  printf("Number of nodes: %d\n", test1.nnode);
  printf("Number of edges: %d\n", test1.nedge);
  printf("Number of inedges for node 0: %d\n",test1.num_in_neighbors(0));
  printf("Number of inedges for node 1: %d\n",test1.num_in_neighbors(1));
  printf("Number of inedges for node 2: %d\n",test1.num_in_neighbors(2));
  test1.show_in_graph();

  int rho = sqrt(test1.nnode) + 1;
  UYSP sp(&test1, rho);
  sp.doPrecomputation();
  printf("Distance: %d\n", sp.query(0, 2));
  printf("Distance: %d\n", sp.query(2, 1));


  /*
  FILE *gfile2 = fopen("data/amazon0302.txt", "r");
  Graph test2 (gfile2);
  printf("Number of nodes: %d\n", test2.nnode);
  printf("Number of edges: %d\n", test2.nedge);
  test2.show_in_graph();
  printf("Number of inedges for node 0: %d\n",test2.num_in_neighbors(0));
  printf("Number of inedges for node 1: %d\n",test2.num_in_neighbors(1));
  printf("Number of inedges for node 2: %d\n",test2.num_in_neighbors(2));
  //test2.show_in_graph();
  */
  return 0;
}
