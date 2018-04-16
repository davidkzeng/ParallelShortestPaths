// graph.cpp

#include "graph.h"

using namespace std;
Graph::Graph(FILE *infile) {
  if (infile == NULL) {
    //change this case later, solely for testing purposes
    nnode = 2;
    nedge = 1;
    neighbor = (int*) calloc(2, sizeof(int));
    neighbor[0] = 1;
    neighbor_start = (int*)calloc(3, sizeof(int));
    neighbor_start[1] = 1;
    neighbor_start[2] = 2;
  } else {
    read_graph(infile);
  }

}

int Graph::num_neighbors(int node_id) {
  return neighbor_start[node_id+1] - neighbor_start[node_id];
}

int* Graph::get_neighbors(int node_id) {
  return &(neighbor[neighbor_start[node_id]]);
}

/* See whether line of text is a comment */
static inline bool is_comment(char* s) {
  int i;
  int n = strlen(s);
  for (i=0; i < n; i++) {
    char c = s[i];
    if (!isspace(c))
      return c == '#';
  }
  return false;
}

bool Graph::new_graph(int num_node, int num_edge) {
  bool ok = true;
  nnode = num_node;
  nedge = num_edge;
  neighbor = (int*) calloc(nedge, sizeof(int));
  ok = ok && neighbor != NULL;
  neighbor_start = (int*)calloc(nnode + 1, sizeof(int));
  ok = ok && neighbor_start != NULL;
  if (!ok) {
    outmsg("Couldn't allocate graph data structures");
    return false;
  }

  return true;
}

bool Graph::read_graph(FILE *infile) {
  char linebuf[MAXLINE];
  int num_node, num_edge;
  int i, hid, tid;
  int nid, eid;

  // Read header information
  while (fgets(linebuf, MAXLINE, infile) != NULL) {
    if (!is_comment(linebuf))
        break;
  }
  if (sscanf(linebuf, "%d %d", &num_node, &num_edge) != 2) {
    outmsg("ERROR, Malformed graph file header (line 1)\n");
    return false;
  }
  if (!new_graph(num_node, num_edge)) {
    return false;
  }

  nid = -1;
  eid = 0;
  for (i = 0; i < num_edge; i++) {
    while (fgets(linebuf, MAXLINE, infile) != NULL) {
      if (!is_comment(linebuf))
        break;
    }
    if (sscanf(linebuf, "%d %d", &hid, &tid) != 2) {
      outmsg("Line #%u of graph file malformed\n", i+2);
      return false;
    }
    if (hid < 0 || hid >= num_node) {
      outmsg("Invalid head index %d on line %d\n", hid, i+2);
      return false;
    }
    if (tid < 0 || tid >= num_node) {
      outmsg("Invalid tail index %d on line %d\n", tid, i+2);
      return false;
    }
    if (hid < nid) {
      outmsg("Head index %d on line %d out of order \n", hid, i+2);
      return false;
    }
    //Starting edges for new nodes(s)
    while (nid < hid) {
      nid++;
      neighbor_start[nid] = eid;
    }
    neighbor[eid++] = tid;
  }
  while(nid < num_node) {
    nid++;
    neighbor_start[nid] = eid;
  }
  neighbor_start[num_node] = eid;
  outmsg("Loaded graph with %d nodes and %d edges \n", num_node, num_edge);
  return true;
}

void Graph::show_graph() {
  int nid = 0;
  int eid = 0;
  outmsg("Graph\n");
  for (nid = 0; nid < nnode; nid++) {
    outmsg("%d:", nid);
    for (eid = neighbor_start[nid]; eid < neighbor_start[nid+1]; eid++) {
      outmsg(" %d", neighbor[eid]);
    }
    outmsg("\n");
  }
}
