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

int* Graph::get_weights(int node_id) {
  return &(weights[neighbor_start[node_id]]);
}

int Graph::num_in_neighbors(int node_id) {
  return in_neighbor_start[node_id+1] - in_neighbor_start[node_id];
}

int* Graph::get_in_neighbors(int node_id) {
  return &(in_neighbor[in_neighbor_start[node_id]]);
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

bool Graph::new_graph(int num_node, int num_edge, bool is_weight) {
  bool ok = true;
  nnode = num_node;
  nedge = num_edge;
  is_weighted = is_weight;
  neighbor = (int*) calloc(nedge, sizeof(int));
  ok = ok && neighbor != NULL;
  weights = (int*) calloc(nedge, sizeof(int));
  ok = ok && weights != NULL;
  neighbor_start = (int*)calloc(nnode + 1, sizeof(int));
  ok = ok && neighbor_start != NULL;
  in_edge_count = (int*) calloc(nnode, sizeof(int));
  ok = ok && in_edge_count != NULL;
  in_neighbor = (int*) calloc(nedge, sizeof(int));
  ok = ok && in_neighbor != NULL;
  in_neighbor_start = (int*) calloc(nnode + 1, sizeof(int));
  ok = ok && in_neighbor_start != NULL;
  if (!ok) {
    outmsg("Couldn't allocate graph data structures");
    return false;
  }

  return true;
}

bool Graph::read_graph(FILE *infile) {
  char linebuf[MAXLINE];
  int num_node, num_edge;
  char weighted;
  int i, hid, tid, wid;
  int nid, eid;

  // Read header information
  while (fgets(linebuf, MAXLINE, infile) != NULL) {
    if (!is_comment(linebuf))
        break;
  }
  if (sscanf(linebuf, "%d %d %c", &num_node, &num_edge, &weighted) != 3) {
    outmsg("ERROR, Malformed graph file header (line 1)\n");
    return false;
  }
  bool is_weight = weighted == 'w';
  if (!new_graph(num_node, num_edge, is_weight)) {
    return false;
  }

  nid = -1;
  eid = 0;
  for (i = 0; i < num_edge; i++) {
    while (fgets(linebuf, MAXLINE, infile) != NULL) {
      if (!is_comment(linebuf))
        break;
    }
    if (!is_weighted) {
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
      in_edge_count[tid] += 1;
    }else{
      if (sscanf(linebuf, "%d %d %d", &hid, &tid,&wid) != 3) {
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
      if (wid < 0) {
        outmsg("Negative weight %d on line %d\n", wid, i+2);
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
      neighbor[eid] = tid;
      weights[eid] = wid;
      eid++;
      in_edge_count[tid] += 1;

    }
  }
  while(nid < num_node) {
    nid++;
    neighbor_start[nid] = eid;
  }
  neighbor_start[num_node] = eid;
  // Generate in edges graph
  create_in_graph(infile);

  outmsg("Loaded graph with %d nodes and %d edges \n", num_node, num_edge);
  return true;
}

bool Graph::create_in_graph(FILE* infile) {
  char linebuf[MAXLINE];
  int num_node,num_edge;
  int i, hid, tid;
  int nid;

  // Puts in values to in_neighbor_start
  int sum = 0;
  for(i = 1; i < nnode + 1; i++) {
    sum += in_edge_count[i-1];
    in_neighbor_start[i] = sum;
  }

  // The index to place the next inedge for each of the nodes
  int *cur_in_index = (int*) calloc (nnode, sizeof(int));
  // Initilize to in_neighbor_start
  for(i = 0; i < nnode; i++){
    cur_in_index[i] = in_neighbor_start[i];
  }

  // Move read pointer to top of file
  rewind(infile);

  // Read header information
  while (fgets(linebuf, MAXLINE, infile) != NULL) {
    if (!is_comment(linebuf))
        break;
  }
  if (sscanf(linebuf, "%d %d", &num_node, &num_edge) != 2) {
    outmsg("ERROR, Malformed graph file header (line 1)\n");
    return false;
  }

  nid = -1;
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
    in_neighbor[cur_in_index[tid]] = hid;
    cur_in_index[tid] += 1;
  }

  free(cur_in_index);
  return true;
}

void Graph::show_graph() {
  int nid = 0;
  int eid = 0;
  outmsg("Graph\n");
  for (nid = 0; nid < nnode; nid++) {
    outmsg("%d:", nid);
    for (eid = neighbor_start[nid]; eid < neighbor_start[nid+1]; eid++) {
      if (is_weighted) {
        outmsg(" %d %d", neighbor[eid], weights[eid]);
      } else {
        outmsg(" %d", neighbor[eid]);
      }
    }
    outmsg("\n");
  }
}

void Graph::show_in_graph() {
  int nid = 0;
  outmsg("In_Graph\n");

  /*
  for (nid = 0; nid < nnode; nid++) {
    outmsg("%d:", nid);
    for (eid = in_neighbor_start[nid]; eid < in_neighbor_start[nid+1]; eid++) {
      outmsg(" %d", in_neighbor[eid]);
    }
    outmsg("\n");
  }*/

  int i = 0;
  // Tests whether class functions work properly
  for (nid = 0; nid < nnode; nid++) {
    outmsg("%d:", nid);
    int* arr = get_in_neighbors(nid);
    int count = num_in_neighbors(nid);
    for(i = 0; i < count; i++) {
      outmsg(" %d", arr[i]);
    }
    outmsg("\n");

  }
}
