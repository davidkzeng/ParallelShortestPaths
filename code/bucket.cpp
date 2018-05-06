#include "bucket.h"

UBA::UBA(int init_size) {
  int min_size = init_size < 1000000 ? 1000000 : init_size;

  store = (int *) calloc(min_size, sizeof(int));
  cap = min_size;
  size = 0;
}

UBA::~UBA() {
  free(store);
}

void UBA::resize() {
  int new_cap = 2 * cap;
  int *new_store = (int *) calloc(new_cap, sizeof(int));

  for (int i = 0; i < cap; i++) {
    new_store[i] = store[i];
  }

  int *old_store = store;
  store = new_store;
  cap = new_cap;
  free(old_store);
}

void UBA::insert(int v) {
  store[size++] = v;
}

void UBA::atomicInsert(int v) {
  int val;

#pragma omp atomic capture
  val = size++;
  store[val] = v;
}

void UBA::clear() {
  size = 0;
}

BucketStore::BucketStore(int delta, int max, int nnode) {
  this->delta = delta;
  this->max_edge_weight = max;

  num_buckets = (max_edge_weight / delta) + 1;
  buckets.reserve(num_buckets);
  for (int i = 0; i < num_buckets; i++) {
    buckets.push_back(new UBA(10));
  }

}

BucketStore::~BucketStore() {
  for (int i = 0; i < num_buckets; i++) {
    delete buckets[i];
  }
  buckets.clear();
}

void BucketStore::atomicInsert(int i, int v) {
  buckets[i % num_buckets]->atomicInsert(v);
}

void BucketStore::insert(int i, int v) {
  buckets[i % num_buckets]->insert(v);
}

void BucketStore::clearBucket(int i) {
  UBA *bucket = buckets[i % num_buckets];
  buckets[i % num_buckets]->clear();
}

bool BucketStore::isBucketEmpty(int i) {
  return buckets[i % num_buckets]->size == 0;
}

bool BucketStore::isEmpty() {
  for (int i = 0; i < num_buckets; i++) {
    if (!isBucketEmpty(i)) {
      return false;
    }
  }
  return true;
}

UBA* BucketStore::getBucket(int i) {
  return buckets[i % num_buckets];
}




