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

  bucket_index = (int *) calloc(nnode, sizeof(int));
  for (int i = 0; i < nnode; i++) {
    bucket_index[i] = -1;
  }

}

BucketStore::~BucketStore() {
  for (int i = 0; i < num_buckets; i++) {
    delete buckets[i];
  }
  buckets.clear();

  free(bucket_index);
}

void BucketStore::atomicInsert(int i, int v) {
  if (bucket_index[v] < 0 || bucket_index[v] > i) {
    bucket_index[v] = i;
    buckets[i % num_buckets]->atomicInsert(v);
  }
}

void BucketStore::insert(int i, int v) {
  if (bucket_index[v] < 0 || bucket_index[v] > i) {
    bucket_index[v] = i;
    buckets[i % num_buckets]->insert(v);
  }
}

void BucketStore::clearBucket(int i) {
  UBA *bucket = buckets[i % num_buckets];
  for (int j = 0; j < bucket->size; j++) {
    bucket_index[bucket->store[j]] = -1;
  }

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




