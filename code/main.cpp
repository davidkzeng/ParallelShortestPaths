#include <cmath>

#include "uysp.h"

int main() {
  UYSP spAlgo(3);

  spAlgo.doPrecomputation();

  const int size = 256;
  double sinTable[size];

#pragma omp parallel for
  for(int n=0; n<size; ++n)
    sinTable[n] = std::sin(2 * M_PI * n / size);
}
