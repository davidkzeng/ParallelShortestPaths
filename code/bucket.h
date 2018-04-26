#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <algorithm>
#include <vector>


class UBA {
  public:
    int cap;
    int size;
    int* store;

    UBA(int init_size);
    ~UBA();
    void insert(int v);
    void clear();

  private:
    void resize();
};

class BucketStore {
  int delta;
  int max_edge_weight;
  int num_buckets;
  std::vector<UBA> buckets;

  public:
    BucketStore(int delta, int max);
    void insert(int index, int v);
    void clearBucket(int index);
    bool isBucketEmpty(int index);
    bool isEmpty();

    UBA *getBucket(int index);
};


