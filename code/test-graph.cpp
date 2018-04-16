#include "graph.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
  //Graph test (NULL);
  //printf("Number of neighbors:%d\n", test.num_neighbors(0));

  FILE *gfile = NULL;
  gfile = fopen("data/g_6_3.gph", "r");

  Graph test1 (gfile);
  //printf("Number of neighbors:%d\n", test1.num_neighbors(0));
  printf("Number of nodes: %d\n", test1.nnode);
  printf("Number of edges: %d\n", test1.nedge);
  test1.show_graph();

  FILE *gfile2 = fopen("data/amazon0302.txt", "r");
  Graph test2 (gfile2);
  printf("Number of nodes: %d\n", test2.nnode);
  printf("Number of edges: %d\n", test2.nedge);
  test2.show_graph();
  return 0;
}
