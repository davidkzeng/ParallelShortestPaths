// Ullman and Yanakakis Shortest Path
#include "uysp.h"

UYSP::UYSP(int a) {
  test = a;
}

int UYSP::doPrecomputation() {
  return 2;
}

int UYSP::query(int a, int b) {
  return a - b;
}
